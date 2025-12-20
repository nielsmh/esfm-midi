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

#endif
