extern int snd_on, g1e8a;
extern void speaker_gate_off();
void sound_stop_reset(void)
{
    snd_on = 0;
    g1e8a = -1;
    speaker_gate_off();
}
