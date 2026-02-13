#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
  #include <windows.h>
  #define POPEN _popen
  #define PCLOSE _pclose
#else
  #include <unistd.h>
  #define POPEN popen
  #define PCLOSE pclose
#endif

#include "ping_runner.h"

void ping_summary_init(PingSummary* s) {
    if (!s) return;
    s->sent = 0; s->recv = 0; s->loss_pct = 100;
    s->rtt_min_ms = -1; s->rtt_avg_ms = -1; s->rtt_max_ms = -1;
}

static void safe_update_rtt(PingSummary* s, int ms) {
    if (!s) return;
    if (ms < 0) return;
    if (s->rtt_min_ms < 0 || ms < s->rtt_min_ms) s->rtt_min_ms = ms;
    if (s->rtt_max_ms < 0 || ms > s->rtt_max_ms) s->rtt_max_ms = ms;
}

static int extract_int_after(const char* line, const char* key) {
    const char* p = strstr(line, key);
    if (!p) return -1;
    p += strlen(key);
    while (*p == ' ' || *p == '=' || *p == '<') p++;
    int val = 0;
    int found = 0;
    while (*p && (*p >= '0' && *p <= '9')) {
        val = val * 10 + (*p - '0');
        p++;
        found = 1;
    }
    return found ? val : -1;
}

int run_ping_command(const char* target, int count, int timeout_ms, PingSummary* out) {
    if (!target || !*target || !out) return -1;
    if (count < 1) count = 1;
    if (count > 20) count = 20;
    if (timeout_ms < 200) timeout_ms = 200;
    if (timeout_ms > 10000) timeout_ms = 10000;

    char cmd[512];
#ifdef _WIN32
    // Windows: -n count, -w timeout (ms)
    snprintf(cmd, sizeof(cmd), "ping -n %d -w %d %s", count, timeout_ms, target);
#else
    // Linux: -c count, -W timeout (seconds)
    int timeout_s = (timeout_ms + 999) / 1000;
    if (timeout_s < 1) timeout_s = 1;
    snprintf(cmd, sizeof(cmd), "ping -c %d -W %d %s 2>&1", count, timeout_s, target);
#endif

    FILE* fp = POPEN(cmd, "r");
    if (!fp) return -2;

    char line[512];
    int replies = 0;
    int sum_rtt = 0;
    int rtt_count = 0;

    while (fgets(line, sizeof(line), fp)) {
#ifdef _WIN32
        if (strstr(line, "TTL=") || strstr(line, "ttl=")) {
            replies++;
            int ms = extract_int_after(line, "time=");
            if (ms < 0) ms = extract_int_after(line, "time<");
            if (ms >= 0) {
                safe_update_rtt(out, ms);
                sum_rtt += ms;
                rtt_count++;
            }
        }
        int sent = extract_int_after(line, "Sent =");
        int recv = extract_int_after(line, "Received =");
        int loss = extract_int_after(line, "Lost =");
        int pct  = extract_int_after(line, "(");
        if (sent >= 0 && recv >= 0 && loss >= 0) {
            out->sent = sent;
            out->recv = recv;
            if (pct >= 0 && pct <= 100) out->loss_pct = pct;
        }
#else
        if (strstr(line, "time=")) {
            replies++;
            const char* p = strstr(line, "time=");
            p += 5;
            int ms = atoi(p);
            if (ms >= 0) {
                safe_update_rtt(out, ms);
                sum_rtt += ms;
                rtt_count++;
            }
        }
        if (strstr(line, "packets transmitted") && strstr(line, "received")) {
            int nums[4] = {-1,-1,-1,-1};
            int n = 0;
            const char* q = line;
            while (*q && n < 4) {
                if (*q >= '0' && *q <= '9') {
                    nums[n++] = atoi(q);
                    while (*q && (*q >= '0' && *q <= '9')) q++;
                } else q++;
            }
            if (n >= 2) {
                out->sent = nums[0];
                out->recv = nums[1];
            }
            if (n >= 3 && nums[2] >= 0 && nums[2] <= 100) out->loss_pct = nums[2];
        }
#endif
    }

    (void)PCLOSE(fp);

    if (out->sent <= 0) out->sent = count;
    if (out->recv <= 0) out->recv = replies;

    if (out->sent > 0) {
        int loss = out->sent - out->recv;
        if (loss < 0) loss = 0;
        out->loss_pct = (int)((loss * 100) / out->sent);
    }
    if (rtt_count > 0) out->rtt_avg_ms = sum_rtt / rtt_count;

    if (out->recv == 0) return 1;
    return 0;
}

void ping_print_summary(const PingSummary* s, int rc) {
    if (!s) return;
    printf("[PING]   Count        : %d\n", s->sent);
    printf("[RESULT] Sent/Recv    : %d/%d\n", s->sent, s->recv);
    printf("[RESULT] Loss         : %d%%\n", s->loss_pct);

    if (s->recv > 0 && s->rtt_min_ms >= 0) {
        printf("[RESULT] RTT (ms)     : min=%d  avg=%d  max=%d\n",
               s->rtt_min_ms,
               (s->rtt_avg_ms >= 0 ? s->rtt_avg_ms : s->rtt_min_ms),
               s->rtt_max_ms);
        printf("[STATUS] OK\n");
    } else {
        printf("[RESULT] RTT (ms)     : n/a\n");
        printf("[STATUS] FAIL (no reply) rc=%d\n", rc);
    }
}
