#include <graph.h>
#include <conio.h>
#include <bios.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <i86.h>

#include "esfm.h"
#include "ui.h"
#include "helptext.h"

extern ESFM_Channel *cur_channel;

enum UI_SelectionRow {
	UI_SR_Channel,
	UI_SR_Op1,
	UI_SR_Op2,
	UI_SR_Op3,
	UI_SR_Op4,
	UI_SR_ChanMenu,
	UI_SR_LAST // sentinel to count
};

enum UI_OpFields {
	UI_OF_OutL,
	UI_OF_OutR,
	UI_OF_OutLevel,
	UI_OF_OutAttenuation,
	UI_OF_Scale,
	UI_OF_ModLevel,
	UI_OF_EnvMode,
	UI_OF_EnvAtk,
	UI_OF_EnvDec,
	UI_OF_EnvSus,
	UI_OF_EnvRel,
	UI_OF_EnvScale,
	UI_OF_SoundWave,
	UI_OF_SoundDrum,
	UI_OF_FxTrm,
	UI_OF_FxVib,
	UI_OF_FreqMode,
	UI_OF_FreqBlock,
	UI_OF_FreqMult,
	UI_OF_FreqNum,
	UI_OF_LAST // sentinel to count
};

enum UI_Colorset {
	UI_C_Default,
	UI_C_DimText,
	UI_C_SelectedField,
	UI_C_Header,
	UI_C_HelpBox,
};

static struct {
	enum UI_SelectionRow row;
	enum UI_OpFields op_field;
	int chan_menu_item;
} current_selection;

static void setcolors(enum UI_Colorset c)
{
	switch (c) {
	case UI_C_Default:
		_setbkcolor(_WHITE);
		_settextcolor(0);
		break;
	case UI_C_DimText:
		_setbkcolor(_WHITE);
		_settextcolor(8);
		break;
	case UI_C_SelectedField:
		_setbkcolor(_BLUE);
		_settextcolor(15);
		break;
	case UI_C_Header:
		_setbkcolor(_WHITE);
		_settextcolor(1);
		break;
	case UI_C_HelpBox:
		_setbkcolor(_BLACK);
		_settextcolor(9);
		break;
	}
}

static void draw_help(int full)
{
	static int last_help_index = -1;
	int help_index = -1;
	
	setcolors(UI_C_HelpBox);
	
	if (full) {
		_settextwindow(40, 4, 49, 76);
		_clearscreen(_GWINDOW);
		_settextwindow(47, 6, 48, 74);
		_outtext("Arrow keys move selected field. PgUp/PgDown change values. Esc quits.\n");
		_outtext("The main letter and number keys play notes. Spacebar stops playing.");
	}

	switch (current_selection.row) {
		case UI_SR_Channel: help_index = UIH_Channel; break;
		case UI_SR_Op1:
		case UI_SR_Op2:
		case UI_SR_Op3:
		case UI_SR_Op4:
			switch (current_selection.op_field) {
				case UI_OF_OutL:
				case UI_OF_OutR:
					help_index = UIH_OP_LeftRight;
					break;
				case UI_OF_OutLevel: help_index = UIH_OP_OutLevel; break;
				case UI_OF_OutAttenuation: help_index = UIH_OP_Attenuation; break;
				case UI_OF_Scale: help_index = UIH_OP_Scale; break;
				case UI_OF_ModLevel:
					help_index = (current_selection.row == UI_SR_Op1) ? UIH_OP_FeedbackLevel : UIH_OP_ModLevel;
					break;
				case UI_OF_EnvMode: help_index = UIH_OP_EnvMode; break;
				case UI_OF_EnvAtk: help_index = UIH_OP_EnvAtk; break;
				case UI_OF_EnvDec: help_index = UIH_OP_EnvDec; break;
				case UI_OF_EnvSus: help_index = UIH_OP_EnvSus; break;
				case UI_OF_EnvRel: help_index = UIH_OP_EnvRel; break;
				case UI_OF_EnvScale: help_index = UIH_OP_EnvScale; break;
				case UI_OF_SoundWave: help_index = UIH_OP_SoundWave; break;
				case UI_OF_SoundDrum: help_index = UIH_OP_SoundDrum; break;
				case UI_OF_FxTrm: help_index = UIH_OP_FxTrm; break;
				case UI_OF_FxVib: help_index = UIH_OP_FxVib; break;
				case UI_OF_FreqMode: help_index = UIH_OP_FreqMode; break;
				case UI_OF_FreqBlock: help_index = UIH_OP_FreqBlock; break;
				case UI_OF_FreqMult: help_index = UIH_OP_FreqMult; break;
				case UI_OF_FreqNum: help_index = UIH_OP_FreqNum; break;
			}
			break;
	}

	_settextwindow(41, 6, 46, 74);

	if (help_index != last_help_index) {
		_clearscreen(_GWINDOW);
		if (help_index >= 0 && help_index < UIH_LAST) {
			_outtext(HELPSTRINGS[help_index]);
			last_help_index = help_index;
		}
	}

	_settextwindow(1, 1, 50, 80);
}

