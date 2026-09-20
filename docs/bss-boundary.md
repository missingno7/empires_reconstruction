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
load address. The source link generates a real TASM `_BSS` contribution with
the recovered public map; TLINK places it, the runtime contributions, BSSEND
and stack. The verifier uses original bytes only to check the evidence.
The complete game BSS range is now partitioned into ordered source
contributions with no aggregate reserve. Historical module ownership of those
contributions remains unresolved, but the exact link no longer uses a synthetic
DGROUP OMF object.

The complete structural experiment now produces the original 79,154-byte EXE
with SHA-256
`1259348425483d8d97fd8821860b47cfdf58fc8029711eb0ed0e78ab33807a10`.
Historical translation-unit recovery and elimination of the remaining
ordering/module adapters are still required before claiming the historical
build recovered.
