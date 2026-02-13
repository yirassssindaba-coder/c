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

#include "trace_runner.h"

void trace_summary_init(TraceSummary* t) {
    if (!t) return;
    t->max_hops = 15;
    t->shown_hops = 0;
    t->timeout_hops = 0;
    t->partial = 0;
    t->hop_lines[0] = '\0';
}

static int line_starts_with_int(const char* s) {
    while (*s == ' ' || *s == '\t') s++;
    return (*s >= '0' && *s <= '9');
}

int run_trace_command(const char* target, int max_hops, int timeout_ms, TraceSummary* out) {
    if (!target || !*target || !out) return -1;
    if (max_hops < 1) max_hops = 1;
    if (max_hops > 60) max_hops = 60;
    if (timeout_ms < 200) timeout_ms = 200;
    if (timeout_ms > 10000) timeout_ms = 10000;

    out->max_hops = max_hops;

    char cmd[512];
#ifdef _WIN32
    snprintf(cmd, sizeof(cmd), "tracert -h %d -w %d %s", max_hops, timeout_ms, target);
#else
    int timeout_s = (timeout_ms + 999) / 1000;
    if (timeout_s < 1) timeout_s = 1;
    snprintf(cmd, sizeof(cmd), "traceroute -m %d -w %d %s 2>&1", max_hops, timeout_s, target);
#endif

    FILE* fp = POPEN(cmd, "r");
    if (!fp) return -2;

    char line[512];
    int rc = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (!line_starts_with_int(line)) continue;

        size_t n = strlen(line);
        while (n > 0 && (line[n-1] == '\n' || line[n-1] == '\r')) {
            line[n-1] = '\0';
            n--;
        }

        if (strstr(line, "*") != NULL) out->timeout_hops++;

        size_t cur = strlen(out->hop_lines);
        if (cur + n + 2 < sizeof(out->hop_lines)) {
            strcat(out->hop_lines, line);
            strcat(out->hop_lines, "\n");
            out->shown_hops++;
        }

        if (out->shown_hops >= max_hops) break;
    }

    (void)PCLOSE(fp);

    if (out->timeout_hops > 0) out->partial = 1;
    if (out->shown_hops == 0) rc = 1;
    return rc;
}

void trace_print_summary(const TraceSummary* t, int rc) {
    if (!t) return;
    printf("[TRACE]  Max Hops     : %d\n", t->max_hops);
    printf("[TRACE]  Timeout      : (see command)\n\n");

    if (t->shown_hops > 0) {
        printf("%s", t->hop_lines);
        if (t->partial) {
            printf("\n[STATUS] PARTIAL (some hops timeout)\n");
        } else {
            printf("\n[STATUS] OK\n");
        }
    } else {
        printf("[STATUS] FAIL (traceroute not available / no output) rc=%d\n", rc);
#ifdef _WIN32
        printf("[HINT]  Pastikan 'tracert' tersedia (Windows bawaan).\n");
#else
        printf("[HINT]  Pastikan 'traceroute' terinstall. (Ubuntu/Debian: sudo apt install traceroute)\n");
#endif
    }
}
