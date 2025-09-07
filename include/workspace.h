#ifndef WORKSPACE_H
#define WORKSPACE_H
#include <stdbool.h>

#define WS_META_DIR ".velviaflow"
#define WS_MARKER   "WORKSPACE"
#define WS_INBOX    "inbox"
#define WS_SORTROOT "velviaSort"

bool ws_is_initialized(const char *workdir);
bool ws_ensure_structure(const char *workdir, bool create_marker);

bool ws_path_meta(const char *workdir, char *out, unsigned out_sz);
bool ws_path_marker(const char *workdir, char *out, unsigned out_sz);
bool ws_path_inbox(const char *workdir, char *out, unsigned out_sz);
bool ws_path_sortroot(const char *workdir, char *out, unsigned out_sz);

#endif