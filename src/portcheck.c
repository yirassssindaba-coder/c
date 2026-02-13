#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #include <windows.h>
  typedef SOCKET socket_t;
  #define CLOSESOCK closesocket
#else
  #include <unistd.h>
  #include <fcntl.h>
  #include <errno.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netdb.h>
  typedef int socket_t;
  #define CLOSESOCK close
#endif

#include "portcheck.h"

void port_result_init(PortResult* pr) {
    if (!pr) return;
    pr->port = 0;
    strcpy(pr->state, "ERROR");
    strcpy(pr->info, "not run");
}

static int set_nonblocking(socket_t s) {
#ifdef _WIN32
    u_long mode = 1;
    return ioctlsocket(s, FIONBIO, &mode);
#else
    int flags = fcntl(s, F_GETFL, 0);
    if (flags < 0) return -1;
    return fcntl(s, F_SETFL, flags | O_NONBLOCK);
#endif
}

static int set_blocking(socket_t s) {
#ifdef _WIN32
    u_long mode = 0;
    return ioctlsocket(s, FIONBIO, &mode);
#else
    int flags = fcntl(s, F_GETFL, 0);
    if (flags < 0) return -1;
    return fcntl(s, F_SETFL, flags & ~O_NONBLOCK);
#endif
}

static int last_sock_error(void) {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

static const char* sock_err_text(int err, char* buf, size_t cap) {
#ifdef _WIN32
    snprintf(buf, cap, "WSA error=%d", err);
    return buf;
#else
    strerror_r(err, buf, cap);
    return buf;
#endif
}

int tcp_port_check(const char* host, int port, int timeout_ms, PortResult* out) {
    if (!host || !*host || !out) return -1;
    if (port < 1 || port > 65535) return -2;
    if (timeout_ms < 200) timeout_ms = 200;
    if (timeout_ms > 10000) timeout_ms = 10000;

    out->port = port;

    char port_str[16];
    snprintf(port_str, sizeof(port_str), "%d", port);

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_UNSPEC;

    struct addrinfo* res = NULL;
    int gai = getaddrinfo(host, port_str, &hints, &res);
    if (gai != 0 || !res) {
        strcpy(out->state, "ERROR");
        snprintf(out->info, sizeof(out->info), "DNS/resolve failed");
        return 3;
    }

    int rc = 4;
    for (struct addrinfo* p = res; p != NULL; p = p->ai_next) {
        socket_t s = (socket_t)socket(p->ai_family, p->ai_socktype, p->ai_protocol);
#ifdef _WIN32
        if (s == INVALID_SOCKET) continue;
#else
        if (s < 0) continue;
#endif
        if (set_nonblocking(s) != 0) {
            CLOSESOCK(s);
            continue;
        }

        int c = connect(s, p->ai_addr, (int)p->ai_addrlen);
#ifdef _WIN32
        int err = (c == SOCKET_ERROR) ? last_sock_error() : 0;
        if (c == 0) {
            set_blocking(s);
            CLOSESOCK(s);
            strcpy(out->state, "OPEN");
            strcpy(out->info, "connection success");
            rc = 0;
            break;
        }
        if (err != WSAEWOULDBLOCK && err != WSAEINPROGRESS) {
            set_blocking(s);
            CLOSESOCK(s);
            if (err == WSAECONNREFUSED) {
                strcpy(out->state, "CLOSED");
                strcpy(out->info, "connection refused");
                rc = 1;
                break;
            }
            strcpy(out->state, "ERROR");
            char eb[64];
            snprintf(out->info, sizeof(out->info), "%s", sock_err_text(err, eb, sizeof(eb)));
            rc = 2;
            break;
        }
#else
        if (c == 0) {
            set_blocking(s);
            CLOSESOCK(s);
            strcpy(out->state, "OPEN");
            strcpy(out->info, "connection success");
            rc = 0;
            break;
        }
        int err = last_sock_error();
        if (err != EINPROGRESS && err != EWOULDBLOCK) {
            set_blocking(s);
            CLOSESOCK(s);
            if (err == ECONNREFUSED) {
                strcpy(out->state, "CLOSED");
                strcpy(out->info, "connection refused");
                rc = 1;
                break;
            }
            strcpy(out->state, "ERROR");
            char eb[128];
            snprintf(out->info, sizeof(out->info), "%s", sock_err_text(err, eb, sizeof(eb)));
            rc = 2;
            break;
        }
#endif

        fd_set wfds;
        FD_ZERO(&wfds);
        FD_SET(s, &wfds);

        struct timeval tv;
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;

        int sel = select((int)(s + 1), NULL, &wfds, NULL, &tv);
        if (sel == 0) {
            set_blocking(s);
            CLOSESOCK(s);
            strcpy(out->state, "FILTERED");
            strcpy(out->info, "timeout");
            rc = 2;
            break;
        }
        if (sel < 0) {
            set_blocking(s);
            CLOSESOCK(s);
            strcpy(out->state, "ERROR");
            strcpy(out->info, "select failed");
            rc = 2;
            break;
        }

        int so_err = 0;
#ifdef _WIN32
        int len = sizeof(so_err);
#else
        socklen_t len = sizeof(so_err);
#endif
        getsockopt(s, SOL_SOCKET, SO_ERROR, (char*)&so_err, &len);

        set_blocking(s);
        CLOSESOCK(s);

        if (so_err == 0) {
            strcpy(out->state, "OPEN");
            strcpy(out->info, "connection success");
            rc = 0;
            break;
        }
#ifdef _WIN32
        if (so_err == WSAECONNREFUSED) {
            strcpy(out->state, "CLOSED");
            strcpy(out->info, "connection refused");
            rc = 1;
            break;
        }
#else
        if (so_err == ECONNREFUSED) {
            strcpy(out->state, "CLOSED");
            strcpy(out->info, "connection refused");
            rc = 1;
            break;
        }
#endif
        strcpy(out->state, "FILTERED");
        strcpy(out->info, "blocked/unreachable");
        rc = 2;
        break;
    }

    freeaddrinfo(res);
    return rc;
}

void port_print_summary(const char* host, const char* resolved_ip, int port, int timeout_ms,
                        const PortResult* pr, int rc) {
    printf("[PORT]   Target       : %s\n", host ? host : "n/a");
    if (resolved_ip && resolved_ip[0] != '\0') {
        printf("[PORT]   Resolved IP  : %s\n", resolved_ip);
    }
    printf("[PORT]   TCP          : %d\n", port);
    printf("[PORT]   Timeout      : %dms\n", timeout_ms);

    printf("[RESULT] Connection   : %s\n", pr ? pr->info : "n/a");
    printf("[STATUS] %s\n", pr ? pr->state : "ERROR");

    // Keep rc visible for debugging scripts/CI.
    printf("[RC]     Code         : %d\n", rc);
}
