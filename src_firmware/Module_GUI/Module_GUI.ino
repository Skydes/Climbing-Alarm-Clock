 /***************************************************************************************
 *
 * Copyright (c) 2017, Paul-Edouard Sarlin
 * All rights reserved.
 *
 * Title:       Climbing Alarm Clock Project
 * File:        Module_GUI.ino
 * Date:        2017-01-09
 * Author:      Paul-Edouard Sarlin
 * Website:     https://github.com/Skydes/Climbing-Alarm-Clock
 *
 * Description: Module for the OLED-based GUI.
 *
 ***************************************************************************************/
 
#include "U8glib.h"
#include <prismino.h>

#define SCK_PIN   A2
#define MOSI_PIN  A3 // SDA pin on OLED
#define A0_PIN    A4 // DC pin on OLED
#define CS_PIN    A5

#define B_PREV_PIN  0 // INT2 interrupt
#define B_OK_PIN    1 // INT3 interrupt
#define B_NEXT_PIN  7 // INT6 interrupt

#define B_PREV_READ  ((PIND >> 2) & 1) // PortD 2
#define B_OK_READ    ((PIND >> 3) & 1) // PortD 3
#define B_NEXT_READ  ((PINE >> 6) & 1) // PortE 6

#define BOUNCE_TOL  20 // 20ms

#define SCREEN_W  128
#define SCREEN_H  64
#define MAIN_MENU_ITEMS 5

U8GLIB_SH1106_128X64 u8g(SCK_PIN, MOSI_PIN, CS_PIN, A0_PIN);

enum Disp_state {TIME_DISP, SETTINGS_MENU};
volatile enum Disp_state disp_state = TIME_DISP;
volatile bool menu_redraw_required = true;
volatile uint8_t main_menu_current = 0;

char* displayed_time = "20:16";
const char *menu_strings[MAIN_MENU_ITEMS] = { "Change clock time", "Change alarm time", "Change ringtone", "Change snooze time", "<-- Back" };

volatile unsigned b_prev_last = 0;
volatile unsigned b_ok_last = 0;
volatile unsigned b_next_last = 0;

// debug variables
unsigned duration = 0;
unsigned count = 0;


void setup()
{
  // set button pinmodes
  pinMode(B_PREV_PIN, INPUT);
  pinMode(B_OK_PIN, INPUT);
  pinMode(B_NEXT_PIN, INPUT);

  // set interrupt vectors
  attachInterrupt(2, b_prev_isr, CHANGE);
  attachInterrupt(3, b_ok_isr, CHANGE);
  attachInterrupt(4, b_next_isr, CHANGE);

  // debug
  pinMode(LED, OUTPUT);
  Serial.begin(9600);
}

void draw(void) 
{
  if(disp_state == TIME_DISP)
  {
    u8g_uint_t w;
    
    u8g.setFontPosCenter();
    u8g.setFont(u8g_font_fub25n);
    w = u8g.getStrWidth(displayed_time);
        
    u8g.drawStr( int((SCREEN_W - w)/2), SCREEN_H/2, displayed_time);
  }
  else if(disp_state == SETTINGS_MENU)
  {
    uint8_t i, h;
    u8g_uint_t w, d;

    u8g.setFont(u8g_font_6x13);
    u8g.setFontRefHeightText();
    u8g.setFontPosTop();
  
    h = u8g.getFontAscent()-u8g.getFontDescent();
    w = u8g.getWidth();

    for( i = 0; i < MAIN_MENU_ITEMS; i++ ) 
    {
      d = (w - u8g.getStrWidth(menu_strings[i]))/2;
      u8g.setDefaultForegroundColor();
      if ( i == main_menu_current ) 
      {
        u8g.drawBox(0, i*h+1, w, h);
        u8g.setDefaultBackgroundColor();
      }
      u8g.drawStr(d, i*h, menu_strings[i]);
    }
  }
}

void b_prev_isr()
{
  bool value = B_PREV_READ;
  if( ((millis() - b_prev_last) > 10) && !menu_redraw_required)
  {
    b_prev_last = millis();
    
    if(!value)
    {
      if ( main_menu_current == 0 )
        main_menu_current = MAIN_MENU_ITEMS;
      main_menu_current--;
      menu_redraw_required = true;
    }
  }
}

void b_ok_isr()
{
  bool value = B_OK_READ;
  if( ((millis() - b_ok_last) > 10)  && !menu_redraw_required)
  {
    b_ok_last = millis();

    digitalWrite(LED, value);
  
    if(!value)
    {
      if(disp_state == TIME_DISP)
        disp_state = SETTINGS_MENU;
      else if(disp_state == SETTINGS_MENU)
        disp_state = TIME_DISP;
      menu_redraw_required = true;
    }
  }
}

void b_next_isr()
{
  bool value = B_NEXT_READ;
  if( ((millis() - b_next_last) > 10) && !menu_redraw_required)
  {
    b_next_last = millis();
  
    if(!value)
    {
      main_menu_current++;
      if ( main_menu_current >= MAIN_MENU_ITEMS )
        main_menu_current = 0;
      menu_redraw_required = true;
    }
  }
}

void loop() 
{  
  if (  menu_redraw_required == true ) 
  {
    u8g.firstPage();
    do  {
      draw();
    } while( u8g.nextPage() );
    menu_redraw_required = false;
    Serial.println(++count); // debug
  }
}

