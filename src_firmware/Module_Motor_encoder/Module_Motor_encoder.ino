 /***************************************************************************************
 *
 * Copyright (c) 2017, Paul-Edouard Sarlin
 * All rights reserved.
 *
 * Title:       Climbing Alarm Clock Project
 * File:        Module_Motor_encoder.ino
 * Date:        2017-01-09
 * Author:      Paul-Edouard Sarlin
 * Website:     https://github.com/Skydes/Climbing-Alarm-Clock
 *
 * Description: Independant module for motor control.
 *              -> SCK (PCINT1, PB1) pin for Snooze button interrupt
 *              -> MOSI and MISO (PCINT2&3, PB2&3) for encoders
 *
 ***************************************************************************************/

#include <prismino.h>

#define B_SN_READ ((PINB >> 1) & 1)
#define ENC1_ON   !(PINB & (1 << 2))
#define ENC2_ON   !(PINB & (1 << 3))

#define SNOOZE_STEPS  2736 // one wheel turn
#define MOTOR_ON      setSpeed(60, 0)
#define MOTOR_OFF     setSpeed(0, 0)

volatile int enc_position = 0;
enum {e0, e1, e2, e3};
volatile uint8_t enc_next_state = e0;

enum Enc_state {WAITING, RUNNING};
volatile uint8_t enc_state = WAITING;

volatile bool b_sn_state = false;
volatile bool snooze_flag = false;

void setup() 
{
  pcinterrupt_setup();  // Pin Change interrupts setup
  
  DDRB &= 0b01110000; // PB7, 6 ,5, 4 in input mode
  
  // debug
  Serial.begin(9600);
  pinMode(LED, INPUT);
}

void loop() 
{
  if(snooze_flag && (enc_state == WAITING))
  {
    snooze_flag = false;
    enc_state = RUNNING;
    enc_position = 0;
    MOTOR_ON;
    Serial.println("Snooze triggered, motor launched");
  }

  if((enc_position > SNOOZE_STEPS) && (enc_state == RUNNING))
  {
    MOTOR_OFF;
    enc_state = WAITING;
    snooze_flag = false;
    Serial.println("Goal reached, motor stopped");
  }

  if(enc_state == RUNNING)
  {
  Serial.println(enc_position);
  delay(200);
  }

}

void pcinterrupt_setup()
{
  PCICR |= (1 << PCIE0); // enable PCINT0 interrupt vector
  PCMSK0 |= (1 << PCINT1) | (1 << PCINT2) | (1 << PCINT3);
  SREG |= (1 << 7);
}

ISR(PCINT0_vect)
{
  if(B_SN_READ != b_sn_state)
  {
    b_sn_state = B_SN_READ;
    digitalWrite(LED, b_sn_state); // debug

    if(b_sn_state)
      snooze_flag = true;
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
