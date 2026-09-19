# Executable-data wave 132

Wave 132 removes 1,274 bytes from opaque EXE fallback as 27 independently
encoded NUL-terminated ASCII records. The sources cover runtime error text,
disk-path and disk-prompt strings, player-dialog text, menu labels and tutorial
headings. CR/LF/TAB controls are preserved as ASCII source characters, and no
mixed control-byte table or relocation-backed record is included.

The strict `ascii-nul-v1` encoder proves every extent byte for byte. The raw EXE
frontier is now 8,777 bytes across 20 owners; matching-C remains 58,895 bytes
across 345 owners with zero matching-ASM owners.
