# Executable-data wave 145

Wave 145 partitions `RAW_01129F` at the first complete relocation-free ASCII
span. The 55-byte prefix contains the original relocation site and remains a
raw control prefix. The following 151 bytes are a CR-delimited level-complete
message ending at the manifest boundary; `ascii-v1` reproduces it without
adding a terminator.

The raw executable frontier is now 4,460 bytes across 13 owners. The proof is
recorded in `recipes/c/matching-wave145.json` and
`tests/test_raw_data_wave145.py`.
