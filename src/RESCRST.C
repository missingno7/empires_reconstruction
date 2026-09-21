/* F_D5F9 -- forward and then clear. */
extern void resource_record_cache_load(), f_c7cb();

void resource_record_cache_reset(int n)
{
    resource_record_cache_load(n);
    f_c7cb(0);
}
