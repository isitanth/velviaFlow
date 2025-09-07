#ifndef VELVIA_VERSION_H
#define VELVIA_VERSION_H

#include <stdio.h>

/* ------- Build-time defines (fallbacks if not provided by Makefile) ------- */
#ifndef APP_VERSION
#define APP_VERSION "dev"
#endif
#ifndef APP_AUTHOR
#define APP_AUTHOR "Unknown"
#endif
#ifndef APP_LICENSE_NAME
#define APP_LICENSE_NAME "Noncommercial"
#endif
#ifndef APP_LICENSE_URL
#define APP_LICENSE_URL "https://example.com/license"
#endif
#ifndef APP_HOMEPAGE
#define APP_HOMEPAGE "https://example.com"
#endif
#ifndef APP_COPYRIGHT
#define APP_COPYRIGHT "(c) 2025"
#endif

/* ------- Optional ANSI colors (disable with -DNO_COLOR) ------- */
#if defined(NO_COLOR)
#  define C_RESET ""
#  define C_DIM   ""
#  define C_BOLD  ""
#else
#  define C_RESET "\x1b[0m"
#  define C_DIM   "\x1b[2m"
#  define C_BOLD  "\x1b[1m"
#endif

/* ------- ASCII banner ------- */
static inline void print_ascii_banner(void) {
    /* Keep lines under ~80 cols for safe rendering in most terminals */
    puts(
"           _       _       _____ _\n"
"__   _____| |_   _(_) __ _|  ___| | _____      __\n"
"\\ \\ / / _ \\ \\ \\ / / |/ _` | |_  | |/ _ \\ \\ /\\ / /\n"
" \\ V /  __/ |\\ V /| | (_| |  _| | | (_) \\ V  V /\n"
"  \\_/ \\___|_| \\_/ |_|\\__,_|_|   |_|\\___/  \\_/\\_/\n"
    );
}

/* ------- OSS-style version banner ------- */
static inline void print_version_banner(void) {
    print_ascii_banner();
    printf(
        C_BOLD "velviaflow %s" C_RESET "\n"
        "%s\n"
        "Author:      %s\n"
        "License:     %s\n"
        "License URL: %s\n"
        "Homepage:    %s\n"
        C_DIM "Build:       %s %s (macOS, arm64)" C_RESET "\n",
        APP_VERSION,
        APP_COPYRIGHT,
        APP_AUTHOR,
        APP_LICENSE_NAME,
        APP_LICENSE_URL,
        APP_HOMEPAGE,
        __DATE__, __TIME__
    );
}

#endif /* VELVIA_VERSION_H */