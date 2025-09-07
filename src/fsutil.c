#include "fsutil.h"
#include <sys/stat.h>
#include <sys/param.h>
#include <copyfile.h>
#include <unistd.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

bool path_is_dir(const char *p) {
    struct stat st; if (stat(p, &st)!=0) return false;
    return S_ISDIR(st.st_mode);
}

static bool mkdir_one(const char *p, mode_t mode) {
    if (mkdir(p, mode)==0) return true;
    if (errno==EEXIST && path_is_dir(p)) return true;
    return false;
}

bool mkdirs_p(const char *path) {
    char tmp[PATH_MAX]; strncpy(tmp, path, sizeof(tmp)); tmp[sizeof(tmp)-1]='\0';
    size_t len = strlen(tmp);
    if (len==0) return false;
    if (tmp[len-1]=='/') tmp[len-1]='\0';
    for (char *p = tmp+1; *p; p++) {
        if (*p=='/') { *p='\0'; if (!mkdir_one(tmp, 0775)) return false; *p='/'; }
    }
    return mkdir_one(tmp, 0775);
}

bool copy_or_move(const char *from, const char *to, bool do_move) {
    char dirbuf[PATH_MAX]; strncpy(dirbuf, to, sizeof(dirbuf)); dirbuf[sizeof(dirbuf)-1]='\0';
    char *dir = dirname(dirbuf);
    if (!mkdirs_p(dir)) return false;

    if (do_move) {
        if (rename(from, to)==0) return true;
        // cross-device fallback
        if (copyfile(from, to, NULL, COPYFILE_ALL)==0) {
            if (unlink(from)==0) return true;
        }
        return false;
    } else {
        return copyfile(from, to, NULL, COPYFILE_ALL)==0;
    }
}

bool file_exists(const char *path) {
    return access(path, F_OK)==0;
}

bool uniquify_name(const char *dir, const char *name, char *out, size_t out_sz) {
    char cand[PATH_MAX];
    snprintf(cand, sizeof(cand), "%s/%s", dir, name);
    if (!file_exists(cand)) {
        snprintf(out, out_sz, "%s", name);
        return true;
    }
    char base[PATH_MAX]; char ext[PATH_MAX]="";
    const char *dot = strrchr(name, '.');
    if (dot && dot!=name) {
        size_t stem_len = (size_t)(dot - name);
        if (stem_len >= sizeof(base)) stem_len = sizeof(base)-1;
        memcpy(base, name, stem_len); base[stem_len]='\0';
        snprintf(ext, sizeof(ext), "%s", dot);
    } else snprintf(base, sizeof(base), "%s", name);

    for (int i=1; i<=9999; ++i) {
        snprintf(cand, sizeof(cand), "%s/%s (%d)%s", dir, base, i, ext);
        if (!file_exists(cand)) {
            snprintf(out, out_sz, "%s (%d)%s", base, i, ext);
            return true;
        }
    }
    return false;
}