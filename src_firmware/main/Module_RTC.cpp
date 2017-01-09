 /***************************************************************************************
 *
 * Copyright (c) 2017, Paul-Edouard Sarlin
 * All rights reserved.
 *
 * Title:       Climbing Alarm Clock Project
 * File:        Module_RTC.cpp
 * Date:        2017-01-09
 * Author:      Paul-Edouard Sarlin
 * Website:     https://github.com/Skydes/Climbing-Alarm-Clock
 *
 ***************************************************************************************/

#include <Wire.h>
#include "Module_RTC.h"

#define SQW_READ    ((PIND >> 4) & 1)

#define SNOOZE_DUR_MAX  30 // minutes
#define SNOOZE_DUR_MIN  1

RTC_DS1307 RTC;
DateTime last_snooze(0);

void update_local_time();
void update_RTC_time();
uint8_t check_hours(int8_t hours);
uint8_t check_minutes(int8_t minutes);

uint8_t clock_hours = 0, clock_minutes = 0, clock_seconds = 0;
uint8_t alarm_hours = 8, alarm_minutes = 0;
uint8_t snooze_duration = 5; // minutes

volatile bool alarm_activated = true;
volatile bool RTC_update_asked = false;

void RTC_setup()
{
 /* Using pin 4 (ICP1) for interrupts at 1Hz from the DS3231
 * --> activate the noise canceller: TCCR1B |= (1 << ICNC1);
 * --> trigger on rising edge: TCCR1B |= (1 << ICES1);
 * --> activate Input Capture interrupt vector: TIMSK1 |= (1 << ICIE1);
 * --> ISR: ISR(TIMER1_CAPT_vect)
 */
  TCCR1B |= (1 << ICNC1) | (1 << ICES1);
  TIMSK1 |= (1 << ICIE1);
  sei();

  Wire.begin();
  RTC.begin();
  RTC.setSqwOutSignal(RTC_DS1307::Frequency_1Hz);

  update_local_time();
}

uint8_t RTC_get_hours(bool ck_al)
{
  if(ck_al == CLOCK)
    return clock_hours; // check has to be performed in order to ensure the in-memory time is the correct one
  else
    return alarm_hours;
}

uint8_t RTC_get_minutes(bool ck_al)
{
  if(ck_al == CLOCK)
    return clock_minutes; // same here
  else
    return alarm_minutes;
}

uint8_t RTC_get_snooze_dur()
{
  return snooze_duration;
}

void RTC_set_hours(int8_t new_hours, bool ck_al)
{
  if(ck_al == CLOCK)
    clock_hours = check_hours(new_hours);
  else
    alarm_hours = check_hours(new_hours);
}

void RTC_set_minutes(int8_t new_minutes, bool ck_al)
{
  if(ck_al == CLOCK)
    clock_minutes = check_minutes(new_minutes);
  else
    alarm_minutes = check_minutes(new_minutes);
}

void RTC_set_snooze_dur(int8_t new_snooze_dur)
{
  if(new_snooze_dur < SNOOZE_DUR_MIN)
    snooze_duration = SNOOZE_DUR_MAX - new_snooze_dur;
  else if(new_snooze_dur > SNOOZE_DUR_MAX)
    snooze_duration = new_snooze_dur - SNOOZE_DUR_MAX;
  else
    snooze_duration = new_snooze_dur;
}

void RTC_update_module()
{
  DateTime now = RTC.now();
  RTC.adjust(DateTime(now.year(), now.month(), now.day(), clock_hours, clock_minutes, clock_seconds));
}

void update_local_time()
{
  DateTime now = RTC.now();
  clock_seconds = now.second();
  clock_minutes = now.minute();
  clock_hours = now.hour();
}

void RTC_set_last_snooze()
{
  last_snooze = DateTime(0, 0, 0, clock_hours, clock_minutes, 0);
}

uint8_t check_hours(int8_t hours)
{
  if(hours < 0)
    return 23;
  else if(hours > 23)
    return 0;
  else
    return hours;
}

uint8_t check_minutes(int8_t minutes)
{
  if(minutes < 0)
    return 60 + minutes;
  else if(minutes > 59)
    return minutes - 60;
  else
    return minutes;
}

ISR(TIMER1_CAPT_vect)
{
  if(SQW_READ)
  {
    clock_seconds++;
    if(clock_seconds >= 60)
    {
      clock_minutes++;
      clock_seconds -= 60;
      redraw_required = true;
      
      if(clock_minutes >= 60)
      {
        clock_hours++;
        clock_seconds -= 60;

        if(clock_hours >= 24)
        {
          clock_hours -= 24;
        }
      }
    }

    if( alarm_activated && (alarm_hours == clock_hours) && (alarm_minutes == clock_minutes) && (clock_seconds == 0) && (global_state == S_WAITING) )
      global_state = S_RINGING;
    if( (global_state == S_SNOOZED) && ( ( (clock_hours - last_snooze.hour())*(unsigned)60 + clock_minutes - last_snooze.minute()) > snooze_duration) )
      global_state = S_RINGING;
  }
}

