#ifndef  TIME_H
#define  TIME_H

#define PIT_OSC_FREQ 1193182//Hz
#define PIT_COMMAND_MODE_PORT        0x43//port for command register
#define PIT_CHANNEL0_DATA_PORT       0x40//port for channel 0 data register

#define PIT_CHANNEL    (0<<6)//channel 0
#define PIT_LOAD_LOHI    (3<<4)//channel 0
#define PIT_MODE3        (3<<1)

void time_init(void);
void exception_handler_time(void);
#endif