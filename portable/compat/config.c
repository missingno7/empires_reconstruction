/* config.c -- host configuration store; see portable/include/config.h.
 *
 * Deliberately tiny: a fixed-capacity array of (dotted key, type, value)
 * and a recursive-descent parser for the JSON subset the file uses
 * (nested objects of numbers, booleans and strings).  No allocation.
 */
#include "config.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum config_type { CT_NUMBER, CT_BOOL, CT_STRING };

struct config_entry {
    char key[CONFIG_KEY_MAX];
    enum config_type type;
    double number;
    bool boolean;
    char string[CONFIG_STRING_MAX];
};

static struct config_entry s_entries[CONFIG_MAX_ENTRIES];
static int s_count;

/* ---- store ---------------------------------------------------------- */

void config_reset(void)
{
    s_count = 0;
}

static struct config_entry *find(const char *key)
{
    for (int i = 0; i < s_count; i++)
        if (strcmp(s_entries[i].key, key) == 0)
            return &s_entries[i];
    return NULL;
}

static struct config_entry *find_or_add(const char *key)
{
    struct config_entry *e = find(key);
    if (e)
        return e;
    if (s_count >= CONFIG_MAX_ENTRIES || strlen(key) >= CONFIG_KEY_MAX || key[0] == '\0')
        return NULL;
    e = &s_entries[s_count++];
    memset(e, 0, sizeof *e);
    snprintf(e->key, sizeof e->key, "%s", key);
    return e;
}

bool config_has(const char *key)
{
    return find(key) != NULL;
}

int config_get_int(const char *key, int def)
{
    const struct config_entry *e = find(key);
    if (!e || e->type != CT_NUMBER)
        return def;
    double v = e->number;
    if (v >= 2147483647.0) return 2147483647;
    if (v <= -2147483648.0) return (int)-2147483647 - 1;
    return (int)v;   /* truncates toward zero */
}

double config_get_number(const char *key, double def)
{
    const struct config_entry *e = find(key);
    return (e && e->type == CT_NUMBER) ? e->number : def;
}

bool config_get_bool(const char *key, bool def)
{
    const struct config_entry *e = find(key);
    return (e && e->type == CT_BOOL) ? e->boolean : def;
}

const char *config_get_string(const char *key, const char *def)
{
    const struct config_entry *e = find(key);
    return (e && e->type == CT_STRING) ? e->string : def;
}

bool config_set_number(const char *key, double value)
{
    struct config_entry *e = find_or_add(key);
    if (!e)
        return false;
    e->type = CT_NUMBER;
    e->number = value;
    return true;
}

bool config_set_int(const char *key, int value)
{
    return config_set_number(key, (double)value);
}

bool config_set_bool(const char *key, bool value)
{
    struct config_entry *e = find_or_add(key);
    if (!e)
        return false;
    e->type = CT_BOOL;
    e->boolean = value;
    return true;
}

bool config_set_string(const char *key, const char *value)
{
    struct config_entry *e;
    if (!value || strlen(value) >= CONFIG_STRING_MAX)
        return false;
    e = find_or_add(key);
    if (!e)
        return false;
    e->type = CT_STRING;
    snprintf(e->string, sizeof e->string, "%s", value);
    return true;
}

void config_default_int(const char *key, int value)
{
    if (!find(key)) config_set_int(key, value);
}

void config_default_bool(const char *key, bool value)
{
    if (!find(key)) config_set_bool(key, value);
}

void config_default_string(const char *key, const char *value)
{
    if (!find(key)) config_set_string(key, value);
}

/* ---- parser --------------------------------------------------------- */

struct parser {
    const char *text;
    size_t pos;
    char *err;
    size_t err_len;
    bool failed;
};

static void fail(struct parser *p, const char *what)
{
    if (!p->failed && p->err && p->err_len)
        snprintf(p->err, p->err_len, "config: %s at byte %zu", what, p->pos);
    p->failed = true;
}

static void skip_ws(struct parser *p)
{
    while (p->text[p->pos] == ' ' || p->text[p->pos] == '\t' ||
           p->text[p->pos] == '\r' || p->text[p->pos] == '\n')
        p->pos++;
}

static bool expect(struct parser *p, char c, const char *what)
{
    skip_ws(p);
    if (p->text[p->pos] != c) {
        fail(p, what);
        return false;
    }
    p->pos++;
    return true;
}

