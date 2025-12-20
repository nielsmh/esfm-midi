#ifndef HELPTEXT_H
#define HELPTEXT_H

enum UI_Helptext {
	UIH_Channel,
	UIH_OP_LeftRight,
	UIH_OP_OutLevel,
	UIH_OP_Attenuation,
	UIH_OP_Scale,
	UIH_OP_FeedbackLevel,
	UIH_OP_ModLevel,
	UIH_OP_EnvMode,
	UIH_OP_EnvAtk,
	UIH_OP_EnvDec,
	UIH_OP_EnvSus,
	UIH_OP_EnvRel,
	UIH_OP_EnvScale,
	UIH_OP_SoundWave,
	UIH_OP_SoundDrum,
	UIH_OP_FxTrm,
	UIH_OP_FxVib,
	UIH_OP_FreqMode,
	UIH_OP_FreqBlock,
	UIH_OP_FreqMult,
	UIH_OP_FreqNum,
	UIH_LAST // sentinel to count
};

extern const char *HELPSTRINGS[UIH_LAST];

#endif

