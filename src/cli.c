#include "cli.h"
#include "ops.h"
#include "version.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

static void usage(void) {
    puts("velviaflow — Fujifilm SD photo workflow (macOS, arm64)");
    puts("");
    puts("USAGE");
    puts("  velviaflow -v | -version | --version");
    puts("  velviaflow -check [--src /Volumes]");
    puts("  velviaflow -start  --src <sd_root> --workdir <dir> [--all | --ext raf,jpg,mov] [--dry-run]");
    puts("  velviaflow -stats  --workdir <dir>");
    puts("  velviaflow -backup --workdir <dir> [--dry-run] [--manifest <file>]");
    puts("  velviaflow -cleanup --manifest <file> [--force] [--dry-run]");
    puts("");
    puts("NOTES");
    puts("  -start is mandatory before -stats/-backup/-cleanup.");
    puts("  Workspace layout: <workdir>/{.velviaflow, inbox/, velviaSort/}");
}

int run_cli(int argc, char **argv) {
    if (argc < 2) { usage(); return 1; }

    // Version flags (-v, -version, --version) — all equivalent
    if (!strcmp(argv[1], "-v") || !strcmp(argv[1], "-version") || !strcmp(argv[1], "--version")) {
        print_version_banner();
        return 0;
    }

    if (!strcmp(argv[1], "-check")) {
        const char *src="/Volumes";
        for (int i=2;i<argc;i++) if (!strcmp(argv[i],"--src") && i+1<argc) src=argv[++i];
        return op_check(src);
    }

    if (!strcmp(argv[1], "-start")) {
        start_opts_t o = {0};
        for (int i=2;i<argc;i++) {
            if (!strcmp(argv[i],"--src") && i+1<argc) o.src=argv[++i];
            else if (!strcmp(argv[i],"--workdir") && i+1<argc) o.workdir=argv[++i];
            else if (!strcmp(argv[i],"--all")) o.select_all=true;
            else if (!strcmp(argv[i],"--ext") && i+1<argc) o.ext_csv=argv[++i];
            else if (!strcmp(argv[i],"--dry-run")) o.dry_run=true;
        }
        if (!o.src || !o.workdir) { usage(); return 1; }
        if (!o.select_all && !o.ext_csv) o.select_all = true; // default: import all media
        return op_start(&o);
    }

    if (!strcmp(argv[1], "-stats")) {
        const char *workdir=NULL;
        for (int i=2;i<argc;i++) if (!strcmp(argv[i],"--workdir") && i+1<argc) workdir=argv[++i];
        if (!workdir) { usage(); return 1; }
        return op_stats_ws(workdir);
    }

    if (!strcmp(argv[1], "-backup")) {
        backup_opts_t o = {0};
        for (int i=2;i<argc;i++) {
            if (!strcmp(argv[i],"--workdir") && i+1<argc) o.workdir=argv[++i];
            else if (!strcmp(argv[i],"--dry-run")) o.dry_run=true;
            else if (!strcmp(argv[i],"--manifest") && i+1<argc) o.manifest_out=argv[++i];
        }
        if (!o.workdir) { usage(); return 1; }
        return op_backup_ws(&o);
    }

    if (!strcmp(argv[1], "-cleanup")) {
        cleanup_opts_t o = {0};
        for (int i=2;i<argc;i++) {
            if (!strcmp(argv[i],"--manifest") && i+1<argc) o.manifest_path=argv[++i];
            else if (!strcmp(argv[i],"--force")) o.force=true;
            else if (!strcmp(argv[i],"--dry-run")) o.dry_run=true;
        }
        if (!o.manifest_path) { usage(); return 1; }
        return op_cleanup(&o);
    }

    usage();
    return 1;
}