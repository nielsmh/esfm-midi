#include <stdio.h>
#include <dos.h>

#include "esfm.h"

int FMBASE = 0x388;

/*
 * Frequency numbers.
 * Index 0 and 13 are for modulation effect base,
 * the range of values for lookup are index 1 to 12, inclusive.
 */
unsigned FNUM_BASE[14] = {
	326, // B
	345, // C
	365, // C#
	387, // D
	410, // D#
	435, // E
	460, // F
	488, // F#
	517, // G
	547, // G#
	580, // A
	614, // A#
	651, // B
	690, // C
};

unsigned FBLOCK_FMULT_OCTAVE[2][9] = {
	{ 1, 1 },
	{ 2, 1 },
	{ 2, 2 },
	{ 2, 4 },
	{ 3, 4 },
	{ 4, 4 },
	{ 5, 4 },
	{ 6, 4 },
	{ 7, 4 },
};

unsigned int fm_regbase(int chan, int op)
{
	return (chan << 5) + (op << 3);
}

void fm_write(unsigned int reg, unsigned char d)
{
	outportb(FMBASE + 2, reg & 0xFF);
	outportb(FMBASE + 3, reg >> 8);
	(void)inportb(FMBASE); /* delay */
	outportb(FMBASE + 1, d);
}

unsigned char fm_read(unsigned int reg)
{
	outportb(FMBASE + 2, reg & 0xFF);
	outportb(FMBASE + 3, reg >> 8);
	(void)inportb(FMBASE); /* delay */
	return inportb(FMBASE + 1);
}

void fm_noteon(int chan)
{
	fm_write(0x240 + chan, 0x01);
}

void fm_noteoff(int chan)
{
	fm_write(0x240 + chan, 0x00);
}

void fm_init(void)
{
	outportb(FMBASE, 0x01);
	fm_write(0x105, 0x80);
}

void fm_setop(int chan, int op, ESFM_Operator *params)
{
	unsigned int regbase = fm_regbase(chan, op);
	fm_write(regbase + 0, (params->trm << 7) | (params->vib << 6) | (params->egt << 5) | (params->ksr << 4) | params->fmult);
	fm_write(regbase + 1, (params->ksl << 3) | params->attenuation);
	fm_write(regbase + 2, (params->attack << 4) | params->decay);
	fm_write(regbase + 3, (params->sustain << 4) | params->release);
	fm_write(regbase + 4, params->fnum & 0xFF);
	fm_write(regbase + 5, (params->delay << 5) | (params->fblock << 2) | (params->fnum >> 8));
	fm_write(regbase + 6, (params->trmd << 7) | (params->vibd << 6) | (params->out_right << 5) | (params->out_left << 4) | (params->mod_level << 1) | params->_unk1);
	fm_write(regbase + 7, (params->out_level << 5) | (params->noise << 3) | params->wave);
}

int main()
{
	int chan = 0;
	int op = 0;
	ESFM_Operator opdata = {0};

	// Frequency: 200 Hz
	opdata.fmult = 4;
	opdata.fnum = 522;
	opdata.fblock = 3;

	// Envelope
	opdata.egt = 1;
	opdata.attack = 7;
	opdata.decay = 1;
	opdata.sustain = 3;
	opdata.release = 5;
	opdata.delay = 0;
	// Tone
	opdata.wave = 1;
	opdata.noise = 0;
	opdata.ksr = 0;
	opdata.vib = 0;
	opdata.trm = 0;
	opdata.ksl = 0;
	opdata.vibd = 0;
	opdata.trmd = 0;
	// Levels
	opdata.attenuation = 0;
	opdata.mod_level = 0;
	opdata.out_left = 1;
	opdata.out_right = 1;
	opdata.out_level = 7;

	printf("Hello world (%u)\n", sizeof(opdata));
	fm_init();

	for (op = 0; op < 1; ++op) {
		fm_setop(chan, op, &opdata);
	}

	fm_noteon(chan);
	delay(1200);
	fm_noteoff(chan);

	puts("Goodbye world\n");

	return 0;
}
