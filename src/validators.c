#include <ctype.h>
#include <string.h>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
#else
  #include <arpa/inet.h>
#endif

#include "validators.h"

int is_valid_ipv4(const char* s) {
    if (!s || !*s) return 0;
    unsigned char buf[16];
#ifdef _WIN32
    return InetPtonA(AF_INET, s, buf) == 1;
#else
    return inet_pton(AF_INET, s, buf) == 1;
#endif
}

static int is_label_char(int c) {
    return isalnum((unsigned char)c) || c == '-';
}

int is_valid_hostname(const char* s) {
    if (!s || !*s) return 0;

    size_t len = strlen(s);
    if (len < 1 || len > 253) return 0;

    if (s[0] == '.' || s[len - 1] == '.') return 0;

    int label_len = 0;
    for (size_t i = 0; i < len; i++) {
        char c = s[i];
        if (c == '.') {
            if (label_len == 0) return 0;
            if (s[i - 1] == '-') return 0;
            label_len = 0;
            continue;
        }
        if (!is_label_char(c)) return 0;
        label_len++;
        if (label_len > 63) return 0;
    }

    if (label_len == 0) return 0;
    if (s[len - 1] == '-') return 0;

    return 1;
}

int is_valid_host_or_ipv4(const char* s) {
    return is_valid_ipv4(s) || is_valid_hostname(s);
}
