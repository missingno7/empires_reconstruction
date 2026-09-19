# Wave 108 executable data proof

Nine additional terminated ASCII help/menu strings add 250 independently
encoded bytes from the help block. The intervening control tables remain raw
because their bytes are not a terminated text format.

The promotion uses recipes/c/matching-wave108.json and ascii-nul-v1. Every
source round-trips against its original extent, and the exact rebuild remains
unchanged.
