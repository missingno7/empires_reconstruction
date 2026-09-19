# Verified game BSS boundary

The source DATA path now reproduces the entire original load image. Two
independent corrections were required; neither writes original addresses into
the executable after linking.

First, the library alias adapter had discarded the offset of an explicitly
named public within its module. It incorrectly redirected farmalloc to malloc,
memmove to movmem, setvect to getvect, and longjmp to setjmp. Respecting actual
OMF public offsets removes those unnecessary aliases and 24 differing code
bytes. TLINK now resolves the original public names itself.

Second, stack paragraph rounding cannot determine an exact BSS extent. The
[boundary verifier](bss-boundary.json) recovers the same table base from EXIT's
external reference and ATEXIT's independent segment reference. HARDERR's BSS
base immediately follows ATEXIT's 64-byte contribution; its four-byte extent
ends at the value recovered independently from C0C's BSSEND fixup. These
constraints give 37,250 game BSS bytes plus 68 pinned runtime BSS bytes.

`src/data/GAME_BSS.json` declares that game reserve as a length, without a final
load address. The source link consumes the length; TLINK places the runtime
contributions, BSSEND and stack. The verifier uses original bytes only to check
the evidence. Internal game BSS allocation and historical module ownership
remain unresolved, so the reserve is still reported as a structural scaffold.

The resulting 79,154-byte EXE has SHA-256
`e50eb1e586515d757707ecb767ef79ab8108e0178de41212b363587341551189`.
Its 78,642-byte load image is identical. All fixed MZ fields except relocation
count match; 34 relocation entries are still missing. Header/table identity,
raw source reconstruction and elimination of the other symbol/module adapters
remain required before claiming the historical build recovered.
