# Executable-data wave 143

Wave 143 partitions the leading 99-byte terminated dialog record from
`RAW_0106F7`. The text includes the historical CR separators and control-key
bytes `0x17 0x18`, and is reproduced by `ascii-nul-v1`.

The remaining 20 bytes are retained as `RAW_01075A`, a separate
relocation-backed control tail. Its boundary is explicit, but its record
format is not yet established, so it remains raw rather than being folded
into the text source.

The raw executable frontier is now 4,937 bytes across 15 owners. The proof is
recorded in `recipes/c/matching-wave143.json` and
`tests/test_raw_data_wave143.py`.
