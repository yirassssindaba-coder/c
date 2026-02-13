#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "menu.h"
#include "validators.h"
#include "net_utils.h"
#include "ping_runner.h"
#include "trace_runner.h"
#include "portcheck.h"

static void print_banner(void) {
    printf("========================================\n");
    printf(" Network Diagnostics Mini-Tool (C)\n");
    printf(" Ping | Traceroute | Port Check\n");
    printf("========================================\n");
}

static int parse_args(int argc, char** argv, AppOptions* opt) {
    // Supported:
    //   netdiag --ping <host> [--count N] [--timeout MS] [--out DIR]
    //   netdiag --trace <host> [--hops N] [--timeout MS] [--out DIR]
    //   netdiag --port <host> <port> [--timeout MS] [--out DIR]
    //   netdiag --menu
    // If no args -> menu
    app_options_init(opt);

    if (argc <= 1) {
        opt->mode = MODE_MENU;
        return 1;
    }

    if (strcmp(argv[1], "--menu") == 0) {
        opt->mode = MODE_MENU;
        return 1;
    }

    if (strcmp(argv[1], "--ping") == 0) {
        if (argc < 3) return 0;
        opt->mode = MODE_PING;
        strncpy(opt->target, argv[2], sizeof(opt->target) - 1);
    } else if (strcmp(argv[1], "--trace") == 0) {
        if (argc < 3) return 0;
        opt->mode = MODE_TRACE;
        strncpy(opt->target, argv[2], sizeof(opt->target) - 1);
    } else if (strcmp(argv[1], "--port") == 0) {
        if (argc < 4) return 0;
        opt->mode = MODE_PORT;
        strncpy(opt->target, argv[2], sizeof(opt->target) - 1);
        opt->port = atoi(argv[3]);
    } else {
        return 0;
    }

    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "--count") == 0 && i + 1 < argc) {
            opt->ping_count = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--hops") == 0 && i + 1 < argc) {
            opt->max_hops = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--timeout") == 0 && i + 1 < argc) {
            opt->timeout_ms = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--out") == 0 && i + 1 < argc) {
            strncpy(opt->out_dir, argv[++i], sizeof(opt->out_dir) - 1);
        }
    }

    return 1;
}

static void run_one_shot(const AppOptions* opt) {
    char resolved_ip[64] = {0};
    int has_ip = 0;

    // Validate target early to avoid shell injection in ping/traceroute runners
    if (!is_valid_host_or_ipv4(opt->target)) {
        printf("[ERROR] Host/IP tidak valid: '%s'\n", opt->target);
        return;
    }

    if (resolve_target_ipv4(opt->target, resolved_ip, sizeof(resolved_ip), &has_ip) != 0) {
        printf("[ERROR] DNS resolve gagal atau target tidak dapat di-resolve.\n");
        return;
    }

    printf("[NETDIAG] Target      : %s\n", opt->target);
    if (has_ip) {
        printf("[NETDIAG] Resolved IP : %s\n", resolved_ip);
    }

    if (opt->mode == MODE_PING) {
        PingSummary s;
        ping_summary_init(&s);
        int rc = run_ping_command(opt->target, opt->ping_count, opt->timeout_ms, &s);
        ping_print_summary(&s, rc);
        if (strlen(opt->out_dir) > 0) {
            save_report_text(opt, resolved_ip, &s, NULL, NULL);
        }
    } else if (opt->mode == MODE_TRACE) {
        TraceSummary t;
        trace_summary_init(&t);
        int rc = run_trace_command(opt->target, opt->max_hops, opt->timeout_ms, &t);
        trace_print_summary(&t, rc);
        if (strlen(opt->out_dir) > 0) {
            save_report_text(opt, resolved_ip, NULL, &t, NULL);
        }
    } else if (opt->mode == MODE_PORT) {
        PortResult pr;
        port_result_init(&pr);
        int rc = tcp_port_check(opt->target, opt->port, opt->timeout_ms, &pr);
        port_print_summary(opt->target, resolved_ip, opt->port, opt->timeout_ms, &pr, rc);
        if (strlen(opt->out_dir) > 0) {
            save_report_text(opt, resolved_ip, NULL, NULL, &pr);
        }
    }
}

int main(int argc, char** argv) {
    net_platform_init();

    print_banner();

    AppOptions opt;
    if (!parse_args(argc, argv, &opt)) {
        printf("Usage:\n");
        printf("  netdiag --menu\n");
        printf("  netdiag --ping <host> [--count N] [--timeout MS] [--out reports]\n");
        printf("  netdiag --trace <host> [--hops N] [--timeout MS] [--out reports]\n");
        printf("  netdiag --port <host> <port> [--timeout MS] [--out reports]\n");
        net_platform_cleanup();
        return 2;
    }

    if (opt.mode == MODE_MENU) {
        run_menu();
    } else {
        run_one_shot(&opt);
    }

    net_platform_cleanup();
    return 0;
}
