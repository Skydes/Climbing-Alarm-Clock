 /***************************************************************************************
 *
 * Copyright (c) 2017, Paul-Edouard Sarlin
 * All rights reserved.
 *
 * Title:       Climbing Alarm Clock Project
 * File:        Module_GUI.cpp
 * Date:        2017-01-09
 * Author:      Paul-Edouard Sarlin
 * Website:     https://github.com/Skydes/Climbing-Alarm-Clock
 *
 ***************************************************************************************/

#include "Module_GUI.h"
#include "Module_RTC.h"
#include "Module_Motor.h"
#include "Module_Buzzer.h"
#include "control.h"

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

#define BOUNCE_TOL  20 // milliseconds

#define SCREEN_W  128
#define SCREEN_H  64
#define SETTINGS_MENU_ITEMS 6
#define SNOOZE_MENU_ITEMS   4

U8GLIB_SH1106_128X64 u8g(SCK_PIN, MOSI_PIN, CS_PIN, A0_PIN);
volatile uint8_t disp_state = TIME_DISP;
volatile bool redraw_required = true;
volatile uint8_t settings_menu_current = 0;
const char *settings_menu_strings[SETTINGS_MENU_ITEMS] = { "Change alarm time", "Change clock time", "Snooze settings", "Change ringtone", "<--- Back", ""}; // Last one is a ghost item
const char *snooze_menu_strings[SNOOZE_MENU_ITEMS] = {"Snooze duration", "Max lifting distance", "<--- Back", ""};
const char *ringtones_menu_strings[SONGS_NUMBER+2] = {"-> Mario", "-> StarWars Main", "-> StarWars Catina", "<--- Back", ""};
const uint8_t bell_bitmap[] PROGMEM = {0x00, 0x00, 0x00,0x00, 0x00, 0x60, 0x00, 0x60, 0x01, 0xf8, 0x01, 0xf8, 0x01, 0xf8, 0x03, 0xfc, 0x07, 0xfe, 0x07, 0xfe, 0x00, 0x60, 0x00, 0x60, 0x00, 0x00 };

volatile uint8_t sub_menu_current = 0;

// software interrupts filtering
volatile unsigned b_prev_last = 0;
volatile unsigned b_ok_last = 0;
volatile unsigned b_next_last = 0;

void b_prev_isr();
void b_ok_isr();
void b_next_isr();
void time_to_string(char *displayed_time, bool ck_al);

void OLED_setup()
{
  // set button pinmodes
  pinMode(B_PREV_PIN, INPUT);
  pinMode(B_OK_PIN, INPUT);
  pinMode(B_NEXT_PIN, INPUT);

  // set interrupt vectors
  attachInterrupt(2, b_prev_isr, CHANGE);
  attachInterrupt(3, b_ok_isr, CHANGE);
  attachInterrupt(4, b_next_isr, CHANGE); // labelled as INT6 interrupt
}

void OLED_refresh()
{
  u8g.firstPage();
  do
  {
    OLED_draw();
  } while( u8g.nextPage() );
}

