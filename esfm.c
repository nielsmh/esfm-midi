#include <stdio.h>
#include <string.h>
#include <dos.h>
#include <conio.h>
#include <bios.h>
#include <mem.h>
#include <graph.h>

#include "esfm.h"
#include "ui.h"

/* the channels on the synth */
ESFM_Channel all_channels[16];

/* channel we're playing on */
ESFM_Channel *cur_channel = &all_channels[0];

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

	if (cur_channel->playing) {
		sprintf(playstatus, "%d-%d", cur_channel->octave, cur_channel->note);
	} else {
		sprintf(playstatus, " - ");
	}

	switch (editmode) {
	case 0:
		editmodestr = "Param";
		editregnamestr = fm_regname(editregnum, &regvalmax);
		regval = fm_getreg(&cur_channel->op[editop], editregnum);
		break;
	case 1:
		editmodestr = "FRule";
		editregnamestr = fm_frulename(editregnum, NULL, &regvalmax);
		regval = fm_getfrule(&cur_channel->frule[editop], editregnum);
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
	fflush(stdout);
}

void regedit_change(int delta)
{
	int minval = 0, maxval = 0, curval;
	switch (editmode) {
	case 0:
		fm_regname(editregnum, &maxval);
		curval = fm_getreg(&cur_channel->op[editop], editregnum);
		curval += delta;
		if (curval < minval) curval = minval;
		if (curval > maxval) curval = maxval;
		fm_setreg(&cur_channel->op[editop], editregnum, curval);
		break;
	case 1:
		fm_frulename(editregnum, &minval, &maxval);
		curval = fm_getfrule(&cur_channel->frule[editop], editregnum);
		curval += delta;
		if (curval < minval) curval = minval;
		if (curval > maxval) curval = maxval;
		fm_setfrule(&cur_channel->frule[editop], editregnum, curval);
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
	UI_Input ui_input;
	unsigned short keypress;
	int note = 1;
	int octave = 3;

	if (!ui_init()) return 1;

	memset(all_channels, 0, sizeof(all_channels));

	// Basic simple frequency rule
	cur_channel->frule[0].use_note = 1;
	cur_channel->frule[1].use_note = 1;
	cur_channel->frule[2].use_note = 1;
	cur_channel->frule[3].use_note = 1;

	// Envelope
	cur_channel->op[0].egt = 1;
	cur_channel->op[0].attack = 7;
	cur_channel->op[0].decay = 3;
	cur_channel->op[0].sustain = 4;
	cur_channel->op[0].release = 5;
	cur_channel->op[0].delay = 0;
	// Tone
	cur_channel->op[0].wave = 1;
	cur_channel->op[0].noise = 0;
	cur_channel->op[0].ksr = 0;
	cur_channel->op[0].vib = 0;
	cur_channel->op[0].trm = 0;
	cur_channel->op[0].ksl = 0;
	cur_channel->op[0].vibd = 0;
	cur_channel->op[0].trmd = 0;
	// Levels
	cur_channel->op[0].attenuation = 0;
	cur_channel->op[0].mod_level = 0;
	cur_channel->op[0].out_left = 1;
	cur_channel->op[0].out_right = 1;
	cur_channel->op[0].out_level = 7;

	fm_init();

	while (run) {
		if (ui_handleinput(&ui_input)) {
			switch (ui_input.command) {
				case UICMD_QUIT:
					fm_noteoff(cur_channel->channel);
					run = 0;
					break;
				case UICMD_NOTEON:
					fm_playchan(cur_channel, ui_input.parm2, ui_input.parm1);
					break;
				case UICMD_ALL_NOTES_OFF:
				case UICMD_NOTEOFF:
					fm_noteoff(cur_channel->channel);
					cur_channel->playing = 0;
					break;
				case UICMD_PARMCHANGE:
					fm_setop(cur_channel->channel, ui_input.parm1, &cur_channel->op[editop]);
					break;
			}
		}

	/*
		keypress = _bios_keybrd(_NKEYBRD_READ);
		printf("<%04x>", keypress); fflush(stdout);
		note = 0;

		if (note > 0) {
			fm_playchan(cur_channel, octave, note);
		}
		if (note == -1) {
			// sound off
			fm_noteoff(cur_channel->channel);
			cur_channel->playing = 0;
		}
		if (note == -2 && cur_channel->playing) {
			// sound off after changing parameter
			fm_noteoff(cur_channel->channel);
			cur_channel->playing = 0;
		}
		if (note == -3) {
			// parameter change, update
			fm_setop(cur_channel->channel, editop, &cur_channel->op[editop]);
		}
		if (note == -255) {
			// ignore?
		}
	*/
	}

	ui_finalize();

	return 0;
}
