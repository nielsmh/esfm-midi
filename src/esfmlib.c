#include <conio.h>
#include <stdlib.h>
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

const char * fm_regname(int regnum, int *maxval)
{
	// make it easier to handle NULL max value pointers
	int dummy;
	if (!maxval) maxval = &dummy;

	switch (regnum) {
	case 0: *maxval = 7; return "OUT ";
	case 1: *maxval = 1; return "OUTL";
	case 2: *maxval = 1; return "OUTR";
	case 3: *maxval = 7; return "MOD ";
	case 4: *maxval = 63; return "ATTN";
	case 5: *maxval = 1; return "EGT ";
	case 6: *maxval = 15; return "ATK ";
	case 7: *maxval = 15; return "DEC ";
	case 8: *maxval = 15; return "SUS ";
	case 9: *maxval = 15; return "REL ";
	case 10: *maxval = 1; return "KSR ";
	case 11: *maxval = 7; return "DELY";
	case 12: *maxval = 3; return "KSL ";
	case 13: *maxval = 2; return "VIB ";
	case 14: *maxval = 2; return "TRM ";
	case 15: *maxval = 3; return "NOIS";
	case 16: *maxval = 7; return "WAVE";
	default:
		return NULL;
	}
}

int fm_getreg(ESFM_Operator *op, int regnum)
{
	switch (regnum) {
	case 0: return op->out_level;
	case 1: return op->out_left;
	case 2: return op->out_right;
	case 3: return op->mod_level;
	case 4: return op->attenuation;
	case 5: return op->egt;
	case 6: return op->attack;
	case 7: return op->decay;
	case 8: return op->sustain;
	case 9: return op->release;
	case 10: return op->ksr;
	case 11: return op->delay;
	case 12: return op->ksl;
	case 13: return op->vib ? (1 + op->vibd) : 0;
	case 14: return op->trm ? (1 + op->trmd) : 0;
	case 15: return op->noise;
	case 16: return op->wave;
	default: return -1;
	}
}

void fm_setreg(ESFM_Operator *op, int regnum, int val)
{
	switch (regnum) {
	case 0: op->out_level = val; break;
	case 1: op->out_left = val; break;
	case 2: op->out_right = val; break;
	case 3: op->mod_level = val; break;
	case 4: op->attenuation = val; break;
	case 5: op->egt = val; break;
	case 6: op->attack = val; break;
	case 7: op->decay = val; break;
	case 8: op->sustain = val; break;
	case 9: op->release = val; break;
	case 10: op->ksr = val; break;
	case 11: op->delay = val; break;
	case 12: op->ksl = val; break;
	case 13: op->vib = val ? 1 : 0; op->vibd = val == 2; break;
	case 14: op->trm = val ? 1 : 0; op->trmd = val == 2; break;
	case 15: op->noise = val; break;
	case 16: op->wave = val; break;
	}
}

const char * fm_frulename(int regnum, int *minval, int *maxval)
{
	int dummy;
	if (!minval) minval = &dummy;
	if (!maxval) maxval = &dummy;

	switch (regnum) {
	case 0: *minval = 0; *maxval = 1; return "NOTE";
	case 1: *minval = -7; *maxval = 7; return "BLOK";
	case 2: *minval = -15; *maxval = 15; return "MULT";
	case 3: *minval = -1023; *maxval = 1023; return "FNUM";
	default: return NULL;
	}
}

int fm_getfrule(ESFM_OpFreqRule *rule, int regnum)
{
	switch (regnum) {
	case 0: return rule->use_note;
	case 1: return rule->dfblock;
	case 2: return rule->dfmult;
	case 3: return rule->dfnum;
	default: return 0;
	}
}

void fm_setfrule(ESFM_OpFreqRule *rule, int regnum, int val)
{
	switch (regnum) {
	case 0: rule->use_note = val; break;
	case 1: rule->dfblock = val; break;
	case 2: rule->dfmult = val; break;
	case 3: rule->dfnum = val; break;
	}
}

