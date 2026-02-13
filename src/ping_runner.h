#ifndef PING_RUNNER_H
#define PING_RUNNER_H

typedef struct {
    int sent;
    int recv;
    int loss_pct;
    int rtt_min_ms;
    int rtt_avg_ms;
    int rtt_max_ms;
} PingSummary;

void ping_summary_init(PingSummary* s);

int run_ping_command(const char* target, int count, int timeout_ms, PingSummary* out);
void ping_print_summary(const PingSummary* s, int rc);

#endif
