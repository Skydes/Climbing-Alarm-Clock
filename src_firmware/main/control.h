 /***************************************************************************************
 *
 * Copyright (c) 2017, Paul-Edouard Sarlin
 * All rights reserved.
 *
 * Title:       Climbing Alarm Clock Project
 * File:        control.h
 * Date:        2017-01-09
 * Author:      Paul-Edouard Sarlin
 * Website:     https://github.com/Skydes/Climbing-Alarm-Clock
 *
 ***************************************************************************************/

#include <Arduino.h>

#ifndef CONTROL_H
#define CONTROL_H

enum Global_state {S_WAITING, S_RINGING, S_SNOOZED};

extern volatile uint8_t global_state;


#endif
