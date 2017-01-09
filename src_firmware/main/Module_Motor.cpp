 /***************************************************************************************
 *
 * Copyright (c) 2017, Paul-Edouard Sarlin
 * All rights reserved.
 *
 * Title:       Climbing Alarm Clock Project
 * File:        Module_Motor.cpp
 * Date:        2017-01-09
 * Author:      Paul-Edouard Sarlin
 * Website:     https://github.com/Skydes/Climbing-Alarm-Clock
 *
 ***************************************************************************************/

#include "Module_Motor.h"

#define B_SN_READ ((PINB >> 1) & 1)
#define ENC1_ON   !(PINB & (1 << 2))
#define ENC2_ON   !(PINB & (1 << 3))

#define SNOOZE_STEPS  2736 // one wheel turn
#define LENGTH_P_TURN 8   // centimeters
#define LENGTH_TOT    70  // centimeters
#define LENGTH_MIN    1   // centimeters
#define MAX_POS       LENGTH_TOT/LENGTH_P_TURN*SNOOZE_STEPS

volatile int enc_position = 0; // total position
volatile int goal_position = 0;
uint8_t snooze_length = 15; // centimeters
volatile bool b_sn_state = false; // used to select interrupt triggers
extern volatile bool rolling_out = false;


enum {e0, e1, e2, e3};
volatile uint8_t enc_next_state = e0;

volatile uint8_t mot_state = M_STOPPED;


void Mot_setup()
{
  // Pin Change interrupts setup
  PCICR |= (1 << PCIE0); // enable PCINT0 interrupt vector
  PCMSK0 |= (1 << PCINT1) | (1 << PCINT2) | (1 << PCINT3);
  SREG |= (1 << 7);
  
  // DDRB &= 0b01110000; // PB7, 6 ,5, 4 in input mode
}

uint8_t Mot_get_sn_dist()
{
  return snooze_length;
}

uint8_t Mot_set_sn_dist(int8_t new_dist)
{
  if(new_dist < LENGTH_MIN)
    snooze_length = LENGTH_TOT - new_dist;
  else if(new_dist > LENGTH_TOT)
    snooze_length = new_dist - LENGTH_TOT;
  else
    snooze_length = new_dist;
}

void Mot_check_pos()
{
  if( ((mot_state == M_UP) && (enc_position > goal_position)) || ((mot_state == M_DOWN) && (enc_position < goal_position)) )
  {
    MOTOR_OFF;
    mot_state = M_STOPPED;
    rolling_out = false;
  }
}

void Mot_set_next_pos()
{
  if(!rolling_out)
  {
    goal_position = enc_position + (unsigned)snooze_length/LENGTH_P_TURN*SNOOZE_STEPS;
    if(goal_position > MAX_POS)
      goal_position = MAX_POS;
  }
  else
    goal_position = 0;
}

ISR(PCINT0_vect)
{
  if(B_SN_READ != b_sn_state)
  {
    b_sn_state = B_SN_READ;

    if( b_sn_state && (global_state == S_RINGING) )
    {
      global_state = S_SNOOZED;
      RTC_set_last_snooze();
      Mot_set_next_pos();
    }
  }
  else
  {
    switch(enc_next_state) 
    {
    case e0: 
      if (ENC1_ON) {enc_position++; enc_next_state = e1; }
      if (ENC2_ON) {enc_position--; enc_next_state = e3; }
      break;
    case e1: 
      if (ENC2_ON)  {enc_position++; enc_next_state = e2; }
      if (!ENC1_ON) {enc_position--; enc_next_state = e0; }
      break;
    case e2: 
      if (!ENC1_ON) {enc_position++; enc_next_state = e3; }
      if (!ENC2_ON) {enc_position--; enc_next_state = e1; }
      break;
    case e3:
      if (!ENC2_ON) {enc_position++; enc_next_state = e0; }
      if (ENC1_ON)  {enc_position--; enc_next_state = e2; }
      break; 
    }
  }
}
