/* game.h -- umbrella header for portable/game/*.c (tu-porting-rules.md
 * sec 1: "The first non-comment line is #include "game.h".  Delete every
 * local `extern` object/function declaration and every local #include of
 * a historical header ...: game.h provides the generated state
 * (game_state.h, game_data.h), the ported structs (game_structs.h), every
 * function prototype (game_funcs.h) and the service headers (gfx.h,
 * resource.h, timer.h, input.h, sound.h, cclib.h)."
 *
 * Include order is deliberate, not arbitrary:
 *   1. dos_types.h first -- every other header depends on the dos_* aliases.
 *   2. game_structs.h before anything that references its struct tags.
 *   3. game_state.h (BSS) then game_data.h (initialized DATA, which itself
 *      targets BSS state -- game_data.h already includes game_state.h, but
 *      spelling it out here keeps this header's own ordering obvious).
 *   4. The hand-written service headers (gfx/resource/timer/input/sound).
 *   5. cclib.h before <stdlib.h>/<string.h> at the bottom -- NOT a hazard
 *      in practice (cclib.h includes those two itself, before defining its
 *      `#define rand cc_rand`-style macros -- see cclib.h's own header
 *      comment), but the ordering below keeps game.h's own text matching
 *      the dependency direction: services and cclib/dosio before the
 *      prototype list that calls into all of them, system headers last.
 * dosio.h has no dependency on cclib.h or vice versa; either order is fine.
 */
#ifndef PORTABLE_GAME_H
#define PORTABLE_GAME_H

#include "dos_types.h"
#include "game_structs.h"
#include "game_state.h"
#include "game_data.h"

#include "gfx.h"
#include "resource.h"
#include "timer.h"
#include "input.h"
#include "sound.h"

#include "cclib.h"
#include "dosio.h"

#include "game_funcs.h"

#include <setjmp.h>
#include <stdlib.h>
#include <string.h>

#endif /* PORTABLE_GAME_H */