void OLED_draw() 
{
  switch(disp_state)
  {
    case TIME_DISP:
    {
      u8g_uint_t w;
      char displayed_time[6];
      time_to_string(displayed_time, CLOCK);
      
      u8g.setFont(u8g_font_fub25n);
      u8g.setFontPosCenter();
      w = u8g.getStrWidth(displayed_time);
      u8g.drawStr( int((SCREEN_W - w)/2), SCREEN_H/2, displayed_time);

      u8g.setFont(u8g_font_6x10);
      u8g.setFontRefHeightText();
      u8g.setFontPosTop();
      
      if(alarm_activated)
      {
        u8g.drawBitmapP( 2, SCREEN_H - 15, 2, 13, bell_bitmap);
        u8g.drawStr(20, SCREEN_H - 12, "Alarm ON");
      }
      
      switch(global_state)
      {
        case S_RINGING:
          u8g.drawStr(80, SCREEN_H - 12, "Ringing");
          break;
        case S_SNOOZED:
          u8g.drawStr(80, SCREEN_H - 12, "Snoozed");
          break;
        default:
          break;
      }

      break;
    }
    case SETTINGS_MENU:
    {
      uint8_t i, h, w, d;
  
      u8g.setFont(u8g_font_6x13);
      u8g.setFontRefHeightText();
      u8g.setFontPosTop();
    
      h = u8g.getFontAscent()-u8g.getFontDescent();
      w = SCREEN_W;
  
      for( i = 0; i < SETTINGS_MENU_ITEMS; i++ ) 
      {
        d = (w - u8g.getStrWidth(settings_menu_strings[i]))/2;
        u8g.setDefaultForegroundColor();
        if ( i == settings_menu_current ) 
        {
          u8g.drawBox(0, i*h+1, w, h);
          u8g.setDefaultBackgroundColor();
        }
        u8g.drawStr(d, i*h, settings_menu_strings[i]);
      }
      break;
    }
    case CLOCK_TIME:
    case ALARM_TIME:
    {
      uint8_t d, wprev, wchar, i, j;
      char displayed_time[6];
      
      time_to_string(displayed_time, (disp_state == CLOCK_TIME) ? CLOCK : ALARM);
      
      u8g.setFont(u8g_font_fub25n);
      u8g.setFontPosCenter();
      d = (SCREEN_W - u8g.getStrWidth(displayed_time))/2;

      i = (sub_menu_current<2) ? sub_menu_current : (sub_menu_current+1);
      char temp_string[6];
      for(j = 0; j<i; j++)
      {
        temp_string[j] = displayed_time[j];
      }
      temp_string[i] = '\0';
      wprev = u8g.getStrWidth(temp_string);
      temp_string[0] = displayed_time[i];
      temp_string[1] = '\0';
      wchar = u8g.getStrWidth(temp_string);

      u8g.drawFrame(d + wprev + 1, 0, wchar, SCREEN_H);
      u8g.drawTriangle(d + wprev + 6, 5, d + wprev + wchar - 4, 5, d + wprev + wchar/2, 10);
      u8g.drawStr( d, SCREEN_H/2, displayed_time);
      
      break;
    }
    case SNOOZE_SETTINGS:
    {
      uint8_t i, h, d;
  
      u8g.setFont(u8g_font_6x13);
      u8g.setFontRefHeightText();
      u8g.setFontPosTop();
    
      h = u8g.getFontAscent()-u8g.getFontDescent();
  
      for( i = 0; i < SNOOZE_MENU_ITEMS; i++ ) 
      {
        d = (SCREEN_W - u8g.getStrWidth(snooze_menu_strings[i]))/2;
        u8g.setDefaultForegroundColor();
        if ( i == sub_menu_current ) 
        {
          u8g.drawBox(0, i*h+1, SCREEN_W, h);
          u8g.setDefaultBackgroundColor();
        }
        u8g.drawStr(d, i*h, snooze_menu_strings[i]);
      }
      break;
    }
    case SNOOZE_DURATION:
    case LIFTING_DISTANCE:
    {
      bool dur_dist = (disp_state == SNOOZE_DURATION) ? true : false;
      uint8_t h1, h2, d2;
      uint8_t val = dur_dist ? RTC_get_snooze_dur() : Mot_get_sn_dist();
      char val_string[3];
      
      u8g.setFont(u8g_font_6x13);
      u8g.setFontRefHeightText();
      u8g.setFontPosTop();
      h1 = u8g.getFontAscent()-u8g.getFontDescent();

      if(dur_dist)
      {
        u8g.drawStr((SCREEN_W - u8g.getStrWidth("Snooze duration"))/2, 0, "Snooze duration");
        u8g.drawStr((SCREEN_W - u8g.getStrWidth("(minutes)"))/2, h1, "(minutes)");
      }
      else
      {
        u8g.drawStr((SCREEN_W - u8g.getStrWidth("Lifting distance"))/2, 0, "Lifting distance");
        u8g.drawStr((SCREEN_W - u8g.getStrWidth("(centimeters)"))/2, h1, "(centimeters)");
      }
     
      if(val < 10)
      {
        val_string[0] = '0';
        val_string[1] = (char)(val + 48);
      }
      else
      {
        val_string[0] = (char)( (int8_t)(val/10) + 48);
        val_string[1] = (char)(val - 10*(int8_t)(val/10) + 48);
      }
      val_string[2] = '\0';
      
      u8g.setFont(u8g_font_fub25n);
      u8g.setFontRefHeightText();
      u8g.setFontPosTop();
      
      h2 = u8g.getFontAscent()-u8g.getFontDescent();
      d2 = (SCREEN_W - u8g.getStrWidth(val_string))/2;
      u8g.drawStr(d2, 2*h1 + (SCREEN_H - 2*h1 - h2)/2, val_string);

      break;
    }
    case RINGTONES:
    {
      uint8_t i, h, d;
  
      u8g.setFont(u8g_font_6x13);
      u8g.setFontRefHeightText();
      u8g.setFontPosTop();
    
      h = u8g.getFontAscent()-u8g.getFontDescent();
  
      for( i = 0; i < SONGS_NUMBER+1; i++ ) 
      {
        d = (SCREEN_W - u8g.getStrWidth(ringtones_menu_strings[i]))/2;
        u8g.setDefaultForegroundColor();
        if ( i == sub_menu_current ) 
        {
          u8g.drawBox(0, i*h+1, SCREEN_W, h);
          u8g.setDefaultBackgroundColor();
        }
        u8g.drawStr(d, i*h, ringtones_menu_strings[i]);
      }
      break;
    }
    
    default:
      break;
  }
}

