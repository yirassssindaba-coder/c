#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "menu.h"
#include "validators.h"
#include "net_utils.h"
#include "ping_runner.h"
#include "trace_runner.h"
#include "portcheck.h"

static void print_menu(void) {
    printf("\n=== MENU ===\n");
    printf("1) Ping\n");
    printf("2) Traceroute (sederhana)\n");
    printf("3) Port Check (TCP)\n");
    printf("4) Exit\n");
    printf("Pilih: ");
}

static void read_line(char* buf, size_t cap) {
    if (!fgets(buf, (int)cap, stdin)) {
        buf[0] = '\0';
        return;
    }
    size_t n = strlen(buf);
    while (n > 0 && (buf[n-1] == '\n' || buf[n-1] == '\r')) {
        buf[n-1] = '\0';
        n--;
    }
}

static int ask_int(const char* prompt, int def_val, int min_val, int max_val) {
    char tmp[64];
    printf("%s (default %d): ", prompt, def_val);
    read_line(tmp, sizeof(tmp));
    if (tmp[0] == '\0') return def_val;
    int v = atoi(tmp);
    if (v < min_val) v = min_val;
    if (v > max_val) v = max_val;
    return v;
}

void run_menu(void) {
    char target[256] = {0};
    char resolved_ip[64] = {0};
    int has_ip = 0;

    while (1) {
        print_menu();
        char choice_buf[16];
        read_line(choice_buf, sizeof(choice_buf));
        int choice = atoi(choice_buf);

        if (choice == 4) {
            printf("Bye!\n");
            break;
        }

        printf("Masukkan Host/IP: ");
        read_line(target, sizeof(target));

        if (!is_valid_host_or_ipv4(target)) {
            printf("[ERROR] Host/IP tidak valid. Contoh: google.com atau 8.8.8.8\n");
            continue;
        }

        if (resolve_target_ipv4(target, resolved_ip, sizeof(resolved_ip), &has_ip) != 0) {
            printf("[ERROR] DNS resolve gagal atau target tidak dapat di-resolve.\n");
            continue;
        }

        printf("[NETDIAG] Target      : %s\n", target);
        if (has_ip) printf("[NETDIAG] Resolved IP : %s\n", resolved_ip);

        if (choice == 1) {
            int count = ask_int("Jumlah ping", 4, 1, 20);
            int timeout_ms = ask_int("Timeout per request (ms)", 1500, 200, 10000);

            PingSummary s;
            ping_summary_init(&s);
            int rc = run_ping_command(target, count, timeout_ms, &s);
            ping_print_summary(&s, rc);

        } else if (choice == 2) {
            int hops = ask_int("Max hops", 15, 1, 60);
            int timeout_ms = ask_int("Timeout per hop (ms)", 2000, 200, 10000);

            TraceSummary t;
            trace_summary_init(&t);
            int rc = run_trace_command(target, hops, timeout_ms, &t);
            trace_print_summary(&t, rc);

        } else if (choice == 3) {
            int port = ask_int("Port (1-65535)", 443, 1, 65535);
            int timeout_ms = ask_int("Timeout connect (ms)", 1500, 200, 10000);

            PortResult pr;
            port_result_init(&pr);
            int rc = tcp_port_check(target, port, timeout_ms, &pr);
            port_print_summary(target, resolved_ip, port, timeout_ms, &pr, rc);

        } else {
            printf("[ERROR] Pilihan tidak dikenal.\n");
        }
    }
}
