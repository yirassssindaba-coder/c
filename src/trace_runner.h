#ifndef TRACE_RUNNER_H
#define TRACE_RUNNER_H

typedef struct {
    int max_hops;
    int shown_hops;
    int timeout_hops;
    int partial;
    char hop_lines[4096];
} TraceSummary;

void trace_summary_init(TraceSummary* t);

int run_trace_command(const char* target, int max_hops, int timeout_ms, TraceSummary* out);
void trace_print_summary(const TraceSummary* t, int rc);

#endif
