#include <stdlib.h>
#include <malloc.h>
#include <conio.h>
#include <time.h>
#include <dos.h>

#include "midi.h"

/* <http://midi.teragonaudio.com/tech/mpu.htm> */

static unsigned MPU_PORT = 0x330;
static unsigned MPU_IRQ = 9;

#define MPU_STATUS_DATA_SET_READY  0x80 /* not ready for read */
#define MPU_STATUS_DATA_READ_READY 0x40 /* not ready for write */

static const unsigned MIDI_BUFSIZE = 512;
static char __far *midi_buffer = NULL;
static unsigned midi_buffer_read = 0;
static unsigned midi_buffer_write = 0;

int midi_in_activity;

static void (__interrupt __far *mpu_prev_intr)();

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

int midi_hasinput(void)
{
    return midi_buffer_read != midi_buffer_write;
}

unsigned char midi_read(void)
{
    unsigned char data = midi_buffer[midi_buffer_read];
    ++midi_buffer_read;
    if (midi_buffer_read == MIDI_BUFSIZE) midi_buffer_read = 0;
    return data;
}

