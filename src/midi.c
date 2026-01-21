#include <stdlib.h>
#include <malloc.h>
#include <conio.h>
#include <time.h>
#include <dos.h>

#include "midi.h"

/* <http://midi.teragonaudio.com/tech/mpu.htm> */

static unsigned MPU_PORT = 0x330;
static unsigned MPU_IRQ = 0x0D;

#define MPU_STATUS_DATA_SET_READY  0x80 /* not ready for read */
#define MPU_STATUS_DATA_READ_READY 0x40 /* not ready for write */

static const unsigned MIDI_BUFSIZE = 512;
static char __far *midi_buffer = NULL;
static volatile unsigned midi_buffer_read = 0;
static volatile unsigned midi_buffer_write = 0;

int volatile midi_in_activity;

static void (__interrupt __far *mpu_prev_intr)();


struct {
    /** Status byte last seen */
    unsigned char status_byte;
    /** Message data currently being received */
    unsigned char data[2];
    /** Reading position for message being received:
     *  0 = received status, waiting data[0]
     *  1 = received status and data[0], waiting data[1]
     */
    unsigned char pos;
} midi_inputstatus = { 0 };


static inline unsigned char mpu_busy_read(void)
{
    return inp(MPU_PORT + 1) & MPU_STATUS_DATA_SET_READY;
}

static inline unsigned char mpu_read(void)
{
    return inp(MPU_PORT);
}

static inline unsigned char mpu_busy_write(void)
{
    return inp(MPU_PORT + 1) & MPU_STATUS_DATA_READ_READY;
}

static inline void mpu_write(unsigned char data)
{
    outp(MPU_PORT, data);
}

static void __interrupt __far mpu_interrupt_handler(void)
{
    unsigned char data;

    /* Read until the status port reports nothing to read */
    while (!mpu_busy_read()) {
        data = mpu_read();
        midi_buffer[midi_buffer_write] = data;
        ++midi_buffer_write;
        if (midi_buffer_write == MIDI_BUFSIZE) midi_buffer_write = 0;
        ++midi_in_activity;
    }
}

int midi_init(void)
{
    clock_t timeout_time;
    int timedout = 0;

    midi_buffer = malloc(MIDI_BUFSIZE);
    midi_in_activity = 0;

    /* Reset the MPU to UART mode */
    timeout_time = clock() + CLOCKS_PER_SEC;
    while (mpu_busy_write()) if (clock() >= timeout_time) { timedout = 1; break; }
    if (timedout) return 0;
    outp(MPU_PORT + 1, 0x3F);

    /* Set interrupt handler */
    mpu_prev_intr = _dos_getvect(MPU_IRQ);
    _dos_setvect(MPU_IRQ, mpu_interrupt_handler);

    return 1;
}

void midi_close(void)
{
    /* Reset interrupt handler */
    _dos_setvect(MPU_IRQ, mpu_prev_intr);

    free(midi_buffer);
}

/** Get number of bytes available in MIDI input buffer */
static inline int midi_numinput(void)
{
    int w = midi_buffer_write;
    /* If the read index is past the write index in the ring buffer,
     * the write has wrapped around, so adjust by buffer length */
    if (midi_buffer_read > w) w += MIDI_BUFSIZE;
    return w - midi_buffer_read;
}

/** Read a byte of MIDI input from offset ofs */
static inline unsigned char midi_getinput(int ofs)
{
    int r = midi_buffer_read + ofs;
    if (ofs >= MIDI_BUFSIZE) ofs -= MIDI_BUFSIZE;
    return midi_buffer[r];
}

/** Move the MIDI input read pointer ahead by count */
static inline void midi_consumeinput(int count)
{
    midi_buffer_read += count;
    while (midi_buffer_read >= MIDI_BUFSIZE) midi_buffer_read -= MIDI_BUFSIZE;
}

