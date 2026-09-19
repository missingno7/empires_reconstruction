# Matching C wave 94

Twenty complete small routines in the raw `0xC4EA`–`0xCB3B` code span now
have independent matching-C sources. Each source preserves its entire
return-terminated instruction sequence, and each fresh object has no external
fixups or loader relocations.

The promoted owners are `F_C2EA`, `F_C359`, `F_C3DB`, `F_C440`, `F_C501`,
`F_C549`, `F_C5A8`, `F_C5B3`, `F_C5C6`, `F_C5D1`, `F_C678`, `F_C6B9`,
`F_C706`, `F_C755`, `F_C77A`, `F_C7CB`, `F_C9A4`, `F_CA03`, `F_CA35`, and
`F_CA51`. Together they add 1,512 matching-C bytes.

`F_C567` was deliberately left out because its upstream extent crosses the
already owned `F_C59A` boundary; the promotion tool rejects that overlap
instead of claiming an incomplete source. The full EXE and both DAT archives
remain exact, with 23,521 executable bytes still raw.
