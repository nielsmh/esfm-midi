#include "helptext.h"

const char *HELPSTRINGS[UIH_LAST] = {
	// UIH_Channel
	"Select which channel is being edited.",
	// UIH_OP_LeftRight
	"Toggle output of this operator on left/right stereo channel.",
	// UIH_OP_OutLevel
	"Select direct output level of this operator.",
	// UIH_OP_Attenuation
	"Lowers the overall level of this operator, both for output and modulation.",
	// UIH_OP_Scale
	"Scale the operator's level depending on pitch.",
	// UIH_OP_FeedbackLevel
	"Modulate the first operator by feedback from the channel output.",
	// UIH_OP_ModLevel
	"Modulate the operator by the previous operator.",
	// UIH_OP_EnvMode
	"Envelope mode T one-shot triggers each note.\nMode H holds each note until key-off.",
	// UIH_OP_EnvAtk
	"Attack speed, how fast the operator rises to maximum level after key-on.",
	// UIH_OP_EnvDec
	"Decay speed, how fast the operator decays to the sustain level after reaching maximum.",
	// UIH_OP_EnvSus
	"Sustain attenuation level, higher values makes the sustain level softer.",
	// UIH_OP_EnvRel
	"Release speed, how fast the operator stops playing after key-off.",
	// UIH_OP_EnvScale
	"Scale the envelope depending on the pitch.",
	// UIH_OP_SoundWave
	"Waveform for the operator.",
	// UIH_OP_SoundDrum
	"Drum noise for the operator, only has effect on the last operator.",
	// UIH_OP_FxTrm
	"Tremolo level for the operator.",
	// UIH_OP_FxVib
	"Vibrato level for the operator.",
	// UIH_OP_FreqMode
	"Select frequency for the operator: Key is relative to the triggered key. Fixed is not.",
	// UIH_OP_FreqBlock
	"Octave shift for the operator pitch.",
	// UIH_OP_FreqMult
	"Multiplier for the operator pitch.",
	// UIH_OP_FreqNum
	"Frequency number shift.",
};

