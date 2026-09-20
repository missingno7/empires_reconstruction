# Canonical source DATA link plan

The normal executable build no longer performs a post-hoc DATA/code interleaving stage. `tools/probe_source_data_link.py` reads [`canonical-link-plan.json`](../recipes/data/canonical-link-plan.json) while it constructs source-derived DATA objects, then writes the final object order directly into `LINK.RSP` before Turbo Link 2.0 runs.

| Compatible reconstructed DATA module | After code | Before code |
|---|---|---|
| `FILE_ERROR_DATA` | `F_6181` | `F_699E` |
| `HELP_MENU_DATA` | `F_7BFC` | `F_9EC3` |
| `PLAYER_DIALOG_DATA` | `F_9EC3` | `F_A09D` |
| `CONTROL_MENU_DATA` | `F_ADCF` | `F_DDD9` |

Each range retains canonical source-component order. The response-file positions are ordinary TLINK inputs: they provide no load address, padding, OMF rewrite, or final-EXE edit. With the three shared source-module replacements, this direct plan preserves the load image and makes all 106 relocation entries appear in historical order.

These are `COMPATIBLE_RECONSTRUCTED_MODULES`, not claims about original C-file names or exact translation-unit boundaries. The older [`interleaving-candidate.json`](../recipes/data/interleaving-candidate.json) and `probe_data_interleaving.py` remain diagnostic evidence outside the canonical build path.
