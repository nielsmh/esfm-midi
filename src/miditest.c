#include <stdio.h>
#include <time.h>
#include "midi.h"

int main()
{
    clock_t cl;
    int count = 20;
    MIDI_Input inputdata;

    if (midi_init()) {
        printf("hello midi\n");

        while (count > 0) {
            cl = clock() + CLOCKS_PER_SEC;
            while (cl > clock()) {
                if (midi_handleinput(&inputdata)) {
                    switch (inputdata.command) {
                        case MCMD_NOTEON:
                            printf("ON@%d:%d=%d ", inputdata.note_on_off.channel, inputdata.note_on_off.note, inputdata.note_on_off.velocity);
                            break;
                        case MCMD_NOTEOFF:
                            printf("OFF@%d:%d=%d ", inputdata.note_on_off.channel, inputdata.note_on_off.note, inputdata.note_on_off.velocity);
                            break;
                        case MCMD_CONTROLLER:
                            printf("CTL@%d:%d=%d ", inputdata.controller.channel, inputdata.controller.controller, inputdata.controller.value);
                            break;
                        case MCMD_PROGRAM_CHANGE:
                            printf("PGM@%d:%d ", inputdata.program_change.channel, inputdata.program_change.program);
                            break;
                        case MCMD_PITCHBEND:
                            printf("PWH@%d:%d ", inputdata.pitchbend.channel, inputdata.pitchbend.value);
                            break;
                        default:
                            printf("c=%d ", inputdata.command);
                            break;
                    }
                    fflush(stdout);
                }
            }
            printf(". ");
            fflush(stdout);
            count--;
        }

        midi_close();
        printf("\ngoodbye midi\n");

        return 0;
    }

    return 1;
}

