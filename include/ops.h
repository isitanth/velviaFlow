#ifndef OPS_H
#define OPS_H
#include <stdbool.h>

// ------- Commands -------

// -check
int op_check(const char *volumes_root);

// -stats (requires workspace; scans workdir/inbox)
int op_stats_ws(const char *workdir);

// -backup (requires workspace; inbox -> velviaSort/...)
typedef struct {
    const char *workdir;
    bool move_instead_of_copy; // usually false (copy within workspace)
    bool dry_run;
    const char *manifest_out;  // optional override
} backup_opts_t;

int op_backup_ws(const backup_opts_t *opts);

// -cleanup (by manifest)
typedef struct {
    const char *manifest_path;
    bool force;
    bool dry_run;
} cleanup_opts_t;

int op_cleanup(const cleanup_opts_t *opts);

// -start (MANDATORY before stats/backup/cleanup)
typedef struct {
    const char *src;        // SD root, e.g. /Volumes/SDCARD
    const char *workdir;    // local workspace
    bool dry_run;
    bool select_all;        // import all supported media
    const char *ext_csv;    // e.g. "raf,jpg,mov" (filter), ignored if select_all
} start_opts_t;

int op_start(const start_opts_t *opts);

#endif