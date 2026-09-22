# Differential parity test: portable/audio/sound_driver.c vs. the real
# asm/SOUND.ASM machine code, executed once ahead of time under Unicorn
# (tools/portable/sound_oracle/) into portable/tests/fixtures/
# sound_scenario_*.txt. No DOS toolchain or assets/ needed at test time --
# see test_sound_parity.c's header comment.
empires_add_test(test_sound_parity test_sound_parity.c)
