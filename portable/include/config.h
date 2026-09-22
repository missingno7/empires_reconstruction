/* config.h -- host configuration store (empires.json).
 *
 * A flat key/value store addressed by dotted paths ("audio.volume") that
 * round-trips a small strict-JSON file of nested objects holding numbers,
 * booleans and strings.  Platform-independent (portable/compat/config.c);
 * the SDL3 front end loads it at start-up, layers environment variables and
 * command-line switches on top, and writes it back so the file is
 * discoverable on first run.  In-game settings screens are expected to write
 * through the same setters and call config_save().
 *
 * Precedence, lowest to highest: built-in defaults (config_default_*),
 * the file, environment variables, command-line switches.  Only the
 * built-in defaults and the file live in this store; the front end applies
 * env/CLI overrides to the running state without writing them back, so a
 * one-off `--volume 50` does not silently rewrite the user's file.
 *
 * Unknown keys read from the file are kept and written back unchanged.
 * Arrays and null are not supported (parse error).  No comments.
 */
#ifndef PORTABLE_CONFIG_H
#define PORTABLE_CONFIG_H

#include <stdbool.h>
#include <stddef.h>

#define CONFIG_MAX_ENTRIES   64
#define CONFIG_KEY_MAX       48
#define CONFIG_STRING_MAX    512

/* Forget every entry. */
void config_reset(void);

/* Parse `text` (NUL-terminated JSON) into the store, replacing entries
 * with the same key.  Returns false (store unchanged for the failing
 * document) and fills `err` (may be NULL) with a one-line diagnostic
 * including the byte offset. */
bool config_parse(const char *text, char *err, size_t err_len);

/* Load `path`.  Returns true on success; false when the file is missing
 * (store untouched, `*missing` set when non-NULL) or malformed (err filled
 * as for config_parse). */
bool config_load(const char *path, bool *missing, char *err, size_t err_len);

/* Serialise the store as nested JSON objects (keys sorted, 2-space
 * indent, trailing newline).  Returns the length that would be written;
 * writes at most `n - 1` characters plus a NUL when `out` is non-NULL. */
size_t config_serialize(char *out, size_t n);

/* Write the store to `path` atomically enough for a settings file
 * (temporary file + rename).  Returns false on any I/O failure. */
bool config_save(const char *path);

/* Typed access.  A getter returns `def` when the key is absent or holds a
 * value of another type.  Numbers are stored as doubles; config_get_int
 * truncates toward zero. */
bool        config_has(const char *key);
int         config_get_int(const char *key, int def);
double      config_get_number(const char *key, double def);
bool        config_get_bool(const char *key, bool def);
const char *config_get_string(const char *key, const char *def);

/* Setters replace any existing entry of the same key (whatever its type).
 * Return false when the store is full or the key/value is too long. */
bool config_set_int(const char *key, int value);
bool config_set_number(const char *key, double value);
bool config_set_bool(const char *key, bool value);
bool config_set_string(const char *key, const char *value);

/* Defaults: set only when the key is absent (so a loaded file wins).
 * Return true when the key was added, so a caller can write the file back
 * and keep it complete as new settings appear. */
bool config_default_int(const char *key, int value);
bool config_default_bool(const char *key, bool value);
bool config_default_string(const char *key, const char *value);

#endif /* PORTABLE_CONFIG_H */
