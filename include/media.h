#ifndef MEDIA_H
#define MEDIA_H
#include <stdbool.h>
#include <stdio.h>     // <-- needed for snprintf

typedef enum { MEDIA_PICTURE, MEDIA_VIDEO, MEDIA_OTHER } media_kind_t;

bool has_ext(const char *path, const char *ext);
media_kind_t classify_media(const char *path);
bool month_key_from_mtime(const char *path, char out[9]);   // "YYYY_MM"
bool yyyymmdd_from_mtime(const char *path, char out[9]);    // "YYYYMMDD"
unsigned long file_size_bytes(const char *path);

#endif