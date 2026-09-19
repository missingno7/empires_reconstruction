# Executable-data wave 144

Wave 144 decodes `RAW_010354` and `RAW_010532` as two relocation-free,
even-sized little-endian control tables. Their boundaries are already fixed by
adjacent owners, and `u16le-table-v1` reproduces all 326 bytes exactly.

The raw executable frontier is now 4,611 bytes across 13 owners. No
relocation-backed or mixed-format extent was reclassified by this wave.
