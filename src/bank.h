#ifndef BANK_H
#define BANK_H

#include <stdio.h>
#include "esfm.h"

/** A Bank Key Rule controls a key trigger's effect on op parameters */
typedef struct {
	/** Use the note value for frequency data base */
	short use_note : 1;
	/** Use the key-down velocity for attenuation scaling */
	short use_vel : 1;
	/** Delta frequency block number*/
	short dfblock : 7;
	/** Delta frequency multiplier number */
	short dfmult : 7;
	/** Frequncy number fractional adjust
	 *  When use_use = 0 this value is used as absolute fnum value for the operator.
	 *  Otherwise:
	 *  When = 0, do not adjust the calculated fnum.
	 *  When < 0, use fnum = fnum * 32768 / (32768 + dfnum) [clamp dfnum to range -32767..-1]
	 *  When > 0, use fnum = fnum * (32768 - dfnum) / 32768
	 */
	short dfnum;
} ESFM_BankKeyRule;

/** A bank entry consists of a base set of operators, and corresponding key rules */
typedef struct {
	ESFM_Operator op[4];
	ESFM_BankKeyRule keyrule[4];
} ESFM_BankEntry;

/** A playable channel contains operator data, a bank reference, and trigger data */
typedef struct {
    /** FM operator data that can be written to chip */
    ESFM_Operator op[4];
    /** Reference to bank data used to fill the operator data */
    const ESFM_BankEntry *bank;
    /** MIDI note value to play (range 0..127) */
    int midinote;
    /** Key-down velocity (range 0..127) */
    int velocity;
    /** Volume level (range 0..127) */
    int volume;
    /** Pitch bend level (range -127..127) */
    int pitchbend;
} ESFM_PlayableChannel;

/** Global soundbank data */
extern ESFM_BankEntry soundbank_data[128];


/** Reset the global soundbank data to a known state */
void bank_init(void);
/** Write the global soundbank data to an open binary file */
void bank_write(FILE *file);
/** Load into the global soundbank data from an open binary file */
void bank_read(FILE *file);

/** Apply a bank to a playable channel
 *  Input: 
 *    ch->bank
 *    ch->midinote
 *    ch->velocity
 *    ch->volume
 *    ch->pitchbend
 *  Output:
 *    ch->op[]
 */
void bank_apply(ESFM_PlayableChannel *ch);

#endif

