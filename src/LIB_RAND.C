/* Exact compact-model RAND/ SRAND contribution reconstructed from C. */
static long state = 1L;

void srand(seed) unsigned seed;
{
    state = seed;
}

int rand()
{
    state = state * 0x015A4E35L + 1L;
    return (int)(state >> 16) & 0x7fff;
}
