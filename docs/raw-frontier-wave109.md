# Raw executable frontier after wave 108

Wave 138 removes 187 bytes of fixed records and terminal padding after wave
137's 28 bytes of paired pointer attributes and wave 136's 56
bytes of pointer-table data and wave 135's 3,155-byte
all-zero region and wave 134's 99 bytes of zero
runs and terminated ASCII records. The remaining raw fallback is 5,126 bytes
across sixteen owners. It consists of alignment gaps, the
relocation-backed numeric/pointer area, help control tables, player-dialog
control tables, gameplay message tables, and terminal initialized data.

No complete upstream machine extent lies wholly in these ranges. The next
mechanical frontier is structure recovery for the control/pointer tables and
relocation-aware numeric records; matching-C coverage is now 58,895 bytes.
