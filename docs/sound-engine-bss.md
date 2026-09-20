# Sound-engine BSS layout

`SOUND_ENGINE_STATE_BSS` now emits its complete 890 bytes as named source
reserves. Its shape is directly constrained by existing matching-C callers:

| Offset | Storage | Evidence |
|---:|---|---|
| `000` | control byte and 9-byte enable array | `F_DA49`, `F_E420`, and `F_D8F0`; the latter clears and copies exactly nine bytes. |
| `00A` | two program-control bytes and 11 voice-note bytes | `F_DA49`, `F_E262`, `F_E420`, `F_DD72`, and `F_E48A`. |
| `017` | voice count | `F_DA20` stores the clamped count here. |
| `019` | 25 × 24-byte voice workspace | `F_DEFA` iterates exactly 25 rows with a 24-byte stride. |
| `271` | 18 × 14-byte voice-state table | the `F_E0xx` routines access the verified 14-byte records; `F_D9E1` iterates 18 voices. |
| `36D` | 11 per-voice flags and audio-mode word | `F_E48A` writes the flags and `F_D9E1` writes the final mode word. |

These are recovered source storage roles, not a claim of original variable
names or original translation-unit ownership. The canonical anchors and total
BSS ordering remain unchanged and are checked by the source-DATA and complete
structural-link receipts.
