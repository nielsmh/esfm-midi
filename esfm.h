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

extern int FMBASE;

extern unsigned FNUM_BASE[14];
extern unsigned FBLOCK_FMULT_OCTAVE[9][2];

void fm_init(void);
unsigned int fm_regbase(int chan, int op);
unsigned char fm_read(unsigned int reg);
void fm_write(unsigned int reg, unsigned char d);
void fm_setop(int chan, int op, ESFM_Operator *params);
void fm_noteon(int chan);
void fm_noteoff(int chan);
void fm_playchan(ESFM_Channel *chan, char octave, char note);


#endif