static void draw_channel_select_row(const ESFM_Channel *channel, int is_selected)
{
	char buf[81];

	// Line 1: "Channel X"
	_settextposition(3, 3);
	sprintf(buf, "Channel %-2d", channel->channel + 1);
	if (is_selected) {
		setcolors(UI_C_SelectedField);
	} else {
		setcolors(UI_C_Default);
	}
	_outtext(buf);
}

static void draw_op_headers(void)
{
	setcolors(UI_C_Header);
	_settextposition(4, 4);
	_outtext("Out             Mod  Envelope              Sound    Fx   Frequency         ");
	_settextposition(5, 4);
	_outtext("LR Lev Attn Sc  Lev  M Atk Dec Sus Rel Sc  Wav Drm  T V  Mode Blk Mul FrNum");
}


static void draw_op_field_sel(const ESFM_Channel *channel, int op, enum UI_OpFields field, int is_selected)
{
	static short field_positions[UI_OF_LAST][2] = {
		// First and last position (inclusive) each field is printed on
		{1, 1},   // L
		{2, 2},   // R
		{4, 6},   // Out Level
		{8, 11},  // Attenuation
		{14, 14}, // Out Scale
		{17, 19}, // Mod Level
		{22, 22}, // Env Mode
		{24, 26}, // Env Attack
		{28, 30}, // Env Decay
		{32, 34}, // Env Sustain
		{36, 38}, // Env Release
		{41, 41}, // Env Scale
		{44, 46}, // Wave
		{48, 50}, // Drum
		{53, 53}, // Fx Tremolo
		{55, 55}, // Fx Vibrato
		{58, 60}, // Freq Mode
		{63, 65}, // Freq Block
		{67, 69}, // Freq Mult
		{71, 75}, // Freq Num
	};
	// Calculate position of the field
	short row = 6 + op;
	short pos1 = field_positions[field][0] + 3;
	short pos2 = field_positions[field][1] + 3 + 1;
	int is_default = 0;
	char buf[10] = "";

	// Format the field
	switch (field) {
	case UI_OF_OutL:
		strcpy(buf, channel->op[op].out_left ? "L" : " ");
		break;
	case UI_OF_OutR:
		strcpy(buf, channel->op[op].out_right ? "R" : " ");
		break;
	case UI_OF_OutLevel:
		itoa(channel->op[op].out_level, buf, 10);
		is_default = channel->op[op].out_level == 0;
		break;
	case UI_OF_OutAttenuation:
		itoa(channel->op[op].attenuation, buf, 10);
		is_default = channel->op[op].attenuation == 0;
		break;
	case UI_OF_Scale:
		itoa(channel->op[op].ksl, buf, 10);
		is_default = channel->op[op].ksl == 0;
		break;
	case UI_OF_ModLevel:
		itoa(channel->op[op].mod_level, buf, 10);
		is_default = channel->op[op].mod_level == 0;
		break;
	case UI_OF_EnvMode:
		strcpy(buf, channel->op[op].egt ? "H" : "T");
		break;
	case UI_OF_EnvAtk:
		itoa(channel->op[op].attack, buf, 10);
		is_default = channel->op[op].attack == 0;
		break;
	case UI_OF_EnvDec:
		itoa(channel->op[op].decay, buf, 10);
		is_default = channel->op[op].decay == 0;
		break;
	case UI_OF_EnvSus:
		itoa(channel->op[op].sustain, buf, 10);
		is_default = channel->op[op].sustain == 0;
		break;
	case UI_OF_EnvRel:
		itoa(channel->op[op].release, buf, 10);
		is_default = channel->op[op].release == 0;
		break;
	case UI_OF_EnvScale:
		itoa(channel->op[op].ksr, buf, 10);
		is_default = channel->op[op].ksr == 0;
		break;
	case UI_OF_SoundWave:
		itoa(channel->op[op].wave, buf, 10);
		break;
	case UI_OF_SoundDrum:
		itoa(channel->op[op].noise, buf, 10);
		is_default = channel->op[op].noise == 0;
		break;
	case UI_OF_FxTrm:
		itoa(channel->op[op].trm ? (1 + channel->op[op].trmd) : 0, buf, 10);
		is_default = channel->op[op].trm == 0;
		break;
	case UI_OF_FxVib:
		itoa(channel->op[op].vib ? (1 + channel->op[op].vibd) : 0, buf, 10);
		is_default = channel->op[op].vib == 0;
		break;
	case UI_OF_FreqMode:
		strcpy(buf, channel->frule[op].use_note ? "Key" : "Fix");
		break;
	case UI_OF_FreqBlock:
		itoa(channel->frule[op].dfblock, buf, 10);
		is_default = channel->frule[op].dfblock == 0;
		break;
	case UI_OF_FreqMult:
		itoa(channel->frule[op].dfmult, buf, 10);
		is_default = channel->frule[op].dfmult == 0;
		break;
	case UI_OF_FreqNum:
		itoa(channel->frule[op].dfnum, buf, 10);
		is_default = channel->frule[op].dfnum == 0;
		break;
	}

	// Set colors
	if (is_selected) {
		setcolors(UI_C_SelectedField);
	} else if (is_default) {
		setcolors(UI_C_DimText);
	} else {
		setcolors(UI_C_Default);
	}

	// Print
	_settextposition(row, pos1);
	// Spaces before
	pos1 = (pos2 - (pos1 + strlen(buf)) + 1) >> 1;
	while (pos1-- > 0) _outtext(" ");
	// The text
	_outtext(buf);
	// Spaces after
	pos1 = _gettextposition().col;
	while (pos1++ < pos2) _outtext(" ");
}

