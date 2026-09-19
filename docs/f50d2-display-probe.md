# F_50D2 display-adapter probe

`F_50D2` is a 237-byte game-owned routine called before the display-mode
selection code. Its current exact source is an `ASM_DB_CAPSULE`; this note
records the complete control-flow facts needed for its next representation as
symbolic TASM.

The routine clears `bbfcd`, obtains the current BIOS video mode with `INT 10h`
function `0Fh`, then follows the adapter-specific paths below. Each assignment
to `bbfcd` is an exact DS-relative OMF fixup, and `b856` is set only by the ROM
signature path.

| Evidence | Resulting state/action |
|---|---|
| Current mode is `07h`; `F000:FFFE` is `FFh` | Continue without assigning an adapter class. |
| Current mode is `07h`; `C000:0000` is `21h` | Set `bbfcd = 3`, set `b856 = 1`. |
| Current mode is `1Ah` and BIOS return `BL < 0Ah` | Set `bbfcd = 4`. |
| `BL` is `04h` or `05h` | Set `bbfcd = 1`. |
| `BL` is `07h` or `08h` | Set `bbfcd = 5`. |
| `BL` is `02h` | Set `bbfcd = 2`. |
| BIOS function `12h`, subfunction `10h`, rejects the probe | Set `bbfcd = 1`. |
| Port `03D4h` echo test fails | Set `bbfcd = 2`. |

The final branch sequence distinguishes the remaining `bbfcd` values around
the `INT 10h` `12h/10h` result. It must be retained exactly in the symbolic
rewrite; the existing [complete matching proof](matching-c-wave127.md) remains
the byte/fixup oracle. None of these conservative names claim original source
terminology.