void b_prev_isr()
{
  bool value = B_PREV_READ;
  if( ((millis() - b_prev_last) > 10) && !redraw_required)
  {
    b_prev_last = millis();

    if(!value)
    {
      switch(disp_state)
      {
        case TIME_DISP:
          break;
          
        case SETTINGS_MENU:
          if ( settings_menu_current == 0 )
            settings_menu_current = SETTINGS_MENU_ITEMS-1;
          settings_menu_current--;
          redraw_required = true;
          break;

        case CLOCK_TIME:
        case ALARM_TIME:
        {
          bool ck_al = (disp_state == CLOCK_TIME) ? CLOCK : ALARM;
          switch(sub_menu_current)
          {
            case 0:
              RTC_set_hours(RTC_get_hours(ck_al) - 10, ck_al);
              break;
            case 1:
              RTC_set_hours(RTC_get_hours(ck_al) - 1, ck_al);
              break;
            case 2:
              RTC_set_minutes(RTC_get_minutes(ck_al) - 10, ck_al);
              break;
            case 3:
              RTC_set_minutes(RTC_get_minutes(ck_al) - 1, ck_al);
              break;
          }
          redraw_required = true;
          break;
        }
        
        case SNOOZE_SETTINGS:
          if( sub_menu_current == 0 )
            sub_menu_current = SNOOZE_MENU_ITEMS-1;
          sub_menu_current--;
          redraw_required = true;
          break;

        case SNOOZE_DURATION:
          RTC_set_snooze_dur(RTC_get_snooze_dur() - 1);
          redraw_required = true;
          break;

        case LIFTING_DISTANCE:
          Mot_set_sn_dist(Mot_get_sn_dist() - 1);
          redraw_required = true;
          break;

        case RINGTONES:
          if( sub_menu_current == 0 )
            sub_menu_current = SONGS_NUMBER+1;
          sub_menu_current--;
          redraw_required = true;
          break;
  
        default:
          break;
      }
    }
  }
}

void b_ok_isr()
{
  bool value = B_OK_READ;
  if( ((millis() - b_ok_last) > 10)  && !redraw_required)
  {
    b_ok_last = millis();
  
    if(!value)
    {
      switch(disp_state)
      {
        case TIME_DISP:
          settings_menu_current = 0;
          disp_state = SETTINGS_MENU;
          break;

        case SETTINGS_MENU:
          switch(settings_menu_current)
          {
            case 1:
              disp_state = CLOCK_TIME;
              sub_menu_current = 0;
              break;

            case 0:
              disp_state = ALARM_TIME;
              sub_menu_current = 0;
              break;

            case 2:
              disp_state = SNOOZE_SETTINGS;
              sub_menu_current = 0;
              break;

            case 3:
              disp_state = RINGTONES;
              sub_menu_current = 0;
              break;

            case 4:
              disp_state = TIME_DISP;
              settings_menu_current = 0;
              break;

            default:
              disp_state = TIME_DISP;
              settings_menu_current = 0;
              break;
          }
          break;

        case CLOCK_TIME:
        case ALARM_TIME:
          if(sub_menu_current < 3)
            sub_menu_current++;
          else
          {
            sub_menu_current = 0;
            if(disp_state == CLOCK_TIME)
              RTC_update_asked = true;
            disp_state = SETTINGS_MENU;
          }
          break;

        case SNOOZE_SETTINGS:
        {
          switch(sub_menu_current)
          {
            case 0:
              disp_state = SNOOZE_DURATION;
              break;
            case 1:
              disp_state = LIFTING_DISTANCE;
              break;

            case 2:
              disp_state = SETTINGS_MENU;
              sub_menu_current = 0;
              break;

            default: 
              break;
          }
          break;
        }

        case SNOOZE_DURATION:
        case LIFTING_DISTANCE:
          disp_state = SNOOZE_SETTINGS;
          break;

        case RINGTONES:
          if(sub_menu_current < SONGS_NUMBER)
            Buzzer_set_ringtone(sub_menu_current);
          disp_state = SETTINGS_MENU;
          break;

        default:
          break;
      }
      redraw_required = true;
    }
  }
}

