#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <bios.h>
#include <mem.h>

#include "esfm.h"

/* channel we're playing on */
static ESFM_Channel channel;

/* current input and run state */
static int run = 1;

static int editregnum = 0;
static int editmode = 0;
static int editop = 0;

void print_status_line(void)
{
	char playstatus[6];
	const char *editmodestr;
	const char *editregnamestr;
	char regvalstr[8];
	int regval, regvalmax;

	if (channel.playing) {
		sprintf(playstatus, "%d-%d", channel.octave, channel.note);
	} else {
		sprintf(playstatus, " - ");
	}

	switch (editmode) {
	case 0:
		editmodestr = "Param";
		editregnamestr = fm_regname(editregnum, &regvalmax);
		regval = fm_getreg(&channel.op[editop], editregnum);
		break;
	case 1:
		editmodestr = "FRule";
		editregnamestr = fm_frulename(editregnum, NULL, &regvalmax);
		regval = fm_getfrule(&channel.frule[editop], editregnum);
		break;
	default:
		editmodestr = "?what?";
		editregnamestr = "reg?";
		regval = 0;
		break;
	}

	if (regvalmax == 1) {
		if (regval) strcpy(regvalstr, "ON");
		else strcpy(regvalstr, "OFF");
	} else {
		sprintf(regvalstr, "%d", regval);
	}

	printf(" %-5s > OP-%d  %6s %2d %-4s = %-5s\r",
		playstatus,
		editop + 1,
		editmodestr,
		editregnum,
		editregnamestr,
		regvalstr
		);
}

void regedit_change(int delta)
{
	int minval = 0, maxval = 0, curval;
	switch (editmode) {
	case 0:
		fm_regname(editregnum, &maxval);
		curval = fm_getreg(&channel.op[editop], editregnum);
		curval += delta;
		if (curval < minval) curval = minval;
		if (curval > maxval) curval = maxval;
		fm_setreg(&channel.op[editop], editregnum, curval);
		break;
	case 1:
		fm_frulename(editregnum, &minval, &maxval);
		curval = fm_getfrule(&channel.frule[editop], editregnum);
		curval += delta;
		if (curval < minval) curval = minval;
		if (curval > maxval) curval = maxval;
		fm_setfrule(&channel.frule[editop], editregnum, curval);
      break;
	}
}


void help(void)
{
	puts("Letters Q-P and Z-M and keys above them play notes. Space stops. ESC exits.");
	puts("F1-F4 selects an operator to edit.");
	puts("F5-F6 selects operator parameters to edit.");
	puts("F7-F8 selects operator f-rule parameters to edit.");
	puts("F9-F10 changes value of the selected parameter.");
	puts("Operator parameters are sent to the synth immediately after edit.");
	puts("Changes to f-rules need a new note trigger to take effect.");
}


int main()
{
	// input
	int note = 1;
	int octave = 3;

	memset(&channel, 0, sizeof(channel));

	// Basic simple frequency rule
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

	printf("Setting up synth...\n");
	fm_init();
	help();
	print_status_line();

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
			// space (stop playing)
			case 0x39: note = -1; break;
			// escape (quit)
			case 0x01: run = 0; note = -1; break;
			// F1-F4 (select operator to edit)
			case 0x3B: case 0x3C: case 0x3D: case 0x3E:
				editop = (note >> 8) - 0x3B;
				note = 0;
				break;
			// F5-F8 (select parameter to edit)
			case 0x3F: case 0x40:
				if (editmode != 0) {
					editmode = 0;
					editregnum = 0;
				} else if ((note >> 8) == 0x3F) {
					editregnum -= 1;
					if (editregnum < 0) editregnum = 18;
				} else {
					editregnum += 1;
					if (editregnum > 18) editregnum = 0;
				}
				note = 0;
				break;
			case 0x41: case 0x42:
				if (editmode != 1) {
					editmode = 1;
					editregnum = 0;
				} else if ((note >> 8) == 0x41) {
					editregnum -= 1;
					if (editregnum < 0) editregnum = 3;
				} else {
					editregnum += 1;
					if (editregnum > 3) editregnum = 0;
				}
				note = 0;
				break;
			// F9-F10 (edit parameter)
			case 0x43:
				regedit_change(-1);
				note = -3;
				break;
			case 0x44:
				regedit_change(+1);
				note = -3;
				break;
			default:  note = -255; break;
		}

		if (note > 0) {
			fm_playchan(&channel, octave, note);
		}
		if (note == -1) {
			// sound off
			fm_noteoff(channel.channel);
			channel.playing = 0;
		}
		if (note == -2 && channel.playing) {
			// sound off after changing parameter
			fm_noteoff(channel.channel);
			channel.playing = 0;
		}
		if (note == -3) {
			// parameter change, update
			fm_setop(channel.channel, editop, &channel.op[editop]);
		}
		if (note == -255) {
			// ignore?
		}

		print_status_line();
	}

	puts("\nGoodbye\n");

	return 0;
}
