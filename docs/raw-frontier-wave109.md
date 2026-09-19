# Raw executable frontier after wave 108

Wave 132 removes 1,274 bytes of terminated ASCII records after wave 131's 122
bytes of zero padding, help text and menu control tables. The remaining raw
fallback is 8,777 bytes across twenty owners. It consists of alignment gaps, the
relocation-backed numeric/pointer area, help control tables, player-dialog
control tables, gameplay message tables, and terminal initialized data.

No complete upstream machine extent lies wholly in these ranges. The next
mechanical frontier is structure recovery for the control/pointer tables and
relocation-aware numeric records; matching-C coverage is now 58,895 bytes.
