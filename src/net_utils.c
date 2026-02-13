#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
  // Keep Windows headers lightweight and avoid winsock include-order warnings.
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif

  // IMPORTANT: winsock2.h must be included BEFORE windows.h
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #include <windows.h>
  #include <direct.h>

  // MSVC auto-link only; GCC/Clang use -lws2_32 on the compile command.
  #ifdef _MSC_VER
    #pragma comment(lib, "ws2_32.lib")
  #endif
#else
  #include <sys/stat.h>
  #include <sys/types.h>
  #include <netdb.h>
  #include <arpa/inet.h>
  #include <unistd.h>
#endif

#include "net_utils.h"
#include "ping_runner.h"
#include "trace_runner.h"
#include "portcheck.h"

void app_options_init(AppOptions* opt) {
    memset(opt, 0, sizeof(*opt));
    opt->mode = MODE_MENU;
    opt->port = 0;
    opt->ping_count = 4;
    opt->max_hops = 15;
    opt->timeout_ms = 1500;
    opt->out_dir[0] = '\0';
}

void net_platform_init(void) {
#ifdef _WIN32
    WSADATA wsa;
    (void)WSAStartup(MAKEWORD(2,2), &wsa);
#endif
}

void net_platform_cleanup(void) {
#ifdef _WIN32
    WSACleanup();
#endif
}

static int ipv4_to_string(const struct sockaddr* sa, char* out_ip, size_t out_cap) {
    if (!sa || !out_ip || out_cap == 0) return -1;
#ifdef _WIN32
    // fallback to inet_ntop for sockaddr_in
    const struct sockaddr_in* in = (const struct sockaddr_in*)sa;
    if (InetNtopA(AF_INET, (PVOID)&in->sin_addr, out_ip, (DWORD)out_cap) == NULL) return -1;
    return 0;
#else
    const struct sockaddr_in* in = (const struct sockaddr_in*)sa;
    if (!inet_ntop(AF_INET, &in->sin_addr, out_ip, out_cap)) return -1;
    return 0;
#endif
}

int resolve_target_ipv4(const char* host_or_ip, char* out_ip, size_t out_cap, int* has_ip) {
    if (has_ip) *has_ip = 0;
    if (!host_or_ip || !*host_or_ip || !out_ip || out_cap == 0) return -1;

    struct in_addr addr4;
#ifdef _WIN32
    if (InetPtonA(AF_INET, host_or_ip, &addr4) == 1) {
#else
    if (inet_pton(AF_INET, host_or_ip, &addr4) == 1) {
#endif
        strncpy(out_ip, host_or_ip, out_cap - 1);
        out_ip[out_cap - 1] = '\0';
        if (has_ip) *has_ip = 1;
        return 0;
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* res = NULL;
    int rc = getaddrinfo(host_or_ip, NULL, &hints, &res);
    if (rc != 0 || !res) return -2;

    int ok = ipv4_to_string(res->ai_addr, out_ip, out_cap);
    freeaddrinfo(res);

    if (ok == 0) {
        if (has_ip) *has_ip = 1;
        return 0;
    }
    return -3;
}

int ensure_dir_exists(const char* path) {
    if (!path || !*path) return -1;
#ifdef _WIN32
    if (_mkdir(path) == 0) return 0;
    DWORD attr = GetFileAttributesA(path);
    if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) return 0;
    return -2;
#else
    if (mkdir(path, 0755) == 0) return 0;
    struct stat st;
    if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) return 0;
    return -2;
#endif
}

int save_text_file(const char* out_path, const char* content) {
    if (!out_path || !*out_path || !content) return -1;
    FILE* f = fopen(out_path, "wb");
    if (!f) return -2;
    fwrite(content, 1, strlen(content), f);
    fclose(f);
    return 0;
}

static void now_timestamp(char* buf, size_t cap) {
    time_t t = time(NULL);
    struct tm tmv;
#ifdef _WIN32
    localtime_s(&tmv, &t);
#else
    localtime_r(&t, &tmv);
#endif
    snprintf(buf, cap, "%04d%02d%02d_%02d%02d%02d",
             tmv.tm_year + 1900, tmv.tm_mon + 1, tmv.tm_mday,
             tmv.tm_hour, tmv.tm_min, tmv.tm_sec);
}

int save_report_text(const AppOptions* opt,
                     const char* resolved_ip,
                     const void* ping_summary,
                     const void* trace_summary,
                     const void* port_result) {
    if (!opt || !opt->out_dir[0]) return 0;

    if (ensure_dir_exists(opt->out_dir) != 0) {
        printf("[WARN] Gagal membuat folder output: %s\n", opt->out_dir);
        return -1;
    }

    char ts[32];
    now_timestamp(ts, sizeof(ts));

    char out_path[512];
#ifdef _WIN32
    snprintf(out_path, sizeof(out_path), "%s\\netdiag_report_%s.txt", opt->out_dir, ts);
#else
    snprintf(out_path, sizeof(out_path), "%s/netdiag_report_%s.txt", opt->out_dir, ts);
#endif

    char buf[8192];
    buf[0] = '\0';

    snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
             "=== NETDIAG REPORT ===\n"
             "Target      : %s\n"
             "Resolved IP : %s\n"
             "Timeout(ms) : %d\n\n",
             opt->target,
             (resolved_ip && *resolved_ip) ? resolved_ip : "(n/a)",
             opt->timeout_ms);

    if (opt->mode == MODE_PING && ping_summary) {
        const PingSummary* s = (const PingSummary*)ping_summary;
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                 "[PING]\nSent/Recv : %d/%d\nLoss     : %d%%\nRTT(ms)  : min=%d avg=%d max=%d\n\n",
                 s->sent, s->recv, s->loss_pct, s->rtt_min_ms, s->rtt_avg_ms, s->rtt_max_ms);
    } else if (opt->mode == MODE_TRACE && trace_summary) {
        const TraceSummary* t = (const TraceSummary*)trace_summary;
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                 "[TRACE]\nMax hops : %d\nShown    : %d\nTimeouts : %d\nStatus   : %s\n\n",
                 t->max_hops, t->shown_hops, t->timeout_hops,
                 t->partial ? "PARTIAL" : "OK");
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                 "Hops:\n%s\n", t->hop_lines);
    } else if (opt->mode == MODE_PORT && port_result) {
        const PortResult* pr = (const PortResult*)port_result;
        snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                 "[PORT]\nTCP  : %d\nState: %s\nInfo : %s\n\n",
                 pr->port, pr->state, pr->info);
    }

    int rc = save_text_file(out_path, buf);
    if (rc == 0) {
        printf("[OK] Report tersimpan: %s\n", out_path);
    } else {
        printf("[WARN] Gagal simpan report: %s\n", out_path);
    }
    return rc;
}
