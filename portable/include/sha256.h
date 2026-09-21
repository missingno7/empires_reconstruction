/* sha256.h -- tiny public-domain-style SHA-256, used only by portable
 * tests to check decoded records against portable/tests/fixtures/
 * resource_golden.json.  Not part of the game/runtime code path.
 */
#ifndef PORTABLE_SHA256_H
#define PORTABLE_SHA256_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t state[8];
    uint64_t bitlen;
    uint8_t buf[64];
    size_t buflen;
} sha256_ctx;

void sha256_init(sha256_ctx *ctx);
void sha256_update(sha256_ctx *ctx, const void *data, size_t len);
/* Writes 32 raw digest bytes to `out`. */
void sha256_final(sha256_ctx *ctx, uint8_t out[32]);

/* Convenience one-shot: writes a 65-byte (64 hex chars + NUL) lowercase hex
 * string to `hex_out`. */
void sha256_hex(const void *data, size_t len, char hex_out[65]);

#endif
