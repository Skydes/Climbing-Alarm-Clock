 /***************************************************************************************
 *
 * Copyright (c) 2017, Paul-Edouard Sarlin
 * All rights reserved.
 *
 * Title:       Climbing Alarm Clock Project
 * File:        Module_Motor.h
 * Date:        2017-01-09
 * Author:      Paul-Edouard Sarlin
 * Website:     https://github.com/Skydes/Climbing-Alarm-Clock
 *
 ***************************************************************************************/
 
#include <prismino.h>
#include "control.h"
#include "Module_RTC.h"

#ifndef MODULE_MOTOR_H
#define MODULE_MOTOR_H

#define MOTOR_UP      setSpeed(50, 0)
#define MOTOR_DOWN    setSpeed(-60, 0)
#define MOTOR_OFF     setSpeed(0, 0)

uint8_t Mot_get_sn_dist();
uint8_t Mot_set_sn_dist(int8_t new_dist);

void Mot_setup();
void Mot_check_pos();
void Mot_set_next_pos();

enum Mot_state {M_STOPPED, M_UP, M_DOWN};

extern volatile uint8_t mot_state;
extern volatile bool snooze_pressed;
extern volatile bool rolling_out;

#endif
