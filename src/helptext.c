#include "helptext.h"

const char *HELPSTRINGS[UIH_LAST] = {
	/* Maximum line length = 68 characters
	 0        1         2         3         4         5         6       6
	 12345678901234567890123456789012345678901234567890123456789012345678
	 */
	// UIH_Channel
	"Select which channel is being edited.",
	// UIH_OP_LeftRight
	"Toggle output of this operator on left/right stereo channel.",
	// UIH_OP_OutLevel
	"Level of direct output from this operator.\n"
	"This amount of signal is routed to the mix.\n"
	"Range: 0-7 (-6 dB steps, 0 = -Inf dB)",
	// UIH_OP_Attenuation
	"Attenuate the overall level of this operator,\n"
	"both for output and modulation.\n"
	"Range: 0-63 (-0.75 dB steps)",
	// UIH_OP_Scale
	"Scale the operator's level depending on pitch.\n"
	"Range: 0-3",
	// UIH_OP_FeedbackLevel
	"Modulate the first operator by feedback from the channel output.\n"
	"Range: 0-7 (-6 dB steps, 0 = -Inf dB)",
	// UIH_OP_ModLevel
	"Modulate the operator by the previous operator in the channel.\n"
	"Range: 0-7 (-6 dB steps, 0 = -Inf dB)",
	// UIH_OP_EnvMode
	"Envelope mode T one-shot triggers each note.\n"
	"Mode H holds each note until key-off.",
	// UIH_OP_EnvAtk
	"Attack speed, how fast the operator signal rises to maximum level\n"
	"after key-on.\n"
	"Range: 0-15 (must be >0 to produce signal)",
	// UIH_OP_EnvDec
	"Decay speed, how fast the operator signal decays to the sustain\n"
	"level after reaching maximum.\n"
	"Range: 0-15",
	// UIH_OP_EnvSus
	"Sustain attenuation level, higher values dampens the sustain level.\n"
	"Range: 0-15 (-3 dB steps, 15 = -93 dB)",
	// UIH_OP_EnvRel
	"Release speed, how fast the operator stops playing after key-off.\n"
	"Range: 0-15",
	// UIH_OP_EnvScale
	"Scale the envelope speed depending on the pitch.",
	// UIH_OP_SoundWave
	"Waveform for the operator.\n"
	"0: Sine; 1: Half rectified sine; 2: Full rectified sine\n"
	"3: Pulsing rectified sine; 4: Double rate half rectified sine\n"
	"5: Pulsing double rate rectified sine; 6: Square; 7: Sawtooth",
	// UIH_OP_SoundDrum
	"Drum noise for the operator, only has effect on the last operator.\n"
	"The sound is affected by the settings of the previous operator too.\n"
	"0: None; 1: Snare; 2: Hi-hat; 3: Cymbal",
	// UIH_OP_FxTrm
	"Tremolo level for the operator.\n"
	"Range: 0-2",
	// UIH_OP_FxVib
	"Vibrato level for the operator.\n"
	"Range: 0-2",
	// UIH_OP_FreqMode
	"Select frequency for the operator:\n"
	"Key is relative to the triggered key.\n"
	"Fixed uses the block/mul/fnum values directly.",
	// UIH_OP_FreqBlock
	"Octave shift for the operator pitch.",
	// UIH_OP_FreqMult
	"Multiplier for the operator pitch.\n"
	"Keys input uses multiplier 4 for all octaves, except the lowest = 2."
	"Special absolute values: 0 => 1/2, 11 => 10, 13 => 12, 14 => 15",
	// UIH_OP_FreqNum
	"Frequency number shift.\n"
	"Absolute range: 0-1023",
};

