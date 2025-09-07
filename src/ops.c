#include "ops.h"
#include "media.h"
#include "fsutil.h"
#include "workspace.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>     // strcasecmp
#include <unistd.h>      // unlink
#include <sys/param.h>   // PATH_MAX (macOS)
#include <sys/stat.h>    // struct stat
#include <ftw.h>
#include <limits.h>
#include <time.h>

/// ---------- check ----------
static int g_has_sd_like_found = 0;

static int has_sd_like_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf){
    (void)sb; (void)ftwbuf;
    if (typeflag==FTW_F && classify_media(fpath)!=MEDIA_OTHER) { g_has_sd_like_found=1; return 1; }
    return 0;
}

static int has_sd_like(const char *root) {
    char dcim[PATH_MAX]; snprintf(dcim, sizeof(dcim), "%s/DCIM", root);
    if (!path_is_dir(dcim)) return 0;
    // MVP: if any media exists under DCIM, consider it an SD with photos
    g_has_sd_like_found = 0;
    nftw(dcim, has_sd_like_cb, 16, FTW_PHYS);
    return g_has_sd_like_found ? 1 : 0;
}

int op_check(const char *volumes_root) {
    FILE *p;
    char cmd[PATH_MAX+64]; snprintf(cmd, sizeof(cmd), "find '%s' -maxdepth 1 -type d -print", volumes_root);
    p = popen(cmd, "r"); if (!p) { perror("popen"); return 1; }

    char line[PATH_MAX]; int found=0, total=0;
    while (fgets(line, sizeof(line), p)) {
        size_t n=strlen(line); if (n && line[n-1]=='\n') line[n-1]='\0';
        total++;
        if (has_sd_like(line)) { printf("Detected removable volume with DCIM at %s\n", line); found=1; }
    }
    pclose(p);
    if (!found) puts("No SD-like volume detected.");
    else printf("Summary: scanned %d volume(s).\n", total);
    return 0;
}

/// ---------- start (workspace init + import) ----------
typedef struct {
    const start_opts_t *o;
    unsigned long count;
    char inbox[PATH_MAX];
    // ext filter
    char exts[16][16]; int exts_n;
} start_ctx_t;

static void parse_ext_csv(start_ctx_t *ctx, const char *csv) {
    ctx->exts_n=0;
    if (!csv || !*csv) return;
    char buf[256]; snprintf(buf, sizeof(buf), "%s", csv);
    for (char *tok=strtok(buf,","); tok && ctx->exts_n<16; tok=strtok(NULL,",")) {
        while (*tok==' ') tok++;
        snprintf(ctx->exts[ctx->exts_n++], sizeof(ctx->exts[0]), "%s", tok);
    }
}

static int ext_pass(start_ctx_t *ctx, const char *path) {
    if (ctx->o->select_all) return 1;
    const char *dot = strrchr(path,'.'); if (!dot||dot==path) return 0;
    for (int i=0;i<ctx->exts_n;i++) if (strcasecmp(dot+1, ctx->exts[i])==0) return 1;
    return 0;
}

static start_ctx_t *g_sctx = NULL;

static int start_walk_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf){
    (void)sb; (void)ftwbuf;
    if (!g_sctx) return 0;
    if (typeflag!=FTW_F) return 0;
    if (classify_media(fpath)==MEDIA_OTHER) return 0;
    if (!ext_pass(g_sctx, fpath)) return 0;

    const char *slash = strrchr(fpath,'/');
    const char *name = slash ? slash+1 : fpath;

    char uniq[NAME_MAX];
    if (!uniquify_name(g_sctx->inbox, name, uniq, sizeof(uniq))) return 0;

    char to[PATH_MAX]; snprintf(to, sizeof(to), "%s/%s", g_sctx->inbox, uniq);

    if (g_sctx->o->dry_run) {
        printf("[DRY] import %s -> %s\n", fpath, to);
    } else {
        if (!copy_or_move(fpath, to, false)) {
            fprintf(stderr, "Failed import: %s -> %s\n", fpath, to);
            return 0;
        }
    }
    g_sctx->count++;
    return 0;
}

