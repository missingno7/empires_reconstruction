/* archive.c -- portable AE000/AE001/AE002 archive access, a semantic
 * transcription of src/RESOURCE.C's resource_file_open/resource_load_record
 * family plus F_6771/F_67DC (sprite_sheet_decode_sequential/indexed).  See
 * portable/include/resource.h for the public contract and staging layout.
 *
 * Unlike the historical loader (which reopens the data file and seeks for
 * every record, retrying through a modal dialog on failure), the port loads
 * each ~230-380 KB archive into memory once and indexes it directly; the
 * *decode* side (the byte-truncation quirks, the flag/type dispatch order)
 * is kept exact.
 */
#include "resource.h"
#include "decode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <dirent.h>
#include <strings.h>
#endif

/* ---- Historical DGROUP objects this subsystem owns (state_ownership.json:
 * portable/resource) -------------------------------------------------- */
uint8_t *ui_gfx_blob = NULL;     /* DS:C5CA */
uint8_t *ui_gfx_shadow_a = NULL; /* DS:C5C6 */
uint8_t *ui_gfx_shadow_b = NULL; /* DS:C5BE */
dos_char gc0cb = 0;              /* DS:C0CB */
dos_char display_mode = 4;       /* DS:BFCD -- the port runs mode 4 ("-M") */

/* ---- Staging block (src/PLAYERSL.C ui_gfx_alloc) --------------------- */
static uint8_t *g_staging_block = NULL;

/* ---- Per-directory in-memory archive image ---------------------------- */
typedef struct {
    uint8_t *data;
    long size;
    unsigned record_count; /* N - 1, where table[0] == 4*N is the stamp */
    int is_open;
} resource_archive;

static resource_archive g_archives[3];
static char g_asset_dir[1024] = ".";

static const char *const kArchiveNames[3] = { "AE000.DAT", "AE001.DAT", "AE002.DAT" };
static const uint32_t kArchiveStamps[3] = { 360u, 528u, 32u }; /* ga52[dir] */

/* ---- Failure hook ------------------------------------------------------ */
static void default_failure_handler(int dir, const char *what)
{
    fprintf(stderr, "resource: directory %d: %s\n", dir, what);
    exit(1);
}

static resource_failure_fn g_failure_fn = default_failure_handler;

void resource_set_failure_handler(resource_failure_fn fn)
{
    g_failure_fn = fn ? fn : default_failure_handler;
}

/* ---- Asset directory / file lookup ------------------------------------ */
void resource_set_asset_dir(const char *dir)
{
    if (!dir || !*dir) {
        dir = ".";
    }
    strncpy(g_asset_dir, dir, sizeof(g_asset_dir) - 1);
    g_asset_dir[sizeof(g_asset_dir) - 1] = '\0';
}

static void join_path(char *out, size_t out_sz, const char *dir, const char *name)
{
    size_t len = strlen(dir);
    if (len > 0 && dir[len - 1] != '/' && dir[len - 1] != '\\') {
        snprintf(out, out_sz, "%s/%s", dir, name);
    } else {
        snprintf(out, out_sz, "%s%s", dir, name);
    }
}

/* Case-insensitive directory scan fallback: the historical row names carry
 * a drive prefix ("A:\\AE000.DAT"); the port drops it and looks the file up
 * by name, then case-insensitively if the exact name is not present. */
static int case_insensitive_find(const char *dir, const char *want, char *out, size_t out_sz)
{
#ifdef _WIN32
    char pattern[1024];
    WIN32_FIND_DATAA fd;
    HANDLE h;

    snprintf(pattern, sizeof(pattern), "%s\\*", dir);
    h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) {
        return -1;
    }
    do {
        if (_stricmp(fd.cFileName, want) == 0) {
            join_path(out, out_sz, dir, fd.cFileName);
            FindClose(h);
            return 0;
        }
    } while (FindNextFileA(h, &fd));
    FindClose(h);
    return -1;
#else
    DIR *d = opendir(dir);
    struct dirent *ent;

    if (!d) {
        return -1;
    }
    while ((ent = readdir(d)) != NULL) {
        if (strcasecmp(ent->d_name, want) == 0) {
            join_path(out, out_sz, dir, ent->d_name);
            closedir(d);
            return 0;
        }
    }
    closedir(d);
    return -1;
#endif
}

static int read_whole_file(const char *path, uint8_t **out_data, long *out_size)
{
    FILE *f;
    long sz;
    uint8_t *buf;

    f = fopen(path, "rb");
    if (!f) {
        return -1;
    }
    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return -1;
    }
    sz = ftell(f);
    if (sz < 0) {
        fclose(f);
        return -1;
    }
    if (fseek(f, 0, SEEK_SET) != 0) {
        fclose(f);
        return -1;
    }
    buf = (uint8_t *)malloc(sz > 0 ? (size_t)sz : 1);
    if (!buf) {
        fclose(f);
        return -1;
    }
    if (sz > 0 && fread(buf, 1, (size_t)sz, f) != (size_t)sz) {
        fclose(f);
        free(buf);
        return -1;
    }
    fclose(f);
    *out_data = buf;
    *out_size = sz;
    return 0;
}

