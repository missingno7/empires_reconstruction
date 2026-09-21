# Relocation Topology Audit

## Rule

The EXE's 106 relocations are written by TLINK in the order of each object's FIXUPP subrecords, object by object in link order. Turbo C's native object writer emits FIXUPP subrecords in DESCENDING source-offset order; the generated DATA object (tools/data_omf.py) emits descending on purpose; TASM-produced objects -- hand-written assembler modules, Turbo C units compiled with '-B', and C units containing an inline `asm` statement (all of which restart the source through the assembler) -- emit ASCENDING. MUSIC (M_DDD9_DF98) is historically descending, so its original was a pure Turbo C unit; the TASM module reproduces it with descending ORG contributions.

Total EXE relocations: 106

## Per-module table

| Module | Tool | Flags | Relocs | Observed | Predicted | Mismatch |
|---|---|---|---|---|---|---|
| C_01BC_0355 | TCC.EXE | -c -mc -1- -f- -N- | 1 | single | ascending |  |
| RUNTIME_BLOCK | TASM.EXE | /mx | 0 | none | ascending |  |
| F_1D47 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_1EA5 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_1EB4 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_1EC0 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_1ECD | TASM.EXE | /mx | 0 | none | ascending |  |
| F_1F17 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | ascending |  |
| F_1F91 | TASM.EXE | /mx | 0 | none | ascending |  |
| C_200F_3986 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | ascending |  |
| C_3A75_4A93 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | ascending |  |
| M_4AA8_4EEB | TASM.EXE | /mx | 0 | none | ascending |  |
| C_4F63_520A | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | ascending |  |
| C_5321_56C6 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| C_5A3B_6021 | TCC.EXE | -c -mc -1- -f- -N- | 1 | single | descending |  |
| M_6036_6181 | TASM.EXE | /mx | 3 | ascending | ascending |  |
| C_622C_625D | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| C_6266_68AA | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_68CF | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| C_695E_697D | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| C_6990_6B74 | TCC.EXE | -c -mc -1- -f- -N- | 1 | single | ascending |  |
| C_6B7A_6C87 | TCC.EXE | -c -mc -1- -f- -N- | 1 | single | ascending |  |
| C_6CA6_6D3C | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | ascending |  |
| M_6D86_6F4B | TASM.EXE | /mx | 0 | none | ascending |  |
| C_6FC3_747B | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| C_75F3_7856 | TCC.EXE | -c -mc -1- -f- -N- | 3 | descending | descending |  |
| F_7856 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_78C3 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_791E | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_7925 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_792C | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_7932 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_7964 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_7BFC | TCC.EXE | -c -mc -1- -f- -N- | 1 | single | descending |  |
| C_7D91_880A | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| C_8A37_969D | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| C_984C_9871 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| M_988F_98CB | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_9908 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| C_9962_99E2 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| C_9A0E_9D79 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_9D8E | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_9DCC | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_9EC3 | TASM.EXE | /mx | 2 | ascending | ascending |  |
| F_9F40 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_A004 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_A036 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| C_A09D_A24E | TCC.EXE | -c -mc -1- -f- -N- | 1 | single | descending |  |
| F_A28D | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| C_A33F_AD0E | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| RELOC_F_AD25_F_ADCF | TCC.EXE | -c -mc -1- -f- -N- | 6 | descending | descending |  |
| C_AF45_C15E | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| M_C1A0_CB48 | TASM.EXE | /mx | 0 | none | ascending |  |
| M_CB5C_CD23 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| C_CDDD_D344 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| M_D386_D3CF | TASM.EXE | /mx | 0 | none | ascending |  |
| F_D3DA | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_D45C | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_D471 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_D487 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_D49D | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_D4B3 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_D555 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_D593 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_D5A6 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_D5B3 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_D5BA | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_D5F9 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_D60C | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| M_D61C_D79C | TASM.EXE | /mx | 0 | none | ascending |  |
| M_D818_D825 | TASM.EXE | /mx | 0 | none | ascending |  |
| C_D85F_D8F0 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | ascending |  |
| C_D99B_DA49 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| F_DA66 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| M_DAD7_DB35 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| C_DB60_DDC7 | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| M_DDD9_DF98 | TCC.EXE | -c -mc -1- -f- -N- | 10 | descending | descending |  |
| C_E095_E54D | TCC.EXE | -c -mc -1- -f- -N- | 0 | none | descending |  |
| LIB_RAND | TCC.EXE | -c -mc -1- -f- -N- | 1 | single | descending |  |

## Generated and library objects

