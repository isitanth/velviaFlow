#include "workspace.h"
#include "fsutil.h"
#include <sys/param.h>
#include <stdio.h>
#include <string.h>

bool ws_path_meta(const char *wd, char *out, unsigned n) {
    return snprintf(out, n, "%s/%s", wd, WS_META_DIR) < (int)n;
}
bool ws_path_marker(const char *wd, char *out, unsigned n) {
    return snprintf(out, n, "%s/%s/%s", wd, WS_META_DIR, WS_MARKER) < (int)n;
}
bool ws_path_inbox(const char *wd, char *out, unsigned n) {
    return snprintf(out, n, "%s/%s", wd, WS_INBOX) < (int)n;
}
bool ws_path_sortroot(const char *wd, char *out, unsigned n) {
    return snprintf(out, n, "%s/%s", wd, WS_SORTROOT) < (int)n;
}

bool ws_is_initialized(const char *wd) {
    char marker[PATH_MAX];
    if (!ws_path_marker(wd, marker, sizeof(marker))) return false;
    return file_exists(marker);
}

bool ws_ensure_structure(const char *wd, bool create_marker) {
    char meta[PATH_MAX], marker[PATH_MAX], inbox[PATH_MAX], sortroot[PATH_MAX];
    if (!ws_path_meta(wd, meta, sizeof(meta))) return false;
    if (!ws_path_marker(wd, marker, sizeof(marker))) return false;
    if (!ws_path_inbox(wd, inbox, sizeof(inbox))) return false;
    if (!ws_path_sortroot(wd, sortroot, sizeof(sortroot))) return false;

    if (!mkdirs_p(wd)) return false;
    if (!mkdirs_p(meta)) return false;
    if (!mkdirs_p(inbox)) return false;
    if (!mkdirs_p(sortroot)) return false;

    if (create_marker && !file_exists(marker)) {
        FILE *f = fopen(marker, "w");
        if (!f) return false;
        fputs("VelviaFlow workspace\n", f);
        fclose(f);
    }
    return true;
}