static int hex_val(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

/* Parses a JSON string (opening quote already consumed is NOT assumed:
 * the current char must be '"').  \uXXXX is emitted as UTF-8 (surrogate
 * pairs combined). */
static bool parse_string(struct parser *p, char *out, size_t n)
{
    size_t len = 0;
    if (!expect(p, '"', "expected string"))
        return false;
    for (;;) {
        char c = p->text[p->pos];
        unsigned cp;
        if (c == '\0') { fail(p, "unterminated string"); return false; }
        p->pos++;
        if (c == '"')
            break;
        if (c == '\\') {
            char e = p->text[p->pos++];
            switch (e) {
            case '"': c = '"'; break;
            case '\\': c = '\\'; break;
            case '/': c = '/'; break;
            case 'b': c = '\b'; break;
            case 'f': c = '\f'; break;
            case 'n': c = '\n'; break;
            case 'r': c = '\r'; break;
            case 't': c = '\t'; break;
            case 'u': {
                int h[4];
                for (int i = 0; i < 4; i++) {
                    h[i] = hex_val(p->text[p->pos]);
                    if (h[i] < 0) { fail(p, "bad \\u escape"); return false; }
                    p->pos++;
                }
                cp = (unsigned)((h[0] << 12) | (h[1] << 8) | (h[2] << 4) | h[3]);
                if (cp >= 0xD800 && cp <= 0xDBFF && p->text[p->pos] == '\\' && p->text[p->pos + 1] == 'u') {
                    int l[4];
                    unsigned lo;
                    p->pos += 2;
                    for (int i = 0; i < 4; i++) {
                        l[i] = hex_val(p->text[p->pos]);
                        if (l[i] < 0) { fail(p, "bad \\u escape"); return false; }
                        p->pos++;
                    }
                    lo = (unsigned)((l[0] << 12) | (l[1] << 8) | (l[2] << 4) | l[3]);
                    cp = 0x10000u + ((cp - 0xD800u) << 10) + (lo - 0xDC00u);
                }
                {
                    char buf[4];
                    int k = 0;
                    if (cp < 0x80) buf[k++] = (char)cp;
                    else if (cp < 0x800) { buf[k++] = (char)(0xC0 | (cp >> 6)); buf[k++] = (char)(0x80 | (cp & 0x3F)); }
                    else if (cp < 0x10000) { buf[k++] = (char)(0xE0 | (cp >> 12)); buf[k++] = (char)(0x80 | ((cp >> 6) & 0x3F)); buf[k++] = (char)(0x80 | (cp & 0x3F)); }
                    else { buf[k++] = (char)(0xF0 | (cp >> 18)); buf[k++] = (char)(0x80 | ((cp >> 12) & 0x3F)); buf[k++] = (char)(0x80 | ((cp >> 6) & 0x3F)); buf[k++] = (char)(0x80 | (cp & 0x3F)); }
                    if (len + (size_t)k >= n) { fail(p, "string too long"); return false; }
                    memcpy(out + len, buf, (size_t)k);
                    len += (size_t)k;
                }
                continue;
            }
            default:
                fail(p, "bad escape");
                return false;
            }
        }
        if (len + 1 >= n) { fail(p, "string too long"); return false; }
        out[len++] = c;
    }
    out[len] = '\0';
    return true;
}

static bool parse_value(struct parser *p, const char *key);

static bool parse_object(struct parser *p, const char *prefix)
{
    if (!expect(p, '{', "expected '{'"))
        return false;
    skip_ws(p);
    if (p->text[p->pos] == '}') { p->pos++; return true; }
    for (;;) {
        char name[CONFIG_KEY_MAX];
        char key[CONFIG_KEY_MAX];
        skip_ws(p);
        if (!parse_string(p, name, sizeof name))
            return false;
        if (name[0] == '\0' || strchr(name, '.')) { fail(p, "bad key"); return false; }
        if (prefix[0])
            snprintf(key, sizeof key, "%s.%s", prefix, name);
        else
            snprintf(key, sizeof key, "%s", name);
        if (strlen(prefix) + strlen(name) + 2 > sizeof key) { fail(p, "key too long"); return false; }
        if (!expect(p, ':', "expected ':'"))
            return false;
        if (!parse_value(p, key))
            return false;
        skip_ws(p);
        if (p->text[p->pos] == ',') { p->pos++; continue; }
        if (p->text[p->pos] == '}') { p->pos++; return true; }
        fail(p, "expected ',' or '}'");
        return false;
    }
}

static bool parse_value(struct parser *p, const char *key)
{
    skip_ws(p);
    char c = p->text[p->pos];
    if (c == '{')
        return parse_object(p, key);
    if (c == '"') {
        char s[CONFIG_STRING_MAX];
        if (!parse_string(p, s, sizeof s))
            return false;
        if (!config_set_string(key, s)) { fail(p, "too many entries"); return false; }
        return true;
    }
    if (strncmp(p->text + p->pos, "true", 4) == 0) {
        p->pos += 4;
        if (!config_set_bool(key, true)) { fail(p, "too many entries"); return false; }
        return true;
    }
    if (strncmp(p->text + p->pos, "false", 5) == 0) {
        p->pos += 5;
        if (!config_set_bool(key, false)) { fail(p, "too many entries"); return false; }
        return true;
    }
    if (c == '-' || (c >= '0' && c <= '9')) {
        char *end;
        double v = strtod(p->text + p->pos, &end);
        if (end == p->text + p->pos) { fail(p, "bad number"); return false; }
        p->pos = (size_t)(end - p->text);
        if (!config_set_number(key, v)) { fail(p, "too many entries"); return false; }
        return true;
    }
    if (c == '[') { fail(p, "arrays are not supported"); return false; }
    if (strncmp(p->text + p->pos, "null", 4) == 0) { fail(p, "null is not supported"); return false; }
    fail(p, "unexpected character");
    return false;
}

bool config_parse(const char *text, char *err, size_t err_len)
{
    struct parser p = { text, 0, err, err_len, false };
    struct config_entry backup[CONFIG_MAX_ENTRIES];
    int backup_count = s_count;
    memcpy(backup, s_entries, sizeof backup);
    if (err && err_len)
        err[0] = '\0';

    if (parse_object(&p, "")) {
        skip_ws(&p);
        if (p.text[p.pos] != '\0')
            fail(&p, "trailing characters");
    }
    if (p.failed) {
        memcpy(s_entries, backup, sizeof backup);
        s_count = backup_count;
        return false;
    }
    return true;
}

bool config_load(const char *path, bool *missing, char *err, size_t err_len)
{
    FILE *f = fopen(path, "rb");
    char *buf;
    long size;
    bool ok;

    if (missing)
        *missing = false;
    if (err && err_len)
        err[0] = '\0';
    if (!f) {
        if (missing)
            *missing = true;
        return false;
    }
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size < 0 || size > 1024 * 1024) {
        fclose(f);
        if (err && err_len)
            snprintf(err, err_len, "config: %s: unreasonable size", path);
        return false;
    }
    buf = (char *)malloc((size_t)size + 1);
    if (!buf) {
        fclose(f);
        return false;
    }
    size = (long)fread(buf, 1, (size_t)size, f);
    fclose(f);
    buf[size] = '\0';
    /* tolerate a UTF-8 BOM */
    ok = config_parse((size >= 3 && (unsigned char)buf[0] == 0xEF &&
                       (unsigned char)buf[1] == 0xBB && (unsigned char)buf[2] == 0xBF) ? buf + 3 : buf,
                      err, err_len);
    free(buf);
    return ok;
}

