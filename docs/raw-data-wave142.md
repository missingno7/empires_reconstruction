# Executable-data wave 142

`RAW_011037` is now a structured `ascii-nul-v1` source. Its 51 bytes are one
terminated player-name instruction record, including the historical CR and
control-key bytes `0x17 0x18`; there is no embedded zero or relocation-backed
binary tail. The encoder reproduces the original extent byte for byte.

This removes 51 bytes from the raw executable fallback while leaving all
code ownership unchanged. The remaining raw frontier is 5,036 bytes across
15 owners.

The proof is reproducible with `tests/test_raw_data_wave142.py` and the
recipe in `recipes/c/matching-wave142.json`.
