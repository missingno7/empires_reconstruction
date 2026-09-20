"""Known, deliberately narrow exact-encoding helpers for TASM 1.0."""
JMP_NEAR = 'JMP_NEAR macro target\n db 0e9h\n dw target-$-2\nendm'
CALL_NEAR = 'CALL_NEAR macro target\n db 0e8h\n dw target-$-2\nendm'
MACROS = {'jmp_near': JMP_NEAR, 'call_near': CALL_NEAR}


def normalized(text):
    return '\n'.join(' '.join(line.split(';',1)[0].lower().split()) for line in text.splitlines() if line.split(';',1)[0].strip())
