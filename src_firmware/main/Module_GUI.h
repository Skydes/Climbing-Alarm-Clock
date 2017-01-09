 /***************************************************************************************
 *
 * Copyright (c) 2017, Paul-Edouard Sarlin
 * All rights reserved.
 *
 * Title:       Climbing Alarm Clock Project
 * File:        Module_GUI.h
 * Date:        2017-01-09
 * Author:      Paul-Edouard Sarlin
 * Website:     https://github.com/Skydes/Climbing-Alarm-Clock
 *
 ***************************************************************************************/

#include <Arduino.h>
#include "U8glib.h"

#ifndef MODULE_GUI_H
#define MODULE_GUI_H

enum Disp_state {TIME_DISP, SETTINGS_MENU, CLOCK_TIME, ALARM_TIME, SNOOZE_SETTINGS, SNOOZE_DURATION, LIFTING_DISTANCE, RINGTONES};

extern U8GLIB_SH1106_128X64 u8g;
extern volatile uint8_t disp_state;
extern volatile bool redraw_required;

void OLED_setup();
void OLED_refresh();
void OLED_draw();

#endif
