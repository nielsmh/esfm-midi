#include <conio.h>
#include <stdlib.h>

#include "esfm.h"

int FMBASE = 0x388;

/*
 * Frequency numbers.
 * Index 0 and 13 are for modulation effect base,
 * the range of values for lookup are index 1 to 12, inclusive.
 */
const unsigned FNUM_BASE[14] = {
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

const unsigned FBLOCK_FMULT_OCTAVE[9][2] = {
	{ 0, 2 },
	{ 0, 4 },
	{ 1, 4 },
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
	outp(FMBASE + 2, reg & 0xFF);
	outp(FMBASE + 3, reg >> 8);
	(void)inp(FMBASE); /* delay */
	outp(FMBASE + 1, d);
}

unsigned char fm_read(unsigned int reg)
{
	outp(FMBASE + 2, reg & 0xFF);
	outp(FMBASE + 3, reg >> 8);
	(void)inp(FMBASE); /* delay */
	return inp(FMBASE + 1);
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
	unsigned char status;
	
	// Reset to OPL3 mode
	outp(FMBASE, 0x01);
	// Switch to ESFM mode
	fm_write(0x105, 0x80);
	// Verify
	status = fm_read(0x505);
	if (!(status & 0x80)) abort();
}

void fm_setop(int chan, int op, ESFM_Operator *params)
{
	unsigned int regbase = fm_regbase(chan, op);
	fm_write(regbase + 0, (params->trm << 7) | (params->vib << 6) | (params->egt << 5) | (params->ksr << 4) | params->fmult);
	fm_write(regbase + 1, ((params->ksl & 2) << 5) | ((params->ksl & 1) << 7) | params->attenuation); // KSL bits are reversed
	fm_write(regbase + 2, (params->attack << 4) | params->decay);
	fm_write(regbase + 3, (params->sustain << 4) | params->release);
	fm_write(regbase + 4, params->fnum & 0xFF);
	fm_write(regbase + 5, (params->delay << 5) | (params->fblock << 2) | (params->fnum >> 8));
	fm_write(regbase + 6, (params->trmd << 7) | (params->vibd << 6) | (params->out_right << 5) | (params->out_left << 4) | (params->mod_level << 1) | params->_unk1);
	fm_write(regbase + 7, (params->out_level << 5) | (params->noise << 3) | params->wave);
}

void fm_playchan(ESFM_Channel *chan, int octave, int note) {
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