void b_next_isr()
{
  bool value = B_NEXT_READ;
  if( ((millis() - b_next_last) > 10) && !redraw_required)
  {
    b_next_last = millis();
  
    if(!value)
    {
      switch(disp_state)
      {
        case TIME_DISP:
          alarm_activated = !alarm_activated;
          if( (global_state == S_RINGING) || (global_state == S_SNOOZED) )
          {
            rolling_out = true;
            Mot_set_next_pos();
          }
          global_state = S_WAITING;
          redraw_required = true;
          break;

        case SETTINGS_MENU:
          settings_menu_current++;
          if ( settings_menu_current >= SETTINGS_MENU_ITEMS-1 )
            settings_menu_current = 0;
          redraw_required = true;
          break;

        case CLOCK_TIME:
        case ALARM_TIME:
        {
          bool ck_al = (disp_state == CLOCK_TIME) ? CLOCK : ALARM;
          switch(sub_menu_current)
          {
            case 0:
              RTC_set_hours(RTC_get_hours(ck_al) + 10, ck_al);
              break;
            case 1:
              RTC_set_hours(RTC_get_hours(ck_al) + 1, ck_al);
              break;
            case 2:
              RTC_set_minutes(RTC_get_minutes(ck_al) + 10, ck_al);
              break;
            case 3:
              RTC_set_minutes(RTC_get_minutes(ck_al) + 1, ck_al);
              break;
          }
          redraw_required = true;
          break;
        }

        case SNOOZE_SETTINGS:
          sub_menu_current++;
          if( sub_menu_current >= SNOOZE_MENU_ITEMS-1 )
            sub_menu_current = 0;
          redraw_required = true;
          break;

        case SNOOZE_DURATION:
          RTC_set_snooze_dur(RTC_get_snooze_dur() + 1);
          redraw_required = true;
          break;

        case LIFTING_DISTANCE:
          Mot_set_sn_dist(Mot_get_sn_dist() + 1);
          redraw_required = true;
          break;

        case RINGTONES:
          sub_menu_current++;
          if ( sub_menu_current > SONGS_NUMBER )
            sub_menu_current = 0;
          redraw_required = true;
          break;
        
        default:
          break;
      }
    }
  }
}

void time_to_string(char* displayed_time, bool ck_al)
{
 
  uint8_t hours = RTC_get_hours(ck_al);
  uint8_t minutes = RTC_get_minutes(ck_al);
  
  displayed_time[2] = ':';
  displayed_time[5] = '\0';
  
  if(hours < 10) // check if first digit is zero or not
  {
    displayed_time[0] = '0';
    displayed_time[1] = (char)(hours + 48);
  }
  else
  {
    displayed_time[0] = (char)( (uint8_t)(hours /10) + 48);
    displayed_time[1] = (char)( hours - 10*(uint8_t)(hours /10) + 48);
  }

  if(minutes < 10)
  {
    displayed_time[3] = '0';
    displayed_time[4] = (char)(minutes + 48);
  }
  else
  {
    displayed_time[3] = (char)( (uint8_t)(minutes /10) + 48);
    displayed_time[4] = (char)( minutes - 10*(uint8_t)(minutes /10) + 48);
  }
}