int resource_archive_open(int dir)
{
    char path[1024];
    uint8_t *data = NULL;
    long size = 0;
    uint32_t stamp;

    if (dir < 0 || dir > 2) {
        return -1;
    }

    join_path(path, sizeof(path), g_asset_dir, kArchiveNames[dir]);
    if (read_whole_file(path, &data, &size) != 0) {
        if (case_insensitive_find(g_asset_dir, kArchiveNames[dir], path, sizeof(path)) != 0) {
            return -1;
        }
        if (read_whole_file(path, &data, &size) != 0) {
            return -1;
        }
    }

    if (size < 4) {
        free(data);
        return -1;
    }
    stamp = dos_rd32(data);
    if (stamp != kArchiveStamps[dir]) {
        free(data);
        return -1;
    }

    free(g_archives[dir].data);
    g_archives[dir].data = data;
    g_archives[dir].size = size;
    /* table[0] == 4*N; record i spans [table[i], table[i+1]) for
     * i = 0..N-2, so there are N-1 records. */
    g_archives[dir].record_count = (unsigned)(stamp / 4u) - 1u;
    g_archives[dir].is_open = 1;
    return 0;
}

const uint8_t *resource_record_raw(int dir, unsigned index, uint32_t *len)
{
    resource_archive *a;
    uint32_t o1, o2;

    if (dir < 0 || dir > 2) {
        return NULL;
    }
    a = &g_archives[dir];
    if (!a->is_open || index >= a->record_count) {
        return NULL;
    }
    o1 = dos_rd32(a->data + 4u * (uint32_t)index);
    o2 = dos_rd32(a->data + 4u * ((uint32_t)index + 1u));
    if (len) {
        *len = o2 - o1; /* full 32-bit end-start, no truncation */
    }
    return a->data + o1;
}

unsigned resource_record_count(int dir)
{
    if (dir < 0 || dir > 2 || !g_archives[dir].is_open) {
        return 0;
    }
    return g_archives[dir].record_count;
}

/* ---- Staging block (src/PLAYERSL.C ui_gfx_alloc) ----------------------- */
void resource_staging_init(void)
{
    if (g_staging_block) {
        return;
    }
    g_staging_block = (uint8_t *)malloc(UI_GFX_BLOCK_BYTES);
    if (!g_staging_block) {
        g_failure_fn(-1, "staging block allocation failure");
        return;
    }
    ui_gfx_blob = g_staging_block + UI_GFX_BLOB_OFFSET;
    ui_gfx_shadow_a = g_staging_block + UI_GFX_SHADOW_A_OFFSET;
    ui_gfx_shadow_b = g_staging_block + UI_GFX_SHADOW_B_OFFSET;
}

void resource_staging_shutdown(void)
{
    free(g_staging_block);
    g_staging_block = NULL;
    ui_gfx_blob = NULL;
    ui_gfx_shadow_a = NULL;
    ui_gfx_shadow_b = NULL;
}

/* ---- F_6771 -- sprite_sheet_decode_sequential -------------------------- */
/* `for(i=0;i<n;){q=p+i;if(*q!=0x47)break; ...; i+=q[34]*q[35]+36;}` -- the
 * step is TC 2.0 `int` (16-bit) arithmetic on two unsigned chars: the
 * product can exceed 32767 and wrap as a signed 16-bit value before the
 * +36 and the addition into `i`. */
static void sprite_sheet_decode_sequential(uint8_t *p, dos_uint n)
{
    dos_uint i;

    for (i = 0; i < n;) {
        uint8_t *q = p + i;
        dos_int step;

        if (*q != 0x47) {
            break;
        }
        if (display_mode == 2) {
            sprite_decode_4bpp_mode13h(q + 2);
        } else {
            sprite_decode_4bpp_planar(q + 2);
        }
        step = dos_add16(dos_mul16((dos_int)q[34], (dos_int)q[35]), 36);
        i = dos_uadd16(i, dos_u16(step));
    }
}

/* ---- F_67DC -- sprite_sheet_decode_indexed ------------------------------ */
/* `t=(unsigned *)p; n=(*t>>1)-1; for(...){q=p+*t;t++; ...}` -- t walks a
 * table of little-endian 16-bit offsets at the front of p; read via
 * dos_rd16 rather than reinterpreting p as uint16_t* (dos_types.h: never
 * reinterpret buffers through wider pointer types). */
