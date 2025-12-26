#ifndef ESFM_H
#define ESFM_H

typedef struct {
	unsigned fmult : 4;
	unsigned ksr : 1;
	unsigned egt : 1;
	unsigned vib : 1;
	unsigned trm : 1;
	unsigned attenuation : 6;
	unsigned ksl : 2;
	unsigned attack : 4;
	unsigned decay : 4;
	unsigned sustain : 4;
	unsigned release : 4;
	unsigned fnum : 10;
	unsigned fblock : 3;
	unsigned delay : 3;
	unsigned _unk1 : 1;
	unsigned mod_level : 3;
	unsigned out_left : 1;
	unsigned out_right : 1;
	unsigned vibd : 1;
	unsigned trmd : 1;
	unsigned wave : 3;
	unsigned noise : 2;
	unsigned out_level : 3;
} ESFM_Operator;

typedef struct {
	int use_note : 1;
	int dfblock : 7;
	int dfmult : 8;
	int dfnum;
} ESFM_OpFreqRule;

typedef struct {
	ESFM_Operator op[4];
	ESFM_OpFreqRule frule[4];
	char channel;
	char playing;
	char octave;
	char note;
} ESFM_Channel;

/** Base port of the ESFM chip */
extern int FMBASE;

/** Base frequency number for notes B-C across one whole octave */
extern const unsigned FNUM_BASE[14];
/** Frequency block and multiplier values for 9 octave range */
extern const unsigned FBLOCK_FMULT_OCTAVE[9][2];

/** Initialize ESFM chip */
void fm_init(void);
/** Get the base register number for a specific operator of a channel */
unsigned int fm_regbase(int chan, int op);
/** Read an ESFM register */
unsigned char fm_read(unsigned int reg);
/** Write an ESFM register */
void fm_write(unsigned int reg, unsigned char d);
/** Set the full parameter set for an ESFM operator */
void fm_setop(int chan, int op, ESFM_Operator *params);
/** Set the note on flag for an ESFM channel */
void fm_noteon(int chan);
/** Clear the note on flag for an ESFM channel */
void fm_noteoff(int chan);
/** Start playing a note on an ESFM channel, setting the frequency parameters */
void fm_playchan(ESFM_Channel *chan, int octave, int note);

/** Get the name and maximum value for an ESFM operator register */
const char * fm_regname(int regnum, int *maxval);
/** Get the value of an ESFM operator register */
int fm_getreg(ESFM_Operator *op, int regnum);
/** Set the value of an ESFM operator register */
void fm_setreg(ESFM_Operator *op, int regnum, int val);

/** Get the name and max/min values for an ESFM operator frequency rule */
const char * fm_frulename(int regnum, int *minval, int *maxval);
/** Get the value for an ESFM operator frequency rule */
int fm_getfrule(ESFM_OpFreqRule *rule, int regnum);
/** Set the value for an ESFM operator frequency rule */
void fm_setfrule(ESFM_OpFreqRule *rule, int regnum, int val);


#endif
