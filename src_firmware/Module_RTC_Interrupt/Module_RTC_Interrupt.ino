 /***************************************************************************************
 *
 * Copyright (c) 2017, Paul-Edouard Sarlin
 * All rights reserved.
 *
 * Title:       Climbing Alarm Clock Project
 * File:        Module_RTC_Interrupt.ino
 * Date:        2017-01-09
 * Author:      Paul-Edouard Sarlin
 * Website:     https://github.com/Skydes/Climbing-Alarm-Clock
 *
 * Description: Module for the interrupt-based RTC.
 *
 ***************************************************************************************/

#include <Wire.h>
#include <prismino.h>
#include <RTClib.h>

#define SQW_READ    ((PIND >> 4) & 1)
#define COMMUTE_LED (PORTC ^= (1 << 7))

RTC_DS1307 RTC;

volatile unsigned counter = 0;
volatile bool redraw_required = false;

void setup() 
{
  // ICP1 interrupt settings
  TCCR1B |= (1 << ICNC1) | (1 << ICES1);
  TIMSK1 |= (1 << ICIE1);
  sei();
  
  Wire.begin();
  RTC.begin();
  Serial.begin(9600); // debug
  pinMode(LED, OUTPUT); // debug
  COMMUTE_LED;

  RTC.setSqwOutSignal(RTC_DS1307::Frequency_1Hz);
}

void loop() 
{
  if(redraw_required)
  {
    Serial.println(counter);
    redraw_required = false;
  }
}

ISR(TIMER1_CAPT_vect)
{
  COMMUTE_LED;
  if(SQW_READ)
  {
    counter++;
    redraw_required = true;
  }
}

