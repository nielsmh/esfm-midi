#ifndef UI_H
#define UI_H

enum UI_Command {
	UICMD_NONE,
	UICMD_NAVIGATE,
	UICMD_QUIT,
	UICMD_ALL_NOTES_OFF,
	UICMD_NOTEON,
	UICMD_NOTEOFF,
	UICMD_PARMCHANGE,
};

typedef struct {
	enum UI_Command command;
	int parm1;
	int parm2;
} UI_Input;

int ui_init(void);
void ui_finalize(void);
int ui_handleinput(UI_Input *inputdata);

#endif

