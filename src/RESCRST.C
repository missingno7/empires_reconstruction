/* F_D5F9 -- forward and then clear. */
extern void resource_record_cache_load(), sound_voice_table_reload();

void resource_record_cache_reset(int n)
{
    resource_record_cache_load(n);
    sound_voice_table_reload(0);
}
