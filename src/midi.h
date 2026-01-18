/* <http://midi.teragonaudio.com/tech/midispec.htm> */
#ifndef MIDI_H
#define MIDI_H


int midi_init(void);
void midi_close(void);
int midi_hasinput(void);
unsigned char midi_read(void);

extern int midi_in_activity;

#endif

