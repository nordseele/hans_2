#include "midi_func.h"
#include "helpers.h"


midi_message_t midi_buffer[MIDI_BUFFER_SIZE]; // Create a buffer to store MIDI messages
uint8_t buffer_index = 0; // Index to keep track of the buffer position

void print_midi_message(char* prefix, midi_message_t message) {
    char dis[256];
    sprintf(dis, "%s 0x%x %02u %03u %03u\n", prefix, message.operation, message.channel, message.param1, message.param2);
    printy(dis);
}

void parse_midi_command(uint8_t buf[]) {
    // Create a new MIDI message structure
    midi_message_t message = {
        .operation = (midi_message_type_t)(buf[0] & 0xF0), // Mask off lower four bits
        .channel = buf[0] & 0x0F, // Mask off upper four bits
        .param1 = buf[1],
        .param2 = buf[2]
    };
    route_midi_message(message);
}

void map_midi_note_with_value(midi_message_t message, int starting_port, int length, int starting_note, const char* module_name, const char* command_name, int command_value) {
    int event_range_start = starting_note;
    int event_range_end = starting_note + length - 1;
    if (message.param1 >= event_range_start && message.param1 <= event_range_end) {
        int index = message.param1 - event_range_start + starting_port;
        char packet_str[256];
        sprintf(packet_str, "/%s/%s/%d/%d", module_name, command_name, index, command_value);
        parseData(packet_str);
    }
}

void map_midi_note_without_value(midi_message_t message, int starting_port, int length, int starting_note, const char* module_name, const char* command_name) {
    int event_range_start = starting_note;
    int event_range_end = starting_note + length - 1;
    if (message.param1 >= event_range_start && message.param1 <= event_range_end) {
        int index = message.param1 - event_range_start + starting_port;
        char packet_str[256];
        sprintf(packet_str, "/%s/%s/%d/%d", module_name, command_name, index, message.operation == NOTE_ON ? 1 : 0);
        parseData(packet_str);
    }
}

void map_midi_control_change(midi_message_t message, int starting_port, int length, int starting_control, const char* module_name, const char* command_name) {
    int event_range_start = starting_control;
    int event_range_end = starting_control + length - 1;
    if (message.param1 >= event_range_start && message.param1 <= event_range_end) {
        int index = message.param1 - event_range_start + starting_port;
        int value = (message.param2 * 32767) / 127; // Map value from 0-127 to 0-32767
        char packet_str[256];
        sprintf(packet_str, "/%s/%s/%d/%d", module_name, command_name, index, value);
        parseData(packet_str);
    }
}

// MIDI Routing Function
// This function routes MIDI messages based on the channel and operation type.
// For each channel-specific operation, the appropriate map function is called
// to map the MIDI message to a specific parameter on a module (e.g., ER-301).
//
// Functions:
// 1. map_midi_note_with_value: Maps a MIDI note message to a specific port and value on the module.
//    Usage: map_midi_note_with_value(message, starting_port, length, starting_note, module_name, command_name, command_value);
//
// 2. map_midi_note_without_value: Maps a MIDI note message to a specific port on the module. 
//    Note On messages are mapped to a value of 1, and Note Off messages are mapped to a value of 0.
//    Usage: map_midi_note_without_value(message, starting_port, length, starting_note, module_name, command_name);
//
// 3. map_midi_control_change: Maps a MIDI control change message to a specific port and value on the module.
//    The control change value is mapped from the MIDI range (0-127) to the desired range (0-32767).
//    Usage: map_midi_control_change(message, starting_port, length, starting_control, module_name, command_name);
//
// Parameters:
// - message: The MIDI message to be routed.
// - starting_port: The port number to start mapping MIDI notes or control changes.
// - length: The number of ports to map.
// - starting_note: The MIDI note number where mapping should start (used in note mapping functions).
// - module_name: The name of the module to which the MIDI messages should be sent (e.g., "er301", "txo").
// - command_name: The command name for the module (e.g., "tr", "tr_pulse", "cv", etc.).
// - (map_midi_note_with_value) command_value: The value to map to the module.


void route_midi_message(midi_message_t message) {
    // Route based on channel
    switch (message.channel) {
        case 1:
            // Route messages for channel 1
            switch (message.operation) {
                case NOTE_OFF:
                    map_midi_note_with_value(message, 1, 12, 60, "er301","tr",0);
                    break;
                case NOTE_ON:
                    map_midi_note_with_value(message, 1, 12, 60, "er301","tr",1);
                    map_midi_note_without_value(message, 1, 12, 48, "er301", "tr_pulse");
                    break;
                case CONTROL_CHANGE:
                    map_midi_control_change(message, 1, 12, 7, "er301", "cv");
                    break;
                default:
                    // Handle other message types for channel 1
                    break;
            }
            break;
        case 2:
            // Route messages for channel 2
            // Add your logic here for channel 2 routing
            break;
        default:
            // Handle other channels
            break;
    }
}