/* ---- writer --------------------------------------------------------- */

struct writer {
    char *out;
    size_t n;
    size_t len;
};

static void put(struct writer *w, const char *s)
{
    size_t l = strlen(s);
    if (w->out && w->len < w->n) {
        size_t room = w->n - 1 - w->len;
        size_t k = l < room ? l : room;
        memcpy(w->out + w->len, s, k);
        w->out[w->len + k] = '\0';
    }
    w->len += l;
}

static void put_indent(struct writer *w, int depth)
{
    while (depth-- > 0)
        put(w, "  ");
}

static void put_string(struct writer *w, const char *s)
{
    char buf[8];
    put(w, "\"");
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        switch (c) {
        case '"': put(w, "\\\""); break;
        case '\\': put(w, "\\\\"); break;
        case '\b': put(w, "\\b"); break;
        case '\f': put(w, "\\f"); break;
        case '\n': put(w, "\\n"); break;
        case '\r': put(w, "\\r"); break;
        case '\t': put(w, "\\t"); break;
        default:
            if (c < 0x20) {
                snprintf(buf, sizeof buf, "\\u%04x", c);
                put(w, buf);
            } else {
                buf[0] = (char)c; buf[1] = '\0';
                put(w, buf);
            }
        }
    }
    put(w, "\"");
}