| Object | Category | Predicted | Note |
|---|---|---|---|
| D0000.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0001.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0002.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0003.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0004.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0005.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0006.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0007.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0008.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0009.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0010.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0011.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0012.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0013.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0014.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0015.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0016.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0017.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0018.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0019.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0020.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0021.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0022.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0023.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0024.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0025.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0026.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0027.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0028.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0029.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0030.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0031.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0032.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0033.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0034.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0035.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0036.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0037.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0038.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0039.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0040.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0041.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0042.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0043.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0044.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0045.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0046.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0047.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0048.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0049.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0050.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0051.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0052.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0053.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0054.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0055.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0056.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0057.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0058.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0059.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0060.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0061.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0062.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0063.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0064.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0065.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0066.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0067.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0068.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0069.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0070.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0071.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0072.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0073.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0074.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0075.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0076.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0077.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0078.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0079.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0080.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0081.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0082.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0083.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0084.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0085.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0086.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0087.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0088.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0089.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0090.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0091.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0092.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0093.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0094.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0095.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0096.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0097.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0098.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0099.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0100.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0101.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0102.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0103.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0104.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0105.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0106.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0107.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0108.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0109.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0110.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0111.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0112.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0113.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0114.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0115.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| D0116.OBJ | generated-data | descending | No EXE-relative module span recorded for DATA objects in production-plan.json; direction is asserted by the generator's own design comment, not independently re-observed here. |
| F01CEBSS.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| ROWPTRS.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| RSTATEB.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| G40D4B.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| B4374B.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| G43B4B.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| F200FB.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| G6F2AB.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| A72B2B.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| GAMEBSS7.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| F200FC.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| A74A2B.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| GAMEBSS8.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| G893CB.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| GAMEBSSA.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| S8C12B.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| G96BSS.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| G9990B.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| G99BSS.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| G9CF2B.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| GA6B6B.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| GAMEBSSF.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| GB07CB.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| GB1CCB.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| GB3AEB.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| GBF66B.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| VIDSTATE.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| SLOTBSS.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| GAMEBSS1.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| ANIMBSS.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| GC360BSS.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| GAMEBSS3.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| C470BSS.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| GAMEBSS.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| FLAGSBS.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| GAMEBSS4.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| OCTAVES.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| NOTEIDX.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| SNDSTATE.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| VOICEPTR.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| GAMEBSS6.OBJ | generated-bss | None | BSS objects declare storage only (no LEDATA); typically carry no fixups |
| C0C.OBJ | library (CC.LIB) | None | pulled from the pinned CC.LIB by TLINK, not a production-plan module |

## Mismatches

None.
## Fresh compile verification

Checked against `build\production-i_l5nnx6\compile\WORK` (20 objects, 13 agree with prediction, 0 disagree).

| Object | Module | Raw fixups | Raw direction | Predicted | Agrees |
|---|---|---|---|---|---|
| R0000.OBJ | C_01BC_0355 | 40 | ascending | ascending | yes |
| R0001.OBJ | RUNTIME_BLOCK | 0 | none | ascending | yes |
| R0006.OBJ | F_1ECD | 6 | ascending | ascending | yes |
| R0007.OBJ | F_1F17 | 1 | single | ascending | yes |
| R0008.OBJ | F_1F91 | 1 | single | ascending | yes |
| R0009.OBJ | C_200F_3986 | 407 | ascending | ascending | yes |
| R0010.OBJ | C_3A75_4A93 | 496 | ascending | ascending | yes |
| R0011.OBJ | M_4AA8_4EEB | 20 | ascending | ascending | yes |
| R0012.OBJ | C_4F63_520A | 77 | ascending | ascending | yes |
| R0015.OBJ | M_6036_6181 | 24 | mixed | ascending | NO |
| R0020.OBJ | C_6990_6B74 | 43 | mixed | ascending | NO |
| R0021.OBJ | C_6B7A_6C87 | 32 | mixed | ascending | NO |
| R0022.OBJ | C_6CA6_6D3C | 14 | ascending | ascending | yes |
| R0023.OBJ | M_6D86_6F4B | 19 | ascending | ascending | yes |
| R0043.OBJ | F_9EC3 | 4 | ascending | ascending | yes |
| R0052.OBJ | M_C1A0_CB48 | 191 | ascending | ascending | yes |
| R0055.OBJ | M_D386_D3CF | 3 | ascending | ascending | yes |
| R0069.OBJ | M_D61C_D79C | 6 | ascending | ascending | yes |
| R0070.OBJ | M_D818_D825 | 0 | none | ascending | yes |
| R0071.OBJ | C_D85F_D8F0 | 15 | ascending | ascending | yes |

## TASM modules with descending historical relocations

Every TASM-tooled module whose historical EXE relocations are descending -- the native-Turbo-C fingerprint -- regardless of whether it is already listed above as a mismatch, so re-C candidates stay visible:

None.

## What mismatches imply for re-C work

A module flagged as a mismatch above is direct object-topology evidence about its ORIGINAL (pre-reconstruction) authorship, independent of any register-ABI or instruction-shape argument: descending relocations under a TASM/assembler-restart production route point to an original native Turbo C unit (a re-C candidate worth pursuing even if instruction-shape evidence looks ambiguous); ascending relocations under a plain-TCC route point to an original hand- or TASM-assembled unit (re-C should not be attempted, or should be treated with low confidence if it appears to succeed).