static inline void draw_op_field(const ESFM_Channel *channel, int op, enum UI_OpFields field)
{
	draw_op_field_sel(channel, op, field, (current_selection.row == (UI_SR_Op1 + op)) && (current_selection.op_field == field));
}

static void draw_op_row(const ESFM_Channel *channel, int op)
{
	char buf[81];
	int field;
	
	// Blank out the line
	_settextposition(6 + op, 1);
	setcolors(UI_C_Header);
	sprintf(buf, " %d                                                                            ", op + 1);
	_outtext(buf);
	// Draw the parameters
	for (field = UI_OF_OutL; field < UI_OF_LAST; ++field) {
		draw_op_field(channel, op, field);
	}
}

static void draw_op_params(const ESFM_Channel *channel)
{
	int op;
	
	draw_channel_select_row(channel, current_selection.row == UI_SR_Channel);
	draw_op_headers();
	for (op = 0; op < 4; ++op) draw_op_row(channel, op);
}

static short orgvideomode;
static short orgtextrows;
static short orgcursor;

int ui_init(void)
{
	int r;
	struct videoconfig vconfig;

	orgcursor = _displaycursor(_GCURSOROFF);
	if (orgcursor == -1) return 0;

	_getvideoconfig(&vconfig);
	orgvideomode = vconfig.mode;
	orgtextrows = vconfig.numtextrows;

	r = _setvideomoderows(_TEXTC80, 50);
	if (!r) return 0;
	
	_setbkcolor(_WHITE);
	_settextcolor(_BLACK);
	_clearscreen(_GCLEARSCREEN);
	_displaycursor(_GCURSOROFF);

	memset(&current_selection, 0, sizeof(current_selection));
	
	return 1;
}

