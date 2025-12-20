#include <stdio.h>
#include <dos.h>
#include <bios.h>
#include <mem.h>

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

unsigned FBLOCK_FMULT_OCTAVE[9][2] = {
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

void fm_playchan(ESFM_Channel *chan, char octave, char note) {
	int op;
	int fblock, fmult, fnum;

	// stop playing
	if (chan->playing) {
		fm_noteoff(chan->channel);
		//delay(2);
		chan->playing = 0;
	}

	// prepare note
	chan->octave = octave;
	chan->note = note;
	for (op = 0; op < 4; ++op) {
		if (chan->frule[op].use_note) {
			fblock = FBLOCK_FMULT_OCTAVE[octave][0] + chan->frule[op].dfblock;
			fmult = FBLOCK_FMULT_OCTAVE[octave][1] + chan->frule[op].dfmult;
			fnum = FNUM_BASE[note] + chan->frule[op].dfnum;
		} else {
			fblock = chan->frule[op].dfblock;
			fmult = chan->frule[op].dfmult;
			fnum = chan->frule[op].dfnum;
		}
		if (fblock < 0) fblock = 0;
		if (fblock > 7) fblock = 7;
		if (fmult < 0) fmult = 0;
		if (fmult > 15) fmult = 15;
		if (fnum < 0) fnum = 0;
		if (fnum > 1023) fnum = 1023;
		chan->op[op].fblock = fblock;
		chan->op[op].fmult = fmult;
		chan->op[op].fnum = fnum;
		fm_setop(chan->channel, op, &chan->op[op]);
	}

	// play!
	fm_noteon(chan->channel);
	chan->playing = 1;
}

int main()
{
	int note, lastnote = -1;
	int octave = 3;
	int run = 1;
	ESFM_Channel channel;

	memset(&channel, 0, sizeof(channel));

	channel.frule[0].use_note = 1;
	channel.frule[1].use_note = 1;
	channel.frule[2].use_note = 1;
	channel.frule[3].use_note = 1;

	// Envelope
	channel.op[0].egt = 1;
	channel.op[0].attack = 7;
	channel.op[0].decay = 3;
	channel.op[0].sustain = 4;
	channel.op[0].release = 5;
	channel.op[0].delay = 0;
	// Tone
	channel.op[0].wave = 1;
	channel.op[0].noise = 0;
	channel.op[0].ksr = 0;
	channel.op[0].vib = 0;
	channel.op[0].trm = 0;
	channel.op[0].ksl = 0;
	channel.op[0].vibd = 0;
	channel.op[0].trmd = 0;
	// Levels
	channel.op[0].attenuation = 0;
	channel.op[0].mod_level = 0;
	channel.op[0].out_left = 1;
	channel.op[0].out_right = 1;
	channel.op[0].out_level = 7;

	printf("Hello world (%u)\n", sizeof(channel));
	fm_init();

	while (run) {
		while (bioskey(1) == 0);
		note = bioskey(0);
		//printf("<%04x>", note);
		switch (note >> 8) {
			// Q2W3E
			case 0x10: note =  1; octave = 2; break;
			case 0x03: note =  2; octave = 2; break;
			case 0x11: note =  3; octave = 2; break;
			case 0x04: note =  4; octave = 2; break;
			case 0x12: note =  5; octave = 2; break;
			// R5T6Y7U
			case 0x13: note =  6; octave = 2; break;
			case 0x06: note =  7; octave = 2; break;
			case 0x14: note =  8; octave = 2; break;
			case 0x07: note =  9; octave = 2; break;
			case 0x15: note = 10; octave = 2; break;
			case 0x08: note = 11; octave = 2; break;
			case 0x16: note = 12; octave = 2; break;
			// I9O0P
			case 0x17: note =  1; octave = 3; break;
			case 0x0A: note =  2; octave = 3; break;
			case 0x18: note =  3; octave = 3; break;
			case 0x0B: note =  4; octave = 3; break;
			case 0x19: note =  5; octave = 3; break;
			// ZSXDCFV
			case 0x2C: note =  6; octave = 3; break;
			case 0x1F: note =  7; octave = 3; break;
			case 0x2D: note =  8; octave = 3; break;
			case 0x20: note =  9; octave = 3; break;
			case 0x2E: note = 10; octave = 3; break;
			case 0x21: note = 11; octave = 3; break;
			case 0x2F: note = 12; octave = 3; break;
			// BHNJM
			case 0x30: note =  1; octave = 4; break;
			case 0x23: note =  2; octave = 4; break;
			case 0x31: note =  3; octave = 4; break;
			case 0x24: note =  4; octave = 4; break;
			case 0x32: note =  5; octave = 4; break;
			// space
			case 0x39: note = -1; break;
			// escape
			case 0x01: run = 0; note = -1; break;
			// F1-F8
			case 0x3B: case 0x3C: case 0x3D: case 0x3E:
			case 0x3F: case 0x40: case 0x41: case 0x42:
				channel.op[0].wave = (unsigned)((note >> 8) - 0x3B) & 7;
				printf("wvf=%d,", channel.op[0].wave);
				note = -2;
				break;
			default:  note = -255; break;
		}

		if (note > 0) {
			printf("%01d-%01x,", octave, note);
			fm_playchan(&channel, octave, note);
		}
		if (note == -1) {
			// sound off
			printf("*,");
			fm_noteoff(channel.channel);
		}
		if (note == -2 && lastnote > 0) {
			// sound off after changing parameter
			fm_noteoff(channel.channel);
		}
		if (note == -255) {
			// ignore?
		}
	}

	puts("*\nGoodbye world\n");

	return 0;
}
