# Matching C wave 101

The 41-byte `F_7DD3` and 51-byte `F_8C04` spans were recovered as matching-C
continuations for the `F_7964` and `F_880A` loops. Disassembly proves that
each span contains the missing cleanup and `RET`; second OMF labels preserve
the exact split boundaries without changing the preceding loop owners.

Both continuations have subsequently been promoted to symbolic TASM. Their
six calls now use real OMF references to reconstructed entry publics; the
sources keep inherited frames, cleanup, and return values explicit. Neither
extent has a loader relocation. Full EXE and DAT equality pass, leaving 19,549
executable bytes raw at this checkpoint.
