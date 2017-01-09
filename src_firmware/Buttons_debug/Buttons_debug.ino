 /***************************************************************************************
 *
 * Copyright (c) 2017, Paul-Edouard Sarlin
 * All rights reserved.
 *
 * Title:       Climbing Alarm Clock Project
 * File:        Button_debug.ino
 * Date:        2017-01-09
 * Author:      Paul-Edouard Sarlin
 * Website:     https://github.com/Skydes/Climbing-Alarm-Clock
 *
 * Description: Test for the filtered buttons.
 *
 ***************************************************************************************/

#include <prismino.h>

#define B_OK    1 // INT3 interrupt

void setup() 
{
  // set button pinmodes
  pinMode(B_OK, INPUT_PULLUP);

  // set interrupt vectors
  attachInterrupt(3, b_ok_isr, CHANGE);

  pinMode(LED, OUTPUT);
}

void loop() { }

void b_ok_isr()
{
  digitalWrite(LED, digitalRead(B_OK));
}

