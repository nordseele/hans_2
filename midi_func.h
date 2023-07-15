#ifndef MIDI_FUNC_H
#define MIDI_FUNC_H

#include <stdint.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include <stdarg.h>

#define MIDI_BUFFER_SIZE 3 // Set the buffer size to accommodate one MIDI message
#define IS_NOTE_IN_RANGE(note, start, end) ((note) >= (start) && (note) <= (end))
#define MAP_NOTE_TO_DEST(note, src_start, dest_start) ((note) - (src_start) + (dest_start))

extern uint8_t midi_dev_addr;
extern bool display_refresh;
extern char midi_monitor[256];

typedef enum {
    NOTE_OFF = 0x80,
    NOTE_ON = 0x90,
    CONTROL_CHANGE = 0xB0,
    PROGRAM_CHANGE = 0xC0,
    MONO_KEY_PRESSURE = 0xD0,
    PITCH_BEND = 0xE0,
    // Add more message types as needed
} midi_message_type_t;

typedef struct {
    midi_message_type_t operation;
    uint8_t channel;
    uint8_t param1;
    uint8_t param2;
} midi_message_t;

void set_midi_monitor(midi_message_t message);
void print_midi_message(char *prefix, midi_message_t message);
void parse_midi_command(uint8_t buf[]);
void route_midi_message(midi_message_t message);
void map_midi_note_with_value(midi_message_t message, int starting_port, int length, int starting_note, const char* module_name, const char* command_name, int command_value);
void map_midi_note_without_value(midi_message_t message, int starting_port, int length, int starting_note, const char* module_name, const char* command_name);
void map_midi_control_change(midi_message_t message, int starting_port, int length, int starting_control, const char* module_name, const char* command_name);

#endif

/*

Note Off (0x80)
param1: Note number (0 to 127) - The MIDI note to turn off.
param2: Note velocity (0 to 127) - Typically set to 0 to indicate a Note Off message.

Note On (0x90)
param1: Note number (0 to 127) - The MIDI note to play.
param2: Note velocity (0 to 127) - The velocity at which the note is played (0 indicates Note Off).

Polyphonic Aftertouch (0xA0)
param1: Note number (0 to 127) - The MIDI note affected by aftertouch.
param2: Pressure value (0 to 127) - The amount of aftertouch pressure applied to the note.

Control Change (0xB0)
param1: Control number (0 to 127) - The MIDI control change number.
param2: Control value (0 to 127) - The value to set for the specified control.

Program Change (0xC0)
param1: Program number (0 to 127) - The new program (patch) number to select.

Channel Aftertouch (0xD0)
param1: Pressure value (0 to 127) - The amount of aftertouch pressure applied to all notes on the channel.
param2: Not used (should be set to 0).

Pitch Bend (0xE0)
param1 and param2: Together they form a 14-bit value to specify the pitch bend amount. The exact interpretation of these values depends on the MIDI device.


*/

/*
    MIDI COMMANDS
    -------------------------------------------------------------------
    name                 status      param 1          param 2
    -------------------------------------------------------------------
    note off             0x80+C       key #            velocity
    note on              0x90+C       key #            velocity
    poly key pressure    0xA0+C       key #            pressure value
    control change       0xB0+C       control #        control value
    program change       0xC0+C       program #        --
    mono key pressure    0xD0+C       pressure value   --
    pitch bend           0xE0+C       range (LSB)      range (MSB)
    system               0xF0+C       manufacturer     model
    -------------------------------------------------------------------
    C is the channel number, from 0 to 15;
    -------------------------------------------------------------------
    source: http://ftp.ec.vanderbilt.edu/computermusic/musc216site/MIDI.Commands.html

    In this program the pitch bend range will be transmitter as 
    one single 8-bit number. So the end result is that MIDI commands 
    will be transmitted as 3 bytes, starting with the operation byte:

    buf[0] --> operation/channel
    buf[1] --> param1
    buf[2] --> param2        (param2 not transmitted on program change or key press)
*/