void ui_finalize(void)
{
	_setvideomoderows(orgvideomode, orgtextrows);
	_setbkcolor(_BLACK);
	_clearscreen(_GCLEARSCREEN);
	_displaycursor(orgcursor);
}

static void noise(int n)
{
	sound(n);
	delay(40);
	nosound();
}

static enum UI_Command input_check_note(unsigned int keypress, int *note, int *octave)
{
	switch (keypress >> 8) {
		// Q2W3E
		case 0x10: *note =  1; *octave = 2; return UICMD_NOTEON;
		case 0x03: *note =  2; *octave = 2; return UICMD_NOTEON;
		case 0x11: *note =  3; *octave = 2; return UICMD_NOTEON;
		case 0x04: *note =  4; *octave = 2; return UICMD_NOTEON;
		case 0x12: *note =  5; *octave = 2; return UICMD_NOTEON;
		// R5T6Y7U
		case 0x13: *note =  6; *octave = 2; return UICMD_NOTEON;
		case 0x06: *note =  7; *octave = 2; return UICMD_NOTEON;
		case 0x14: *note =  8; *octave = 2; return UICMD_NOTEON;
		case 0x07: *note =  9; *octave = 2; return UICMD_NOTEON;
		case 0x15: *note = 10; *octave = 2; return UICMD_NOTEON;
		case 0x08: *note = 11; *octave = 2; return UICMD_NOTEON;
		case 0x16: *note = 12; *octave = 2; return UICMD_NOTEON;
		// I9O0P
		case 0x17: *note =  1; *octave = 3; return UICMD_NOTEON;
		case 0x0A: *note =  2; *octave = 3; return UICMD_NOTEON;
		case 0x18: *note =  3; *octave = 3; return UICMD_NOTEON;
		case 0x0B: *note =  4; *octave = 3; return UICMD_NOTEON;
		case 0x19: *note =  5; *octave = 3; return UICMD_NOTEON;
		// ZSXDCFV
		case 0x2C: *note =  6; *octave = 3; return UICMD_NOTEON;
		case 0x1F: *note =  7; *octave = 3; return UICMD_NOTEON;
		case 0x2D: *note =  8; *octave = 3; return UICMD_NOTEON;
		case 0x20: *note =  9; *octave = 3; return UICMD_NOTEON;
		case 0x2E: *note = 10; *octave = 3; return UICMD_NOTEON;
		case 0x21: *note = 11; *octave = 3; return UICMD_NOTEON;
		case 0x2F: *note = 12; *octave = 3; return UICMD_NOTEON;
		// BHNJM
		case 0x30: *note =  1; *octave = 4; return UICMD_NOTEON;
		case 0x23: *note =  2; *octave = 4; return UICMD_NOTEON;
		case 0x31: *note =  3; *octave = 4; return UICMD_NOTEON;
		case 0x24: *note =  4; *octave = 4; return UICMD_NOTEON;
		case 0x32: *note =  5; *octave = 4; return UICMD_NOTEON;
		// space (stop playing)
		case 0x39: return UICMD_NOTEOFF;
		// Not a note key
		default: return UICMD_NONE;
	}
}

static void move_row(int delta)
{
	// change row
	int drawn = 0;
	int old_row = current_selection.row;
	int new_row = current_selection.row + delta;
	if (new_row < 0) new_row = 0;
	if (new_row >= UI_SR_LAST) new_row = UI_SR_LAST - 1;
	current_selection.row = new_row;

	// redraw
	if (old_row >= UI_SR_Op1 && old_row <= UI_SR_Op4) {
		draw_op_field_sel(cur_channel, old_row - UI_SR_Op1, current_selection.op_field, 0);
		++drawn;
	}
	if (new_row >= UI_SR_Op1 && new_row <= UI_SR_Op4) {
		draw_op_field_sel(cur_channel, new_row - UI_SR_Op1, current_selection.op_field, 1);
		++drawn;
	}
	if (drawn != 2) {
		// TODO: be more efficient
		draw_op_params(cur_channel);
	}

	draw_help(0);
}

