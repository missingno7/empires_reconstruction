# Raw executable frontier after wave 108

Wave 135 removes a 3,155-byte all-zero region after wave 134's 99 bytes of zero
runs and terminated ASCII records. The remaining raw fallback is 5,397 bytes
across twenty-one owners. It consists of alignment gaps, the
relocation-backed numeric/pointer area, help control tables, player-dialog
control tables, gameplay message tables, and terminal initialized data.

No complete upstream machine extent lies wholly in these ranges. The next
mechanical frontier is structure recovery for the control/pointer tables and
relocation-aware numeric records; matching-C coverage is now 58,895 bytes.
