 /***************************************************************************************
 *
 * Copyright (c) 2017, Paul-Edouard Sarlin
 * All rights reserved.
 *
 * Title:       Climbing Alarm Clock Project
 * File:        Module_Buzzer.h
 * Date:        2017-01-09
 * Author:      Paul-Edouard Sarlin
 * Website:     https://github.com/Skydes/Climbing-Alarm-Clock
 *
 ***************************************************************************************/

#include <Arduino.h>
#include <prismino.h>
#include "control.h"

#ifndef MODULE_BUZZER_H
#define MODULE_BUZZER_H

#define SONGS_NUMBER  3

void Buzzer_set_ringtone(uint8_t new_ringtone);
bool Buzzer_play();

#endif
