#ifndef II_H
#define II_H

#include <stdio.h>
#include <stdbool.h>

typedef struct {
    int command_number;
    char name[16];
    struct {
        char name[10];
        int number_of_bytes;
    } args[2];
    int arg_count;
} II_command;

typedef struct {
    char name[8];
    int max_ports;
    int number_of_cmd;
    int addresses[8];
    II_command *cmd_set;
} Module_info;


II_command er301_cmd_set[] = {
    {0x0, "tr", {{"port", 1}, {"state", 2}}, 2},
    {0x01, "tr_tog", {{"port", 1}}, 1},
    {0x5, "tr_pulse", {{"port", 1}}, 1},
    {0x32, "tr_time", {{"port", 1}, {"time", 2}}, 2},
    {0x6, "tr_pol", {{"port", 1}, {"polarity", 2}}, 2},
    {0x10, "cv", {{"port", 1}, {"volt", 2}}, 2},
    {0x12, "cv_slew", {{"port", 1}, {"time", 2}}, 2},
    {0x11, "cv_set", {{"value", 1}, {"volt", 2}}, 2},
    {0x15, "cv_offset", {{"offset", 1}, {"offset", 2}}, 2},
};

II_command txo_cmd_set[] = {
    {0x0 ,"tr", {{"port", 1}}, 1},
    {0x01 ,"tr_tog", {{"port", 1}}, 1},
    {0x5 ,"tr_pulse", {{"port", 1}}, 1},
    {0x32 ,"tr_time", {{"port", 1}, {"time", 2}}, 2},
    {0x6 ,"tr_pol", {{"port", 1}, {"polarity", 2}}, 2},
    {0x10 ,"cv", {{"port", 1}, {"volt", 2}}, 2},
    {0x12 ,"cv_slew", {{"port", 1}, {"time", 2}}, 2},
    {0x11 ,"cv_set", {{"port", 1}, {"volt", 2}}, 2},
    {0x15 ,"cv_offset", {{"port", 1}, {"offset", 2}}, 2},
    {0x3 ,"tr_time_s", {{"port", 1}, {"s", 2}}, 2},
    {0x4 ,"tr_time_m", {{"port", 1}, {"m", 2}}, 2},
    {0x7 ,"tr_pulse_div", {{"port", 1}, {"pulses", 2}}, 2},
    {0x8 ,"tr_m", {{"port", 1}, {"ms", 2}}, 2},
    {0x9 ,"tr_m_s", {{"port", 1}, {"s", 2}}, 2},
    {0x0a ,"tr_m_m", {{"port", 1}, {"m", 2}}, 2},
    {0x0b ,"tr_m_bpm", {{"port", 1}, {"bpm", 2}}, 2},
    {0x0c ,"tr_m_act", {{"port", 1}, {"state", 2}}, 2},
    {0x0d ,"tr_m_sync", {{"port", 1}, {"state", 2}}, 2},
    {0x0e ,"tr_m_width", {{"port", 1}, {"width", 2}}, 2},
    {0x0f ,"tr_m_count", {{"port", 1}, {"count", 2}}, 2},
    {0x17 ,"tr_m_mul", {{"port", 1}, {"mult", 2}}, 2},
    {0x16 ,"tr_m_mute", {{"port", 1}, {"state", 2}}, 2},
    {0x13 ,"cv_slew_s", {{"port", 1}, {"s", 2}}, 2},
    {0x14 ,"cv_slew_m", {{"port", 1}, {"m", 2}}, 2},
    {0x30 ,"cv_qt", {{"port", 1}, {"qt", 2}}, 2},
    {0x31 ,"cv_qt_set", {{"port", 1}, {"qt", 2}}, 2},
    {0x32 ,"cv_n", {{"port", 1}, {"note", 2}}, 2},
    {0x33 ,"cv_n_set", {{"port", 1}, {"note", 2}}, 2},
    {0x34 ,"cv_scale", {{"port", 1}, {"scale", 2}}, 2},
    {0x35 ,"cv_log", {{"port", 1}, {"log", 2}}, 2},
    {0x40 ,"osc", {{"port", 1}, {"volts", 2}}, 2},
    {0x41 ,"osc_set", {{"port", 1}, {"volts", 2}}, 2},
    {0x42 ,"osc_qt", {{"port", 1}, {"volts", 2}}, 2},
    {0x43 ,"osc_qt_set", {{"port", 1}, {"volts", 2}}, 2},
    {0x46 ,"osc_n", {{"port", 1}, {"volts", 2}}, 2},
    {0x47 ,"osc_n_set", {{"port", 1}, {"volts", 2}}, 2},
    {0x44 ,"osc_fq", {{"port", 1}, {"fq", 2}}, 2},
    {0x45 ,"osc_fq_set", {{"port", 1}, {"fq", 2}}, 2},
    {0x48 ,"osc_lfo", {{"port", 1}, {"fq", 2}}, 2},
    {0x49 ,"osc_lfo_set", {{"port", 1}, {"fq", 2}}, 2},
    {0x4a ,"osc_wave", {{"port", 1}, {"wave", 2}}, 2},
    {0x4b ,"osc_sync", {{"port", 1}, {"wave", 2}}, 2},
    {0x53 ,"osc_phase", {{"port", 1}, {"phase", 2}}, 2},
    {0x4c ,"osc_width", {{"port", 1}, {"width", 2}}, 2},
    {0x4d ,"osc_rect", {{"port", 1}, {"pol", 2}}, 2},
    {0x4f ,"osc_slew", {{"port", 1}, {"ms", 2}}, 2},
    {0x50 ,"osc_slew_s", {{"port", 1}, {"s", 2}}, 2},
    {0x51 ,"osc_slew_m", {{"port", 1}, {"m", 2}}, 2},
    {0x4e ,"osc_scale", {{"port", 1}, {"scale", 2}}, 2},
    {0x54 ,"osc_cyc", {{"port", 1}, {"ms", 2}}, 2},
    {0x55 ,"osc_cyc_s", {{"port", 1}, {"s", 2}}, 2},
    {0x56 ,"osc_cyc_m", {{"port", 1}, {"m", 2}}, 2},
    {0x57 ,"osc_cyc_set", {{"port", 1}, {"ms", 2}}, 2},
    {0x57 ,"osc_cyc_s_set", {{"port", 1}, {"s", 2}}, 2},
    {0x59 ,"osc_cyc_m_set", {{"port", 1}, {"m", 2}}, 2},
    {0x5a ,"osc_ctr", {{"port", 1}, {"ctr", 2}}, 2},
    {0x60 ,"env_act", {{"port", 1}, {"state", 2}}, 2},
    {0x61 ,"env_att", {{"port", 1}, {"ms", 2}}, 2},
    {0x62 ,"env_att_s", {{"port", 1}, {"s", 2}}, 2},
    {0x63 ,"env_att_m", {{"port", 1}, {"m", 2}}, 2},
    {0x64 ,"env_dec", {{"port", 1}, {"ms", 2}}, 2},
    {0x65 ,"env_dec_s", {{"port", 1}, {"s", 2}}, 2},
    {0x66 ,"env_dec_m", {{"port", 1}, {"m", 2}}, 2},
    {0x67 ,"env_trig", {{"port", 1}, {"state", 2}}, 2},
    {0x6b ,"env_eoc", {{"port", 1}, {"state", 2}}, 2},
    {0x6a ,"env_eor", {{"port", 1}, {"state", 2}}, 2},
    {0x6c ,"env_loop", {{"port", 1}, {"state", 2}}, 2},
    {0x6d ,"env", {{"port", 1}, {"state", 2}}, 2},
    {0x20 ,"kill", {{"port", 1}, }, 1},
    {0x22 ,"tr_init", {{"port", 1}}, 1},
    {0x23 ,"cv_init", {{"port", 1}}, 1},
    {0x6e ,"cv_calib", {{"port", 1}}, 1},
    {0x6f ,"cv_reset", {{"port", 1}}, 1},
    {0x24 ,"init", {{"unit", 1}}, 1},
};

Module_info txo_info = {"TXO", 64, 74, {0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67}, txo_cmd_set};
Module_info er301_info = {"ER-301", 400, 9, {0x31, 0x32, 0x33, 0x34}, er301_cmd_set};

#endif
