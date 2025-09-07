#ifndef FSUTIL_H
#define FSUTIL_H
#include <stdbool.h>
#include <stddef.h>

bool path_is_dir(const char *p);
bool mkdirs_p(const char *path);
bool copy_or_move(const char *from, const char *to, bool do_move);
bool uniquify_name(const char *dir, const char *name, char *out, size_t out_sz);
bool file_exists(const char *path);

#endif