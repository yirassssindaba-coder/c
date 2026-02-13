#ifndef NET_UTILS_H
#define NET_UTILS_H

#include <stddef.h>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  typedef SOCKET socket_t;
#else
  typedef int socket_t;
#endif

typedef enum {
    MODE_MENU = 0,
    MODE_PING = 1,
    MODE_TRACE = 2,
    MODE_PORT = 3
} Mode;

typedef struct {
    Mode mode;
    char target[256];
    int port;
    int ping_count;
    int max_hops;
    int timeout_ms;
    char out_dir[256]; // optional, save report if not empty
} AppOptions;

void app_options_init(AppOptions* opt);

void net_platform_init(void);
void net_platform_cleanup(void);

int resolve_target_ipv4(const char* host_or_ip, char* out_ip, size_t out_cap, int* has_ip);

int ensure_dir_exists(const char* path);
int save_text_file(const char* out_path, const char* content);

int save_report_text(const AppOptions* opt,
                     const char* resolved_ip,
                     const void* ping_summary,
                     const void* trace_summary,
                     const void* port_result);

#endif