static void move_col(int delta)
{
	int old_col;
	int new_col;
	switch (current_selection.row) {
		case UI_SR_Op1:
		case UI_SR_Op2:
		case UI_SR_Op3:
		case UI_SR_Op4:
			// keep previous for redraw
			old_col = current_selection.op_field;
			// calculate new value
			new_col = current_selection.op_field + delta;
			if (new_col < 0) new_col = UI_OF_LAST - 1;
			if (new_col >= UI_OF_LAST) new_col = 0;
			current_selection.op_field = new_col;
			// redraw
			draw_op_field_sel(cur_channel, current_selection.row - UI_SR_Op1, old_col, 0);
			draw_op_field_sel(cur_channel, current_selection.row - UI_SR_Op1, new_col, 1);
			break;
		case UI_SR_ChanMenu:
			// keep previous for redraw
			old_col = current_selection.chan_menu_item;
			// calculate new value
			new_col = old_col + delta;
			if (new_col < 0) new_col = 0;
			if (new_col > 3) new_col = 3;
			current_selection.chan_menu_item = new_col;
			// redraw
			// TODO
			break;
		default:
			// Nothing to change here
			break;
	}

	draw_help(0);
}

static inline int clamp(int val, int min, int max)
{
	if (val < min) return min;
	if (val > max) return max;
	return val;
}

static int change_op_param(int delta, int opnum)
{
	ESFM_Operator *op = &cur_channel->op[opnum];
	ESFM_OpFreqRule *frule = &cur_channel->frule[opnum];
	int old_val, new_val;
	switch (current_selection.op_field) {
		case UI_OF_OutL:
			op->out_left = !op->out_left;
			return 1;
		case UI_OF_OutR:
			op->out_right = !op->out_right;
			return 1;
		case UI_OF_OutLevel:
			old_val = op->out_level;
			new_val = clamp(old_val + delta, 0, 7);
			if (old_val != new_val) op->out_level = new_val;
			return old_val != new_val;
		case UI_OF_OutAttenuation:
			old_val = op->attenuation;
			new_val = clamp(old_val + delta, 0, 63);
			if (old_val != new_val) op->attenuation = new_val;
			return old_val != new_val;
		case UI_OF_Scale:
			old_val = op->ksl;
			new_val = clamp(old_val + delta, 0, 3);
			if (old_val != new_val) op->ksl = new_val;
			return old_val != new_val;
		case UI_OF_ModLevel:
			old_val = op->mod_level;
			new_val = clamp(old_val + delta, 0, 7);
			if (old_val != new_val) op->mod_level = new_val;
			return old_val != new_val;
		case UI_OF_EnvMode:
			op->egt = !op->egt;
			return 1;
		case UI_OF_EnvAtk:
			old_val = op->attack;
			new_val = clamp(old_val + delta, 0, 15);
			if (old_val != new_val) op->attack = new_val;
			return old_val != new_val;
		case UI_OF_EnvDec:
			old_val = op->decay;
			new_val = clamp(old_val + delta, 0, 15);
			if (old_val != new_val) op->decay = new_val;
			return old_val != new_val;
		case UI_OF_EnvSus:
			old_val = op->sustain;
			new_val = clamp(old_val + delta, 0, 15);
			if (old_val != new_val) op->sustain = new_val;
			return old_val != new_val;
		case UI_OF_EnvRel:
			old_val = op->release;
			new_val = clamp(old_val + delta, 0, 15);
			if (old_val != new_val) op->release = new_val;
			return old_val != new_val;
		case UI_OF_EnvScale:
			op->ksr = !op->ksr;
			return 1;
		case UI_OF_SoundWave:
			old_val = op->wave;
			new_val = clamp(old_val + delta, 0, 7);
			if (old_val != new_val) op->wave = new_val;
			return old_val != new_val;
		case UI_OF_SoundDrum:
			old_val = op->noise;
			new_val = clamp(old_val + delta, 0, 3);
			if (old_val != new_val) op->noise = new_val;
			return old_val != new_val;
		case UI_OF_FxTrm:
			old_val = op->trm ? (1 + op->trmd) : 0;
			new_val = clamp(old_val + delta, 0, 2);
			op->trm = new_val >= 1;
			op->trmd = new_val == 2;
			return old_val != new_val;
		case UI_OF_FxVib:
			old_val = op->vib ? (1 + op->vibd) : 0;
			new_val = clamp(old_val + delta, 0, 2);
			op->vib = new_val >= 1;
			op->vibd = new_val == 2;
			return old_val != new_val;
		case UI_OF_FreqMode:
			frule->use_note = !frule->use_note;
			return 1;
		case UI_OF_FreqBlock:
			old_val = frule->dfblock;
			new_val = clamp(old_val + delta, -7, 7);
			if (old_val != new_val) frule->dfblock = new_val;
			return old_val != new_val;
		case UI_OF_FreqMult:
			old_val = frule->dfmult;
			new_val = clamp(old_val + delta, -15, 15);
			if (old_val != new_val) frule->dfmult = new_val;
			return old_val != new_val;
		case UI_OF_FreqNum:
			old_val = frule->dfnum;
			new_val = clamp(old_val + delta, -1023, 1023);
			if (old_val != new_val) frule->dfnum = new_val;
			return old_val != new_val;
		default:
			return 0;
	}
}

