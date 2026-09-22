/* test_config.c -- unit tests for portable/compat/config.c (empires.json
 * store: parse, typed access, defaults, serialise round-trip, file I/O). */
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int s_failures = 0;

#define CHECK(cond) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
            s_failures++; \
        } \
    } while (0)

static void test_parse_and_get(void)
{
    char err[128];
    config_reset();
    CHECK(config_parse("{\n  \"audio\": { \"volume\": 75, \"gain\": 0.5 },\n"
                       "  \"video\": { \"fullscreen\": true, \"integer_scaling\": true },\n"
                       "  \"debug\": { \"enabled\": true },\n"
                       "  \"paths\": { \"assets\": \"C:\\\\games\\\\ae\", \"saves\": \"\" },\n"
                       "  \"name\": \"caf\\u00e9\", \"neg\": -3, \"deep\": { \"a\": { \"b\": 1 } }\n}\n",
                       err, sizeof err));
    CHECK(err[0] == '\0');
    CHECK(config_get_int("audio.volume", 0) == 75);
    CHECK(config_get_number("audio.gain", 0) == 0.5);
    CHECK(config_get_int("audio.gain", 9) == 0);            /* truncates toward zero */
    CHECK(config_get_bool("video.fullscreen", false) == true);
    CHECK(config_get_bool("video.integer_scaling", false) == true);
    CHECK(config_get_bool("debug.enabled", false) == true);
    CHECK(strcmp(config_get_string("paths.assets", ""), "C:\\games\\ae") == 0);
    CHECK(strcmp(config_get_string("paths.saves", "x"), "") == 0);
    CHECK(strcmp(config_get_string("name", ""), "caf\xc3\xa9") == 0);
    CHECK(config_get_int("neg", 0) == -3);
    CHECK(config_get_int("deep.a.b", 0) == 1);
    /* wrong type -> default; absent -> default */
    CHECK(config_get_int("video.fullscreen", 42) == 42);
    CHECK(config_get_string("audio.volume", "d") != NULL && strcmp(config_get_string("audio.volume", "d"), "d") == 0);
    CHECK(config_get_bool("nope", true) == true);
    CHECK(config_has("audio.volume") && !config_has("audio"));
}

static void test_defaults_and_set(void)
{
    config_reset();
    CHECK(config_parse("{\"audio\": {\"volume\": 30}}", NULL, 0));
    config_default_int("audio.volume", 100);     /* file wins */
    config_default_int("audio.other", 7);        /* absent -> set */
    config_default_bool("video.fullscreen", false);
    config_default_bool("video.integer_scaling", false);
    config_default_bool("debug.enabled", false);
    config_default_string("paths.assets", "");
    CHECK(config_get_int("audio.volume", 0) == 30);
    CHECK(config_get_int("audio.other", 0) == 7);
    CHECK(config_get_bool("debug.enabled", true) == false);
    CHECK(config_get_bool("video.integer_scaling", true) == false);
    CHECK(config_set_bool("audio.volume", true));   /* setter replaces type */
    CHECK(config_get_bool("audio.volume", false) == true);
    CHECK(config_get_int("audio.volume", -1) == -1);
}

static void test_errors_leave_store_untouched(void)
{
    char err[128];
    config_reset();
    CHECK(config_parse("{\"a\": 1}", NULL, 0));
    CHECK(!config_parse("{\"a\": 2, \"b\": [1,2]}", err, sizeof err));
    CHECK(strstr(err, "arrays") != NULL);
    CHECK(config_get_int("a", 0) == 1);          /* rolled back */
    CHECK(!config_has("b"));
    CHECK(!config_parse("{\"a\": null}", err, sizeof err));
    CHECK(!config_parse("{\"a\": 1} x", err, sizeof err));
    CHECK(strstr(err, "trailing") != NULL);
    CHECK(!config_parse("{\"a\" 1}", err, sizeof err));
    CHECK(!config_parse("{\"a.b\": 1}", err, sizeof err));   /* dots reserved */
    CHECK(!config_parse("", err, sizeof err));
    CHECK(config_parse("{}", err, sizeof err));
    CHECK(config_get_int("a", 0) == 1);
}

