CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -std=c99

SRC = src/main.c src/menu.c src/validators.c src/net_utils.c src/ping_runner.c src/trace_runner.c src/portcheck.c
OUT = netdiag

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) -o $(OUT) $(SRC)

clean:
	rm -f $(OUT)

.PHONY: all clean
