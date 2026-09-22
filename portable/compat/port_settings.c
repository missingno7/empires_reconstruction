/* port_settings.c -- thread-safe portable runtime/config bridge. */
#include "port_settings.h"

#include "audio.h"
#include "config.h"
#include "sync.h"

#include <stdio.h>
#include <string.h>

#define PORT_SETTINGS_PATH_MAX 1024

static sync_mutex s_lock;
static bool s_lock_init;
static int s_music_volume = 100;
static int s_sound_volume = 100;
static bool s_interpolation;
static char s_config_path[PORT_SETTINGS_PATH_MAX];

static void ensure_lock(void)
{
    if (!s_lock_init) {
        sync_mutex_init(&s_lock);
        s_lock_init = true;
    }
}

static int clamp_volume(int percent)
{
    if (percent < 0) return 0;
    if (percent > 200) return 200;
    return percent;
}

bool port_settings_init(int music_volume, int sound_volume,
                        bool interpolation, const char *config_path)
{
    if (!config_path || strlen(config_path) >= sizeof s_config_path)
        return false;

    ensure_lock();
    sync_mutex_lock(&s_lock);
    s_music_volume = clamp_volume(music_volume);
    s_sound_volume = clamp_volume(sound_volume);
    s_interpolation = interpolation;
    snprintf(s_config_path, sizeof s_config_path, "%s", config_path);
    sync_mutex_unlock(&s_lock);

    /* This is intentionally runtime-only: CLI-effective values must not be
     * written merely because the service was initialized. */
    audio_mixer_set_music_volume(s_music_volume);
    audio_mixer_set_effects_volume(s_sound_volume);
    return true;
}

int port_settings_music_volume(void)
{
    int value;
    ensure_lock();
    sync_mutex_lock(&s_lock); value = s_music_volume; sync_mutex_unlock(&s_lock);
    return value;
}

int port_settings_sound_volume(void)
{
    int value;
    ensure_lock();
    sync_mutex_lock(&s_lock); value = s_sound_volume; sync_mutex_unlock(&s_lock);
    return value;
}

bool port_settings_interpolation(void)
{
    bool value;
    ensure_lock();
    sync_mutex_lock(&s_lock); value = s_interpolation; sync_mutex_unlock(&s_lock);
    return value;
}

void port_settings_preview_music_volume(int percent)
{
    percent = clamp_volume(percent);
    ensure_lock();
    sync_mutex_lock(&s_lock); s_music_volume = percent; sync_mutex_unlock(&s_lock);
    audio_mixer_set_music_volume(percent);
}

void port_settings_preview_sound_volume(int percent)
{
    percent = clamp_volume(percent);
    ensure_lock();
    sync_mutex_lock(&s_lock); s_sound_volume = percent; sync_mutex_unlock(&s_lock);
    audio_mixer_set_effects_volume(percent);
}

static bool save_after_set(const char *key, int value)
{
    bool set_ok = config_set_int(key, value);
    bool save_ok = set_ok && config_save(s_config_path);
    if (!set_ok)
        fprintf(stderr, "portable settings: cannot update config key %s\n", key);
    else if (!save_ok)
        fprintf(stderr, "portable settings: cannot save %s\n", s_config_path);
    return set_ok && save_ok;
}

static bool save_after_bool(const char *key, bool value)
{
    bool set_ok = config_set_bool(key, value);
    bool save_ok = set_ok && config_save(s_config_path);
    if (!set_ok)
        fprintf(stderr, "portable settings: cannot update config key %s\n", key);
    else if (!save_ok)
        fprintf(stderr, "portable settings: cannot save %s\n", s_config_path);
    return set_ok && save_ok;
}

bool port_settings_commit_music_volume(int percent)
{
    port_settings_preview_music_volume(percent);
    return save_after_set("audio.music_volume", port_settings_music_volume());
}

bool port_settings_commit_sound_volume(int percent)
{
    port_settings_preview_sound_volume(percent);
    return save_after_set("audio.sound_volume", port_settings_sound_volume());
}

bool port_settings_commit_interpolation(bool enabled)
{
    ensure_lock();
    sync_mutex_lock(&s_lock); s_interpolation = enabled; sync_mutex_unlock(&s_lock);
    return save_after_bool("video.interpolation", enabled);
}
