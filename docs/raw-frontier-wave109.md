# Raw executable frontier after wave 108

Wave 108 removes 250 help/menu text bytes. The remaining raw fallback is
10,173 bytes across fourteen owners. It consists of alignment gaps, the
relocation-backed numeric/pointer area, help control tables, player-dialog
control tables, gameplay message tables, and terminal initialized data.

No complete upstream machine extent lies wholly in these ranges. The next
mechanical frontier is structure recovery for the control/pointer tables and
relocation-aware numeric records; matching-C coverage is now 57,257 bytes.
