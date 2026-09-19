# Raw executable frontier after wave 108

Wave 134 removes 99 bytes of zero runs and terminated ASCII records after wave
133's 126 bytes of zero padding, word tables and text. The remaining raw
fallback is 8,552 bytes across twenty owners. It consists of alignment gaps, the
relocation-backed numeric/pointer area, help control tables, player-dialog
control tables, gameplay message tables, and terminal initialized data.

No complete upstream machine extent lies wholly in these ranges. The next
mechanical frontier is structure recovery for the control/pointer tables and
relocation-aware numeric records; matching-C coverage is now 58,895 bytes.
