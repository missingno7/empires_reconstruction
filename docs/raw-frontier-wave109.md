# Raw executable frontier after wave 108

Wave 139 removes 11 bytes of isolated zero-initialized data after wave 138's
187 bytes of fixed records and terminal padding. Earlier waves removed wave
137's 28 bytes of paired pointer attributes, wave 136's 56 bytes of pointer
table data, wave 135's 3,155-byte all-zero region, and wave 134's 99 bytes of
zero runs and terminated ASCII records. The remaining raw fallback is 5,115
bytes across eighteen owners. It consists of alignment gaps, the relocation-
backed numeric/pointer area, help control tables, player-dialog control tables,
gameplay message tables, and terminal initialized data.

No complete upstream machine extent lies wholly in these ranges. The next
mechanical frontier is structure recovery for the control/pointer tables and
relocation-aware numeric records; matching-C coverage is now 58,895 bytes.
