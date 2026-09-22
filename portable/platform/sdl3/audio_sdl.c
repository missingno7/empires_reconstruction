/* audio_sdl.c -- SDL3 audio device glue (see audio_sdl.h).
 *
 * Opens one logical playback device + SDL_AudioStream via
 * SDL_OpenAudioDeviceStream(), mono 16-bit PCM at AUDIO_SDL_SAMPLE_RATE.
 * SDL converts/resamples between our fixed-rate stream and whatever
 * format the physical device actually negotiates (task: "48000 Hz (or
 * whatever the device negotiates via SDL)") -- audio_render() (audio.h)
 * always renders at AUDIO_SDL_SAMPLE_RATE; the device-side conversion is
 * entirely SDL's problem, not ours.
 *
 * Uses the pull-callback form (SDL_OpenAudioDeviceStream's callback
 * parameter, equivalent to SDL_SetAudioStreamGetCallback on a stream
 * created separately): SDL invokes audio_feed_callback() on its own audio
 * thread whenever the device's internal buffer needs more data, and we
 * fill exactly that much by calling audio_render() and feeding it back
 * with SDL_PutAudioStreamData().  No separate feeder thread/sync.h queue
 * is needed with this callback form.
 */
#include "audio_sdl.h"

#include "audio.h"

#include <SDL3/SDL.h>

#include <stdio.h>

#define AUDIO_SDL_SAMPLE_RATE 48000
#define AUDIO_SDL_FEED_CHUNK_FRAMES 1024 /* stack buffer size for the feed callback */

static SDL_AudioStream *s_stream;
static bool s_audio_subsystem_inited;

static void audio_feed_callback(void *userdata, SDL_AudioStream *stream, int additional_amount, int total_amount)
{
    (void)userdata;
    (void)total_amount;

    if (additional_amount <= 0)
        return;

    int16_t buf[AUDIO_SDL_FEED_CHUNK_FRAMES];
    int frames_needed = additional_amount / (int)sizeof(int16_t);

    while (frames_needed > 0) {
        int chunk = frames_needed > AUDIO_SDL_FEED_CHUNK_FRAMES ? AUDIO_SDL_FEED_CHUNK_FRAMES : frames_needed;
        audio_render(buf, chunk);
        SDL_PutAudioStreamData(stream, buf, chunk * (int)sizeof(int16_t));
        frames_needed -= chunk;
    }
}

bool audio_sdl_init(void)
{
    audio_mixer_init(AUDIO_SDL_SAMPLE_RATE);

    /* sdl_video_init() (video_sdl.c) only requests SDL_INIT_VIDEO -- unlike
     * some other SDL3 subsystems, SDL_OpenAudioDeviceStream() does NOT
     * implicitly initialise SDL_INIT_AUDIO, it fails outright if it was
     * never requested, so this module owns its own explicit
     * SDL_InitSubSystem() (ref-counted, safe alongside video_sdl.c's own
     * SDL_Init()) rather than reaching into video_sdl.c. */
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        fprintf(stderr, "audio: SDL_InitSubSystem(SDL_INIT_AUDIO) failed: %s (continuing without audio)\n", SDL_GetError());
        return false;
    }
    s_audio_subsystem_inited = true;

    SDL_AudioSpec spec;
    spec.format = SDL_AUDIO_S16;
    spec.channels = 1;
    spec.freq = AUDIO_SDL_SAMPLE_RATE;

    s_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, audio_feed_callback, NULL);
    if (!s_stream) {
        fprintf(stderr, "audio: SDL_OpenAudioDeviceStream failed: %s (continuing without audio)\n", SDL_GetError());
        return false;
    }

    /* Log the negotiated device format once, per task requirement 7. */
    SDL_AudioDeviceID devid = SDL_GetAudioStreamDevice(s_stream);
    SDL_AudioSpec dev_spec;
    int dev_frames = 0;
    if (devid != 0 && SDL_GetAudioDeviceFormat(devid, &dev_spec, &dev_frames)) {
        fprintf(stderr, "audio: device format %d Hz, %d channel(s), SDL_AudioFormat 0x%04x, %d-frame device buffer\n",
                dev_spec.freq, dev_spec.channels, (unsigned)dev_spec.format, dev_frames);
    } else {
        fprintf(stderr, "audio: opened stream, device format unavailable: %s\n", SDL_GetError());
    }

    if (!SDL_ResumeAudioStreamDevice(s_stream)) {
        fprintf(stderr, "audio: SDL_ResumeAudioStreamDevice failed: %s (continuing without audio)\n", SDL_GetError());
        SDL_DestroyAudioStream(s_stream);
        s_stream = NULL;
        return false;
    }

    return true;
}

void audio_sdl_shutdown(void)
{
    if (s_stream) {
        /* Destroying a stream opened by SDL_OpenAudioDeviceStream also
         * closes the logical device it owns. */
        SDL_DestroyAudioStream(s_stream);
        s_stream = NULL;
    }
    if (s_audio_subsystem_inited) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        s_audio_subsystem_inited = false;
    }
    audio_mixer_shutdown();
}
