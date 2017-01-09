 /***************************************************************************************
 *
 * Copyright (c) 2017, Paul-Edouard Sarlin
 * All rights reserved.
 *
 * Title:       Climbing Alarm Clock Project
 * File:        Module_RTC.h
 * Date:        2017-01-09
 * Author:      Paul-Edouard Sarlin
 * Website:     https://github.com/Skydes/Climbing-Alarm-Clock
 *
 ***************************************************************************************/
 
 #include <Arduino.h>
#include <RTClib.h>
#include "control.h"
#include "Module_GUI.h"

#ifndef MODULE_RTC_H
#define MODULE_RTC_H

#define CLOCK     true
#define ALARM     false

void RTC_setup();

uint8_t RTC_get_hours(bool ck_al);
uint8_t RTC_get_minutes(bool ck_all);
uint8_t RTC_get_snooze_dur();

void RTC_set_hours(int8_t new_hours, bool ck_al);
void RTC_set_minutes(int8_t new_minutes, bool ck_al);
void RTC_set_snooze_dur(int8_t new_snooze_dur);

void RTC_update_module();
void RTC_set_last_snooze();

extern volatile bool alarm_activated;
extern volatile bool RTC_update_asked;

#endif