static void sprite_sheet_decode_indexed(uint8_t *p)
{
    dos_int i, n;
    dos_uint t_off;

    n = dos_i16((int32_t)(dos_rd16(p) >> 1) - 1);
    t_off = 0;
    for (i = 0; i < n; i++) {
        dos_uint off = dos_rd16(p + t_off);
        uint8_t *q = p + off;

        t_off = dos_uadd16(t_off, 2);
        if (*q == 0x47) {
            if (display_mode == 2) {
                sprite_decode_4bpp_mode13h(q + 2);
            } else {
                sprite_decode_4bpp_planar(q + 2);
            }
        }
    }
}

/* ---- F_656C -- resource_load_record ------------------------------------ */
dos_int resource_load_record(dos_uint packed)
{
    dos_uint d = (dos_uint)(packed >> 12);
    dos_uint p = (dos_uint)(packed & 0xfffu);
    resource_archive *a;
    uint32_t o1, o2;
    dos_int s;
    dos_uint copy_len;
    dos_char fl;

    if (d > 2u) {
        g_failure_fn((int)d, "invalid directory index");
        return 0;
    }
    if (!g_archives[d].is_open) {
        if (resource_archive_open((int)d) != 0) {
            g_failure_fn((int)d, "resource_archive_open failed");
            return 0;
        }
    }
    a = &g_archives[d];
    if (p >= a->record_count) {
        g_failure_fn((int)d, "record index out of range");
        return 0;
    }

    o1 = dos_rd32(a->data + 4u * (uint32_t)p);
    o2 = dos_rd32(a->data + 4u * ((uint32_t)p + 1u));
    /* `s = (int)o2 - (int)o1` -- 16-bit subtraction of the two longs'
     * truncated low words (dos_types.h cites this exact site). */
    s = dos_sub16(dos_i16((int32_t)o2), dos_i16((int32_t)o1));
    copy_len = dos_u16(s);

    /* read(slot_file_handle, ui_gfx_blob, s) -- ui_gfx_blob[0..1] becomes
     * the header, and ui_gfx_blob+2 == ui_gfx_shadow_a (see resource.h),
     * so this one copy populates both. */
    memcpy(ui_gfx_blob, a->data + o1, copy_len);

    gc0cb = (dos_char)ui_gfx_blob[0];
    fl = (dos_char)ui_gfx_blob[1];
    s = dos_sub16(s, 2);

    if ((fl & 2) && (fl & 1)) {
        s = (dos_int)lz_decompress(ui_gfx_shadow_a, ui_gfx_shadow_b, dos_u16(s));
        s = (dos_int)rle_packbits_decode(ui_gfx_shadow_b, ui_gfx_shadow_a, dos_u16(s));
    } else if (fl & 2) {
        s = (dos_int)lz_decompress(ui_gfx_shadow_a, ui_gfx_shadow_b, dos_u16(s));
        memmove(ui_gfx_shadow_a, ui_gfx_shadow_b, dos_u16(s));
    } else if (fl & 1) {
        s = (dos_int)rle_packbits_decode(ui_gfx_shadow_a, ui_gfx_shadow_b, dos_u16(s));
        memmove(ui_gfx_shadow_a, ui_gfx_shadow_b, dos_u16(s));
    }

    if (display_mode != 5) {
        switch (gc0cb) {
        case 0x47:
            if (display_mode == 2) {
                sprite_decode_4bpp_mode13h(ui_gfx_shadow_a);
            } else {
                sprite_decode_4bpp_planar(ui_gfx_shadow_a);
            }
            break;
        case 0:
            sprite_sheet_decode_sequential(ui_gfx_shadow_a, dos_u16(s));
            break;
        case 1:
            sprite_sheet_decode_indexed(ui_gfx_shadow_a);
            break;
        default:
            break;
        }
    }

    return s;
}

/* ---- F_684A -- resource_load_record_alloc ------------------------------ */
void resource_load_record_alloc(dos_int packed, uint8_t **pp)
{
    dos_int n = resource_load_record((dos_uint)packed);
    dos_uint len = dos_u16(n);
    uint8_t *buf = (uint8_t *)malloc(len > 0 ? (size_t)len : 1);

    if (!buf) {
        g_failure_fn((int)((dos_uint)packed >> 12), "resource_load_record_alloc allocation failure");
        *pp = NULL;
        return;
    }
    memcpy(buf, ui_gfx_shadow_a, len);
    *pp = buf;
}

/* ---- F_68AA -- resource_load_record_into ------------------------------- */
void resource_load_record_into(dos_uint packed, uint8_t *q)
{
    dos_int n = resource_load_record(packed);
    memcpy(q, ui_gfx_shadow_a, dos_u16(n));
}
