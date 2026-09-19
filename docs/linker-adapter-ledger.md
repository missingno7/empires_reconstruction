# Linker adapter ledger

The structural path now has an exact code-prefix placement under the locally
verified Turbo Link 2.0 candidate. The remaining adapters are tracked here so
each can be removed against a measurable linker invariant.

| Adapter | Why it exists now | Satisfies | Replacement evidence | Current status |
|---|---|---|---|---|
| `LIBDEMAND.OBJ` historical-library demand | The fixed manifest records library ownership, while historical callers/data units are not all reconstructed yet | Requests the publics that select the 42 observed `CC.LIB` modules | Real EXTDEFs from reconstructed startup, data, and game modules must select the same module sequence | **Removed for the current code experiment**: demand and no-demand Turbo Link 2.0 maps select the same 391 module rows and the same 42 library modules; data remains unresolved |
| `DGSCF.OBJ` synthetic DGROUP | Most initialized data and BSS ownership is not yet represented by relocatable objects | Supplies temporary `_DATA`, `_BSS`, and stack sizing plus selected data publics | Replace its 13,931 initialized bytes and 37,253 BSS bytes with canonical data/BSS source objects | **Active**; Turbo Link 2.0 currently rejects the generated object as `bad object file`, after duplicate startup publics were filtered |
| Oracle-copied initialized DATA tail | The synthetic DGROUP needs bytes to reach the observed initialized-image span | Holds the unresolved initialized-data extent | Decode the two largest raw data owners and compile/encode their records into `_DATA` contributions | **Active and prohibited in the final path** |
| Recovered symbol aliases (`_delay` → `_f6c57`, `_main`) | Recovered objects use numeric/source-local names while startup and callers use historical publics | Resolves verified call targets without changing code bytes | Recover the containing translation unit and its actual public names | **Temporary, tracked per binding** |
| EXTDEF case normalization | Turbo C emits case variants that differ from explicit recovered publics | Resolves case-sensitive OMF externals under the linker candidate | Reconstruct the historical declaration/public spelling in the source module | **Temporary** |
| Injected internal numeric publics (`_Fxxxx`) | Separate recovered objects refer to internal addresses before their owning module is known | Exposes exact code-offset targets to the relocatable experiment | Group contiguous owners into shared historical modules or expose proven real publics | **Temporary** |

The no-demand result is deliberately narrower than a recovered build: it proves
that the demand object is not needed for the current `_TEXT` ordering, while the
424 unresolved DGROUP/data references show why the synthetic data object remains
the next structural target.
