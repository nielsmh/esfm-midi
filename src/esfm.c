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


int main()
{
	// input
	UI_Input ui_input;
	/* current input and run state */
	int run = 1;

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
					fm_setop(cur_channel->channel, ui_input.parm1, &cur_channel->op[ui_input.parm1]);
					break;
			}
		}
	}

	ui_finalize();

	return 0;
}
