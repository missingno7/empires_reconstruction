# Ready-to-run grinder prompt

Work serially on the verified runtime CHEAP queue in this checkout. Read
`docs/current/grinder-instructions.md` and the selected task card before editing.
Do not launch another worker, perform ASM-to-C conversions, change compiler flags,
or consume the held C-reconstruction inventory.

Preflight:

```
python tools/audit_grinder_queue.py
python tools/prepare_grinder_handover.py --check
python tools/reconstruction_factory.py next
```

If either preflight check fails, stop and report the diagnostic. Do not refresh
away an unaccepted edit. Preserve all pre-existing local changes. The supervisor
C recoveries and handover files may be uncommitted; do not stage them as your work.

For each CHEAP runtime card, edit only the permitted source interval. Run the
card's FAST command, then its full acceptance/promotion command. After two failed
targeted corrections, use the documented --block command, which archives the
attempt and restores the isolated edit. Continue with the next eligible card.
Never change an oracle or weaken verification to obtain a pass.

Use fresh `next` output after every promotion/block; do not retain old line
numbers. At the end run the queue audit, regenerate/check the readiness inventory,
and report accepted bytes/cards, blocked cards, remaining work and exact EXE hash.
Stop when no CHEAP card remains or an acceptance/scope failure cannot be restored
safely. Do not fill remaining time with MEDIUM, supervisor, overlay or C work.

## Supervisor review completed

All remaining 44 production ASM sources and their existing C counterparts were
inventoried in grinder-readiness.json. Most C counterparts contain inline ASM/DB
capsules, so a compiler-flag change cannot recover ordinary C from them.

The three ordinary-C arithmetic candidates were each compiled both directly and
through -B/TASM. All six match instruction bytes. F_DDD9/F_DE7E still emit the
relocations in ascending order, while the historical EXE requires descending
order. F_DEFA matches in isolation but belongs to that shared four-owner module.
These are held, not unattended promotion cards. Re-run evidence with:

```
python tools/probe_c_paths.py
```

F_28AC also remains held: -B did not resolve its coordinate conversions. Four
larger C-like routines have bounded follow-up scopes in the readiness inventory,
but need supervisor preparation before becoming executable cards. There are no
new safe flag-only C cards from this review. The existing 72 CHEAP runtime cards
cover 3,866 unresolved bytes and remain the unattended workload.
