/* Focused tests for the portable runtime/configuration bridge. */
#include "audio.h"
#include "config.h"
#include "port_settings.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef EMPIRES_FIXTURE_DIR
#define EMPIRES_FIXTURE_DIR "."
#endif

static int failures;

static void check(int condition, const char *what)
{
    if (!condition) {
        fprintf(stderr, "test_port_settings: FAIL: %s\n", what);
        failures++;
    }
}

static void test_settings(void)
{
    char path[512];
    char before[1024], after[1024];
    FILE *f;
    size_t before_len, after_len;
    bool missing;
    char err[128];

    snprintf(path, sizeof path, "%s/port_settings_test.json", EMPIRES_FIXTURE_DIR);
    f = fopen(path, "wb");
    check(f != NULL, "create temporary config");
    if (!f) return;
    fputs("{\n  \"audio\": {\"music_volume\": 37, \"sound_volume\": 61},\n"
          "  \"video\": {\"interpolation\": true}\n}\n", f);
    fclose(f);

    f = fopen(path, "rb");
    before_len = fread(before, 1, sizeof before - 1, f);
    before[before_len] = 0;
    fclose(f);

    config_reset();
    check(config_load(path, &missing, err, sizeof err), "load temporary config");
    audio_mixer_init(48000);
    check(port_settings_init(150, 125, false, path), "initialize settings");
    f = fopen(path, "rb");
    after_len = fread(after, 1, sizeof after - 1, f);
    after[after_len] = 0;
    fclose(f);
    check(before_len == after_len && memcmp(before, after, before_len) == 0,
          "initialization does not rewrite config");
    check(port_settings_music_volume() == 150 && audio_mixer_music_volume() == 150,
          "effective music volume is applied");
    check(port_settings_sound_volume() == 125 && audio_mixer_effects_volume() == 125,
          "effective effects volume is applied");
    check(!port_settings_interpolation(), "effective interpolation is applied");

    port_settings_preview_music_volume(-5);
    check(port_settings_music_volume() == 0 && audio_mixer_music_volume() == 0,
          "music preview clamps at zero");
    port_settings_preview_sound_volume(999);
    check(port_settings_sound_volume() == 200 && audio_mixer_effects_volume() == 200,
          "effects preview clamps at 200");

    check(port_settings_commit_music_volume(73), "music commit saves");
    check(port_settings_commit_sound_volume(187), "effects commit saves");
    check(port_settings_commit_interpolation(true), "interpolation commit saves");
    check(port_settings_music_volume() == 73 && port_settings_sound_volume() == 187 &&
          port_settings_interpolation(), "committed values remain live");
    check(config_get_int("audio.music_volume", -1) == 73 &&
          config_get_int("audio.sound_volume", -1) == 187 &&
          config_get_bool("video.interpolation", false),
          "committed values update the config store");

    config_reset();
    check(config_load(path, &missing, err, sizeof err), "reload saved config");
    check(config_get_int("audio.music_volume", -1) == 73 &&
          config_get_int("audio.sound_volume", -1) == 187 &&
          config_get_bool("video.interpolation", false),
          "saved values survive reload");
    remove(path);
}

int main(void)
{
    test_settings();
    if (failures) {
        fprintf(stderr, "test_port_settings: %d failure(s)\n", failures);
        return 1;
    }
    puts("test_port_settings: OK");
    return 0;
}
