# F_9EC3 symbolic clipped-row copy

`F_9EC3` is now a 125-byte symbolic TASM owner. It selects indexed far source
and destination rows, translates the row endpoints through two fixed lookup
tables, and copies the clipped sixteen-byte spans.

The source preserves its four structural fixups: two offset fixups to the far
pointer table and two DGROUP-base fixups used while translating through the
lookup tables. The routine remains at its existing DATA/code ordering boundary;
the boundary is not claimed as a recovered historical translation unit.
