# Raw executable frontier after wave 108

Wave 133 removes 126 bytes of zero padding, relocation-free word tables and one
terminated ASCII record after wave 132's 1,274 bytes of text. The remaining raw
fallback is 8,651 bytes across twenty-one owners. It consists of alignment gaps, the
relocation-backed numeric/pointer area, help control tables, player-dialog
control tables, gameplay message tables, and terminal initialized data.

No complete upstream machine extent lies wholly in these ranges. The next
mechanical frontier is structure recovery for the control/pointer tables and
relocation-aware numeric records; matching-C coverage is now 58,895 bytes.
