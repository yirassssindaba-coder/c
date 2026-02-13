#ifndef PORTCHECK_H
#define PORTCHECK_H

typedef struct {
    int port;
    char state[16];
    char info[128];
} PortResult;

void port_result_init(PortResult* pr);

int tcp_port_check(const char* host, int port, int timeout_ms, PortResult* out);
void port_print_summary(const char* host, const char* resolved_ip, int port, int timeout_ms,
                        const PortResult* pr, int rc);

#endif