int op_start(const start_opts_t *o) {
    if (!o || !o->src || !o->workdir) { fprintf(stderr, "-start: need --src and --workdir\n"); return 1; }
    if (!ws_ensure_structure(o->workdir, true)) {
        fprintf(stderr, "-start: cannot prepare workspace at %s\n", o->workdir);
        return 1;
    }
    start_ctx_t ctx = { .o=o, .count=0, .exts_n=0 };
    if (!ws_path_inbox(o->workdir, ctx.inbox, sizeof(ctx.inbox))) return 1;
    parse_ext_csv(&ctx, o->ext_csv);
    g_sctx = &ctx;
    if (nftw(o->src, start_walk_cb, 16, FTW_PHYS)!=0) { perror("nftw"); g_sctx=NULL; return 1; }
    g_sctx = NULL;
    printf("Start import complete: %lu file(s) %s.\n", ctx.count, o->dry_run? "planned":"imported into workspace inbox");
    return 0;
}

/// ---------- stats (workspace/inbox) ----------
typedef struct {
    unsigned long pics, vids, total;
    unsigned long long bytes_total;
} ws_stats_t;

static ws_stats_t g_wstats;

static int ws_stats_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf){
    (void)sb; (void)ftwbuf;
    if (typeflag!=FTW_F) return 0;
    media_kind_t k = classify_media(fpath);
    if (k==MEDIA_PICTURE) g_wstats.pics++;
    else if (k==MEDIA_VIDEO) g_wstats.vids++;
    else return 0;
    g_wstats.total++;
    g_wstats.bytes_total += file_size_bytes(fpath);
    return 0;
}

int op_stats_ws(const char *workdir) {
    if (!ws_is_initialized(workdir)) {
        fprintf(stderr, "-stats: workspace not initialized. Run -start first.\n");
        return 2;
    }
    char inbox[PATH_MAX]; if (!ws_path_inbox(workdir, inbox, sizeof(inbox))) return 1;
    memset(&g_wstats, 0, sizeof(g_wstats));
    if (nftw(inbox, ws_stats_cb, 16, FTW_PHYS)!=0) { perror("nftw"); return 1; }
    printf("Workspace stats (inbox)\n");
    printf("  total:   %lu\n", g_wstats.total);
    printf("  photos:  %lu\n", g_wstats.pics);
    printf("  videos:  %lu\n", g_wstats.vids);
    printf("  size:    %.2f GB\n", g_wstats.bytes_total/(1024.0*1024.0*1024.0));
    return 0;
}

/// ---------- backup (workspace/inbox -> workspace/velviaSort) ----------
typedef struct {
    const backup_opts_t *opts;
    char inbox[PATH_MAX];
    char sortroot[PATH_MAX];
    FILE *manifest;
    unsigned long count;
} bws_ctx_t;

static bws_ctx_t *g_bws = NULL;

static void now_stamp(char out[32]) {
    time_t t=time(NULL); struct tm *lt=localtime(&t);
    strftime(out, 32, "%Y%m%d_%H%M%S", lt);
}

static int bws_walk_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf){
    (void)sb; (void)ftwbuf;
    if (!g_bws) return 0;
    if (typeflag!=FTW_F) return 0;
    media_kind_t k = classify_media(fpath);
    if (k==MEDIA_OTHER) return 0;

    const char *sub = (k==MEDIA_PICTURE) ? "rawPictures" : "rawRushs";
    char month[9]; if (!month_key_from_mtime(fpath, month)) return 0;

    char outdir[PATH_MAX]; snprintf(outdir, sizeof(outdir), "%s/%s/%s", g_bws->sortroot, month, sub);
    const char *slash = strrchr(fpath,'/'); const char *name = slash?slash+1:fpath;

    char uniq[NAME_MAX];
    if (!uniquify_name(outdir, name, uniq, sizeof(uniq))) return 0;

    char to[PATH_MAX]; snprintf(to, sizeof(to), "%s/%s", outdir, uniq);

    if (g_bws->opts->dry_run) {
        printf("[DRY] %s -> %s\n", fpath, to);
    } else {
        if (!copy_or_move(fpath, to, g_bws->opts->move_instead_of_copy)) {
            fprintf(stderr, "Failed: %s -> %s\n", fpath, to);
            return 0;
        }
        if (g_bws->manifest) fprintf(g_bws->manifest, "%s\t%s\n", fpath, to);
    }
    g_bws->count++;
    return 0;
}