static void test_serialize_round_trip(void)
{
    char out[1024];
    size_t len;
    config_reset();
    CHECK(config_set_int("video.fullscreen_hint", 2));
    CHECK(config_set_bool("video.fullscreen", false));
    CHECK(config_set_bool("video.integer_scaling", false));
    CHECK(config_set_bool("debug.enabled", true));
    CHECK(config_set_int("audio.volume", 100));
    CHECK(config_set_string("paths.assets", "a \"b\"\\c\n"));
    CHECK(config_set_number("tween.speed", 1.25));
    CHECK(config_set_int("deep.a.b", 1));
    CHECK(config_set_int("deep.a.c", 2));
    CHECK(config_set_int("deep.d", 3));
    len = config_serialize(out, sizeof out);
    CHECK(len == strlen(out));
    CHECK(strcmp(out,
                 "{\n"
                 "  \"audio\": {\n"
                 "    \"volume\": 100\n"
                 "  },\n"
                 "  \"debug\": {\n"
                 "    \"enabled\": true\n"
                 "  },\n"
                 "  \"deep\": {\n"
                 "    \"a\": {\n"
                 "      \"b\": 1,\n"
                 "      \"c\": 2\n"
                 "    },\n"
                 "    \"d\": 3\n"
                 "  },\n"
                 "  \"paths\": {\n"
                 "    \"assets\": \"a \\\"b\\\"\\\\c\\n\"\n"
                 "  },\n"
                 "  \"tween\": {\n"
                 "    \"speed\": 1.25\n"
                 "  },\n"
                 "  \"video\": {\n"
                 "    \"fullscreen\": false,\n"
                 "    \"fullscreen_hint\": 2,\n"
                 "    \"integer_scaling\": false\n"
                 "  }\n"
                 "}\n") == 0);
    if (s_failures)
        fputs(out, stderr);

    /* parse what we wrote and compare */
    config_reset();
    CHECK(config_parse(out, NULL, 0));
    CHECK(config_get_int("audio.volume", 0) == 100);
    CHECK(config_get_bool("video.fullscreen", true) == false);
    CHECK(config_get_bool("video.integer_scaling", true) == false);
    CHECK(config_get_bool("debug.enabled", false) == true);
    CHECK(strcmp(config_get_string("paths.assets", ""), "a \"b\"\\c\n") == 0);
    CHECK(config_get_number("tween.speed", 0) == 1.25);
    CHECK(config_get_int("deep.a.c", 0) == 2 && config_get_int("deep.d", 0) == 3);

    /* size query and truncation */
    CHECK(config_serialize(NULL, 0) == len);
    CHECK(config_serialize(out, 8) == len && strlen(out) == 7);

    config_reset();
    CHECK(config_serialize(out, sizeof out) == 3 && strcmp(out, "{}\n") == 0);
}

static void test_file_io(const char *dir)
{
    char path[1024], err[128];
    bool missing = false;
    snprintf(path, sizeof path, "%s/test_config_%d.json", dir, (int)0x1234);

    remove(path);
    config_reset();
    CHECK(!config_load(path, &missing, err, sizeof err) && missing);

    CHECK(config_set_int("audio.volume", 55));
    CHECK(config_set_string("paths.saves", "saves"));
    CHECK(config_save(path));
    config_reset();
    CHECK(config_load(path, &missing, err, sizeof err) && !missing);
    CHECK(config_get_int("audio.volume", 0) == 55);
    CHECK(strcmp(config_get_string("paths.saves", ""), "saves") == 0);

    /* overwrite works (MSVC rename() does not clobber) */
    CHECK(config_set_int("audio.volume", 56));
    CHECK(config_save(path));
    config_reset();
    CHECK(config_load(path, NULL, NULL, 0));
    CHECK(config_get_int("audio.volume", 0) == 56);

    /* BOM tolerated; malformed file reports an error and keeps the store */
    {
        FILE *f = fopen(path, "wb");
        CHECK(f != NULL);
        if (f) { fputs("\xEF\xBB\xBF{\"x\": 1}", f); fclose(f); }
        CHECK(config_load(path, NULL, err, sizeof err) && config_get_int("x", 0) == 1);
        f = fopen(path, "wb");
        if (f) { fputs("{\"x\": ", f); fclose(f); }
        CHECK(!config_load(path, &missing, err, sizeof err) && !missing && err[0] != '\0');
        CHECK(config_get_int("x", 0) == 1);
    }
    remove(path);
}

int main(int argc, char **argv)
{
    const char *dir = argc > 1 ? argv[1] : ".";
    test_parse_and_get();
    test_defaults_and_set();
    test_errors_leave_store_untouched();
    test_serialize_round_trip();
    test_file_io(dir);
    if (s_failures) {
        fprintf(stderr, "test_config: %d failure(s)\n", s_failures);
        return 1;
    }
    puts("test_config: OK");
    return 0;
}