int midi_handleinput(MIDI_Input *inputdata)
{
    int available;
    unsigned char data, channel;

    /* Read until the status port reports nothing to read */
#if 0
    while (!mpu_busy_read()) {
        midi_buffer[midi_buffer_write] = mpu_read();
        ++midi_buffer_write;
        if (midi_buffer_write == MIDI_BUFSIZE) midi_buffer_write = 0;
        ++midi_in_activity;
    }
#endif

    /* Get amount of data available, and do early return check */
    available = midi_numinput();
    if (available == 0) return 0;

    /* Get some data */
    data = midi_getinput(0);
    midi_consumeinput(1);

    /* Check if it's a status byte */
    if (data & 0x80) {
        /* Check for system realtime messages (ignore those, even Reset) */
        if (data >= 0xF8) return 0;
        /* Otherwise store the status byte to handle in next iteration */
        midi_inputstatus.status_byte = data;
        midi_inputstatus.pos = 0;
        return 0;
    }

    /* Must be a data byte then, process per status */
    channel = midi_inputstatus.status_byte & 0x0F;
    switch (midi_inputstatus.status_byte >> 4) {
        case 0x8: /* Note off */
        case 0x9: /* Note on */
            /* Append data */
            midi_inputstatus.data[midi_inputstatus.pos++] = data;
            /* Check if message was completed */
            if (midi_inputstatus.pos == 2) {
                midi_inputstatus.pos = 0;
                inputdata->note_on_off.channel = channel;
                inputdata->note_on_off.note = midi_inputstatus.data[0];
                inputdata->note_on_off.velocity = midi_inputstatus.data[1];
                /* Velocity zero, or bit 4 not set, means it's a note off */
                if (inputdata->note_on_off.velocity == 0 || (midi_inputstatus.status_byte & 0x10) == 0) {
                    inputdata->note_on_off.command = MCMD_NOTEOFF;
                } else {
                    inputdata->note_on_off.command = MCMD_NOTEON;
                }
                return 1;
            } else {
                return 0;
            }

        case 0xA: /* Aftertouch */
            /* Message is ignored, just count bytes */
            midi_inputstatus.data[midi_inputstatus.pos++] = data;
            if (midi_inputstatus.pos == 2) midi_inputstatus.pos = 0;
            return 0;

        case 0xB: /* Controller */
            /* Append data */
            midi_inputstatus.data[midi_inputstatus.pos++] = data;
            /* Check if message was completed */
            if (midi_inputstatus.pos == 2) {
                midi_inputstatus.pos = 0;
                inputdata->controller.command = MCMD_CONTROLLER;
                inputdata->controller.channel = channel;
                inputdata->controller.controller = midi_inputstatus.data[0];
                inputdata->controller.value = midi_inputstatus.data[1];
                return 1;
            } else {
                return 0;
            }

        case 0xC: /* Program */
            inputdata->program_change.channel = channel;
            inputdata->program_change.program = data;
            inputdata->program_change.command = MCMD_PROGRAM_CHANGE;
            return 1;

        case 0xD: /* Pressure */
            /* Message is ignored, just ignore bytes */
            return 0;

        case 0xE: /* Pitch */
            /* Append data */
            midi_inputstatus.data[midi_inputstatus.pos++] = data;
            /* Check if message was completed */
            if (midi_inputstatus.pos == 2) {
                midi_inputstatus.pos = 0;
                inputdata->pitchbend.command = MCMD_PITCHBEND;
                inputdata->pitchbend.channel = channel;
                inputdata->pitchbend.value = ((int)midi_inputstatus.data[0] | ((int)midi_inputstatus.data[1] << 7)) - 0x2000;
                return 1;
            } else {
                return 0;
            }

        case 0xF: /* System */
            switch (channel) {
                case 0x0: /* System exclusive */
                case 0x7: /* System exclusive end */
                    /* Ignore, for now */
                    return 0;

                default:
                    /* Pretend they don't exist */
                    return 0;
            }

        default:
            /* ??? */
            return 0;
    }
}

