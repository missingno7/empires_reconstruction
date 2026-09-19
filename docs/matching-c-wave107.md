# Wave 107 executable data proof

Ten terminated ASCII dialog strings are now independently encoded from the
help/player raw spans. They cover 700 bytes, including carriage returns and
the original embedded line breaks. The interleaved control-byte tables and
DGROUP pointer records remain raw because they do not satisfy the strict text
format.

The promotion uses recipes/c/matching-wave107.json and the ascii-nul-v1
encoder. Every source round-trips against its original extent, and the full
EXE plus both DAT archives remain byte-identical.