static int change_param(int delta)
{
	switch (current_selection.row) {
		case UI_SR_Channel:
			// ignore, for now
			return 0;
		case UI_SR_Op1:
		case UI_SR_Op2:
		case UI_SR_Op3:
		case UI_SR_Op4:
			if (change_op_param(delta, current_selection.row - UI_SR_Op1)) {
				draw_op_field_sel(cur_channel, current_selection.row - UI_SR_Op1, current_selection.op_field, 1);
				return 1;
			} else {
				return 0;
			}
		default:
			return 0;
	}
}

static enum UI_Command input_check_navigate(unsigned int keypress)
{
	switch (keypress) {
		case 0x4BE0: // Left
			noise(300);
			move_col(-1);
			return UICMD_NAVIGATE;
		case 0x4DE0: // Right
			noise(400);
			move_col(+1);
			return UICMD_NAVIGATE;
		case 0x48E0: // Up
			noise(200);
			move_row(-1);
			return UICMD_NAVIGATE;
		case 0x50E0: // Down
			noise(600);
			move_row(+1);
			return UICMD_NAVIGATE;
		case 0x49E0: // PageUp
			noise(100);
			return change_param(+1) ? UICMD_PARMCHANGE : UICMD_NONE;
		case 0x51E0: // PageDown
			noise(150);
			return change_param(-1) ? UICMD_PARMCHANGE : UICMD_NONE;
		case 0x011B: // ESC
			return UICMD_QUIT;
		default:
			return UICMD_NONE;
	}
}

int ui_handleinput(UI_Input *inputdata)
{
	static int need_full_redraw = 1;
	unsigned short keypress;
	enum UI_Command cmd = UICMD_NONE;
	
	if (need_full_redraw) {
		draw_help(1);
		draw_op_params(cur_channel);
		need_full_redraw = 0;
	}

	if(_bios_keybrd(_NKEYBRD_READY)) {
		keypress = _bios_keybrd(_NKEYBRD_READ);
		cmd = input_check_navigate(keypress);
		if (cmd == UICMD_PARMCHANGE && inputdata) inputdata->parm1 = current_selection.row - UI_SR_Op1;
		if (cmd == UICMD_NONE && inputdata) cmd = input_check_note(keypress, &inputdata->parm1, &inputdata->parm2);
	}

	if (inputdata) inputdata->command = cmd;

	return cmd != UICMD_NONE;
}

