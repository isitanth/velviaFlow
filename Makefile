# --- Makefile (fix quotes & shell) ---

SHELL    := /bin/bash

APP      = velviaflow
CC       = clang
CFLAGS   = -std=c17 -O2 -Wall -Wextra -Werror \
           -mmacosx-version-min=11.0 -target arm64-apple-macos11 \
           -DAPP_VERSION='"1.0"' \
           -DAPP_AUTHOR='"Anth"' \
           -DAPP_LICENSE_NAME='"MIT license"' \
           -DAPP_LICENSE_URL='"https://mit-license.org/"' \
           -DAPP_HOMEPAGE='"https://github.com/isitanth"' \
           -DAPP_COPYRIGHT='"© 2025 Anth"'
LDFLAGS  =
INCLUDES = -Iinclude

SRC = src/main.c src/cli.c src/media.c src/fsutil.c src/ops.c src/workspace.c
OBJ = $(SRC:.c=.o)

all: $(APP)

$(APP): $(OBJ)
	$(CC) $(CFLAGS) -o "$@" $(OBJ) $(LDFLAGS)

%.o: %.c include/*.h
	$(CC) $(CFLAGS) $(INCLUDES) -c "$<" -o "$@"

clean:
	rm -f $(OBJ) $(APP)

install: $(APP)
	install -m 0755 "$(APP)" /usr/local/bin/$(APP)

.PHONY: all clean install