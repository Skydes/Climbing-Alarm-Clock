 /***************************************************************************************
 *
 * Copyright (c) 2017, Paul-Edouard Sarlin
 * All rights reserved.
 *
 * Title:       Climbing Alarm Clock Project
 * File:        main.ino
 * Date:        2017-01-09
 * Author:      Paul-Edouard Sarlin
 * Website:     https://github.com/Skydes/Climbing-Alarm-Clock
 *
 ***************************************************************************************/

#include <prismino.h>
#include <Wire.h>
#include "U8glib.h"
#include <RTClib.h>
#include "Module_Buzzer.h"
#include "Module_Motor.h"
#include "Module_GUI.h"
#include "Module_RTC.h"
#include "control.h"

// to do: save settings data into eeprom and read it at start

void setup() 
{
  OLED_setup();
  Mot_setup();
  RTC_setup();
  
  Serial.begin(9600); // debug
}

void loop() 
{
  if(redraw_required) 
  {
    OLED_refresh();
    redraw_required = false;
    Serial.println(F("Redraw asked")); // debug
  }

  if(global_state == S_RINGING)
  {
    disp_state = TIME_DISP;
    OLED_refresh();
    Buzzer_play();
    redraw_required = true;
  }
  else if(global_state == S_SNOOZED)
  {
    MOTOR_UP;
    mot_state = M_UP;
    Mot_check_pos();
    delay(20);
  }

  if(RTC_update_asked)
  {
    RTC_update_module();
    RTC_update_asked = false;
  }

  if(rolling_out)
  {
    MOTOR_DOWN;
    mot_state = M_DOWN;
    Mot_check_pos();
    delay(20);
  }
}

