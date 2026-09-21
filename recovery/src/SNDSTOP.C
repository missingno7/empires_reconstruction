extern int g1770, g1e8a;
extern void speaker_gate_off();
void sound_stop_reset(void)
{
    g1770 = 0;
    g1e8a = -1;
    speaker_gate_off();
}
