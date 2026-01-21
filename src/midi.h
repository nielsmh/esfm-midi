/* <http://midi.teragonaudio.com/tech/midispec.htm> */
#ifndef MIDI_H
#define MIDI_H

enum MIDI_Command {
    MCMD_NOTEON,
    MCMD_NOTEOFF,
    MCMD_CONTROLLER,
    MCMD_PITCHBEND,
    MCMD_PROGRAM_CHANGE,
};

struct MIDI_Input_NoteOnOff {
    enum MIDI_Command command;
    unsigned char channel;
    unsigned char note;
    unsigned char velocity;
};

struct MIDI_Input_Controller {
    enum MIDI_Command command;
    unsigned char channel;
    unsigned char controller;
    unsigned char value;
};


struct MIDI_Input_ProgramChange {
    enum MIDI_Command command;
    unsigned char channel;
    unsigned char program;
};

struct MIDI_Input_Pitchbend {
    enum MIDI_Command command;
    unsigned char channel;
    int value;
};

typedef union {
    enum MIDI_Command command;
    struct MIDI_Input_NoteOnOff note_on_off;
    struct MIDI_Input_Controller controller;
    struct MIDI_Input_ProgramChange program_change;
    struct MIDI_Input_Pitchbend pitchbend;
} MIDI_Input;


/** Initialize MIDI input.
 *  Returns 0 if failed, 1 if success.
 */
int midi_init(void);
/** Shut down MIDI input */
void midi_close(void);
/** Check for and return any received MIDI input.
 *  Returns 1 if a message was received and returned in the passed-in buffer.
 *  Returns 0 if there was no supported message.
 */
int midi_handleinput(MIDI_Input *inputdata);

extern volatile int midi_in_activity;

#endif