int op_backup_ws(const backup_opts_t *opts) {
    if (!opts || !opts->workdir) { fprintf(stderr, "-backup: need --workdir\n"); return 1; }
    if (!ws_is_initialized(opts->workdir)) {
        fprintf(stderr, "-backup: workspace not initialized. Run -start first.\n");
        return 2;
    }

    bws_ctx_t ctx = { .opts=opts, .manifest=NULL, .count=0 };
    if (!ws_path_inbox(opts->workdir, ctx.inbox, sizeof(ctx.inbox))) return 1;
    if (!ws_path_sortroot(opts->workdir, ctx.sortroot, sizeof(ctx.sortroot))) return 1;

    if (!opts->dry_run) {
        char manifest[PATH_MAX];
        if (opts->manifest_out && *opts->manifest_out) {
            snprintf(manifest, sizeof(manifest), "%s", opts->manifest_out);
        } else {
            char meta[PATH_MAX], stamp[32];
            ws_path_meta(opts->workdir, meta, sizeof(meta));
            now_stamp(stamp);
            snprintf(manifest, sizeof(manifest), "%s/manifest_%s.txt", meta, stamp);
        }
        char meta_dir[PATH_MAX]; ws_path_meta(opts->workdir, meta_dir, sizeof(meta_dir));
        if (!mkdirs_p(meta_dir)) { fprintf(stderr, "-backup: cannot create meta dir\n"); return 1; }
        ctx.manifest = fopen(manifest, "w");
        if (!ctx.manifest) { perror("manifest fopen"); return 1; }
        printf("Manifest: %s\n", manifest);
    }

    g_bws = &ctx;
    if (nftw(ctx.inbox, bws_walk_cb, 16, FTW_PHYS)!=0) { perror("nftw"); if (ctx.manifest) fclose(ctx.manifest); g_bws=NULL; return 1; }
    g_bws = NULL;
    if (ctx.manifest) fclose(ctx.manifest);

    printf("Backup complete: %lu file(s) %s.\n", ctx.count, opts->dry_run? "planned":"written");
    return 0;
}

/// ---------- cleanup (by manifest) ----------
int op_cleanup(const cleanup_opts_t *opts) {
    if (!opts || !opts->manifest_path) { fprintf(stderr, "-cleanup: need --manifest <path>\n"); return 1; }
    FILE *mf = fopen(opts->manifest_path, "r");
    if (!mf) { perror("fopen manifest"); return 1; }

    char line[PATH_MAX*2];
    unsigned long ok=0, skipped=0, missing=0, deleted=0;
    while (fgets(line, sizeof(line), mf)) {
        char *tab = strchr(line, '\t'); if (!tab) { skipped++; continue; }
        *tab = '\0';
        char *src = line;
        char *dest = tab+1;
        size_t n = strlen(dest); if (n && dest[n-1]=='\n') dest[n-1]='\0';

        if (!file_exists(src)) { skipped++; continue; }
        if (!file_exists(dest) && !opts->force) { missing++; fprintf(stderr,"WARN: dest missing, keep %s\n", src); continue; }

        if (opts->dry_run) {
            printf("[DRY] delete %s\n", src);
            ok++;
        } else {
            if (unlink(src)==0) { deleted++; ok++; }
            else { perror("unlink"); }
        }
    }
    fclose(mf);
    printf("Cleanup summary: ok=%lu, deleted=%lu, skipped=%lu, dest-missing=%lu%s\n",
           ok, deleted, skipped, missing, opts->dry_run? " (dry-run)":"");
    if (missing && !opts->force) {
        fprintf(stderr, "Some destination files were not found. Re-run with --force to override (NOT recommended).\n");
        return 2;
    }
    return 0;
}