static void put_number(struct writer *w, double v)
{
    char buf[64];
    if (v == floor(v) && fabs(v) < 1e15)
        snprintf(buf, sizeof buf, "%.0f", v);
    else
        snprintf(buf, sizeof buf, "%.17g", v);
    put(w, buf);
}

static int cmp_entry(const void *a, const void *b)
{
    return strcmp(((const struct config_entry *)a)->key, ((const struct config_entry *)b)->key);
}

/* Number of leading dotted components shared by a and b. */
static int shared_prefix_depth(const char *a, const char *b)
{
    int depth = 0;
    size_t i = 0;
    for (;;) {
        if (a[i] != b[i])
            return depth;
        if (a[i] == '\0')
            return depth;
        if (a[i] == '.')
            depth++;
        i++;
    }
}

static int key_depth(const char *k)
{
    int d = 0;
    for (; *k; k++)
        if (*k == '.') d++;
    return d;
}

static const char *component(const char *k, int index, char *buf, size_t n)
{
    const char *start = k;
    while (index-- > 0) {
        start = strchr(start, '.');
        if (!start)
            return "";
        start++;
    }
    {
        const char *end = strchr(start, '.');
        size_t l = end ? (size_t)(end - start) : strlen(start);
        if (l >= n) l = n - 1;
        memcpy(buf, start, l);
        buf[l] = '\0';
    }
    return buf;
}

size_t config_serialize(char *out, size_t n)
{
    struct config_entry sorted[CONFIG_MAX_ENTRIES];
    struct writer w = { out, n, 0 };
    char comp[CONFIG_KEY_MAX];
    int open_depth = 0;   /* number of nested objects currently open */

    if (out && n)
        out[0] = '\0';
    memcpy(sorted, s_entries, sizeof(sorted[0]) * (size_t)s_count);
    qsort(sorted, (size_t)s_count, sizeof sorted[0], cmp_entry);

    put(&w, "{");
    for (int i = 0; i < s_count; i++) {
        const struct config_entry *e = &sorted[i];
        int depth = key_depth(e->key);
        int keep = i ? shared_prefix_depth(sorted[i - 1].key, e->key) : 0;
        if (keep > depth) keep = depth;   /* a leaf can't be a prefix of another key, but be safe */

        /* close objects deeper than the shared prefix */
        while (open_depth > keep) {
            put(&w, "\n");
            put_indent(&w, open_depth);
            put(&w, "}");
            open_depth--;
        }
        if (i)
            put(&w, ",");
        /* open the objects this key needs */
        while (open_depth < depth) {
            put(&w, "\n");
            put_indent(&w, open_depth + 1);
            put_string(&w, component(e->key, open_depth, comp, sizeof comp));
            put(&w, ": {");
            open_depth++;
        }
        put(&w, "\n");
        put_indent(&w, depth + 1);
        put_string(&w, component(e->key, depth, comp, sizeof comp));
        put(&w, ": ");
        switch (e->type) {
        case CT_NUMBER: put_number(&w, e->number); break;
        case CT_BOOL: put(&w, e->boolean ? "true" : "false"); break;
        case CT_STRING: put_string(&w, e->string); break;
        }
    }
    while (open_depth > 0) {
        put(&w, "\n");
        put_indent(&w, open_depth);
        put(&w, "}");
        open_depth--;
    }
    put(&w, s_count ? "\n}\n" : "}\n");
    return w.len;
}

bool config_save(const char *path)
{
    size_t len = config_serialize(NULL, 0);
    char *buf = (char *)malloc(len + 1);
    char tmp[1024];
    FILE *f;
    bool ok;

    if (!buf)
        return false;
    config_serialize(buf, len + 1);
    snprintf(tmp, sizeof tmp, "%s.tmp", path);
    f = fopen(tmp, "wb");
    if (!f) {
        free(buf);
        return false;
    }
    ok = fwrite(buf, 1, len, f) == len;
    ok = (fclose(f) == 0) && ok;
    free(buf);
    if (!ok) {
        remove(tmp);
        return false;
    }
    remove(path);              /* MSVC's rename() refuses to overwrite */
    if (rename(tmp, path) != 0) {
        remove(tmp);
        return false;
    }
    return true;
}
