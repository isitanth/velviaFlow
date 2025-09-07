#include "media.h"
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <time.h>

static int ends_with_ci(const char *s, const char *dotext) {
    size_t ls = strlen(s), le = strlen(dotext);
    if (le > ls) return 0;
    return strcasecmp(s + (ls - le), dotext) == 0;
}

bool has_ext(const char *path, const char *ext) {
    char dotext[32];
    snprintf(dotext, sizeof(dotext), ".%s", ext);
    return ends_with_ci(path, dotext);
}

media_kind_t classify_media(const char *p) {
    const char* pics[] = {"raf","jpg","jpeg","png","heic",NULL};
    for (int i=0; pics[i]; ++i) if (has_ext(p, pics[i])) return MEDIA_PICTURE;
    const char* vids[] = {"mov","mp4","avi","mkv",NULL};
    for (int i=0; vids[i]; ++i) if (has_ext(p, vids[i])) return MEDIA_VIDEO;
    return MEDIA_OTHER;
}

static int get_mtime(const char *path, struct tm *out_tm) {
    struct stat st; if (stat(path, &st) != 0) return 0;
    time_t t = st.st_mtime;
    struct tm *lt = localtime(&t);
    if (!lt) return 0;
    *out_tm = *lt;
    return 1;
}

bool month_key_from_mtime(const char *path, char out[9]) {
    struct tm tm; if (!get_mtime(path, &tm)) return false;
    return strftime(out, 9, "%Y_%m", &tm) != 0;
}

bool yyyymmdd_from_mtime(const char *path, char out[9]) {
    struct tm tm; if (!get_mtime(path, &tm)) return false;
    return strftime(out, 9, "%Y%m%d", &tm) != 0;
}

unsigned long file_size_bytes(const char *path) {
    struct stat st; if (stat(path, &st)!=0) return 0;
    return (unsigned long)st.st_size;
}