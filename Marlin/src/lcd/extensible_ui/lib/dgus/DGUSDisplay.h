/**
 * Marlin 3D Printer Firmware
 * Copyright (C) 2019 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#pragma once

/* DGUS implementation written by coldtobi in 2019 for Marlin */

#include "../../../../inc/MarlinConfigPre.h"

#include "../../../../Marlin.h"
#include "DGUSVPVariable.h"

enum DGUSLCD_Screens : uint8_t;

#if ENABLED(DEBUG_DGUSLCD)
  extern bool dguslcd_local_debug;
  #define DGUS_DEBUG(V) REMEMBER(dgus,dguslcd_local_debug,V)
#else
  constexpr bool dguslcd_local_debug = false;
  #define DGUS_DEBUG(V)
#endif

#define DGUS_CHAR(x)                 do{if (dguslcd_local_debug) SERIAL_CHAR(x);}while(0)
#define DGUS_ECHO(x)                 do{if (dguslcd_local_debug) SERIAL_ECHO(x);}while(0)
#define DGUS_ECHOPGM(x)              do{if (dguslcd_local_debug) SERIAL_ECHOPGM(x);}while(0)
#define DGUS_ECHOLN(x)               do{if (dguslcd_local_debug) SERIAL_ECHOLN(x);}while(0)
#define DGUS_ECHOLNPGM(x)            do{if (dguslcd_local_debug) SERIAL_ECHOLNPGM(x);}while(0)
#define DGUS_ECHOPAIR(pre,value)     do{if (dguslcd_local_debug) SERIAL_ECHOPAIR(pre, value);}while(0)
#define DGUS_ECHOLNPAIR(pre,value)   do{if (dguslcd_local_debug) SERIAL_ECHOLNPAIR(pre, value);}while(0)
#define DGUS_ECHO_F(x,y)             do{if (dguslcd_local_debug) SERIAL_ECHO_F(x,y);}while(0)

typedef enum : uint8_t {
  DGUS_IDLE,           //< waiting for DGUS_HEADER1.
  DGUS_HEADER1_SEEN,   //< DGUS_HEADER1 received
  DGUS_HEADER2_SEEN,   //< DGUS_HEADER2 received
  DGUS_WAIT_TELEGRAM,  //< LEN received, Waiting for to receive all bytes.
} rx_datagram_state_t;

// Low-Level access to the display.
class DGUSDisplay {
public:

  DGUSDisplay() = default;

  static void InitDisplay();

  // Variable access.
  static void WriteVariable(uint16_t adr, const void* values, uint8_t valueslen, bool isstr=false);
  static void WriteVariablePGM(uint16_t adr, const void* values, uint8_t valueslen, bool isstr=false);
  template<typename T>
  static void WriteVariable(uint16_t adr, T value) {
    WriteVariable(adr, static_cast<const void*>(&value), sizeof(T));
  }

  // Until now I did not have the need to actively read from the display. That's why there is no ReadVariable
  // (I extensively use the auto upload of the display)

  // Force display into another screen.
  // (And trigger update of containing VPs)
  // (to implement an pop up message, which may not be nested)
  static void RequestScreen(DGUSLCD_Screens screen);

  // Periodic tasks, eg. Rx-Queue handling.
  static void loop();

public:
  // Helper for users of this class to estimate if an interaction would be blocking.
  static size_t GetFreeTxBuffer();

  // Checks two things: Can we confirm the presence of the display and has we initiliazed it.
  // (both boils down that the display answered to our chatting)
  static inline bool isInitialized() { return Initialized; }

private:
  static void WriteHeader(uint16_t adr, uint8_t cmd, uint8_t payloadlen);
  static void WritePGM(const char str[], uint8_t len);
  static void ProcessRx();

  static rx_datagram_state_t rx_datagram_state;
  static uint8_t rx_datagram_len;
  static bool Initialized, no_reentrance;
};

extern DGUSDisplay dgusdisplay;

// compile-time x^y
constexpr float cpow(const float x, const int y) { return y == 0 ? 1.0 : x * cpow(x, y-1); }

class DGUSScreenVariableHandler {
public:
  // Gets callback from RX when screen has changed
  // Schedules updates for Variables
  DGUSScreenVariableHandler() = default;

  static bool loop();

  // Callback for VP "Display wants to change screen on idle printer"
  static void ScreenChangeHookIfIdle(DGUS_VP_Variable &ref_to_this, void *ptr_to_new_value);
  // Callback for VP "Screen has been changed"
  static void ScreenChangeHook(DGUS_VP_Variable &ref_to_this, void *ptr_to_new_value);
  // Callback for VP "All Heaters Off"
  static void HandleAllHeatersOff(DGUS_VP_Variable &ref_to_this, void *ptr_to_new_value);
  // Hook for "Change this temperature"
  static void HandleTemperatureChanged(DGUS_VP_Variable &ref_to_this, void *ptr_to_new_value);
  // Hook for manual move.
  static void HandleManualMove(DGUS_VP_Variable &ref_to_this, void *ptr_to_new_value);


  // Update data after went to new screen (by display or by GotoScreen)
  // remember: store the last displayed screen, so that one can get back
  // to it. (e.g for pop up messages)
  static void UpdateNewScreen(DGUSLCD_Screens newscreen, bool popup=false);

  // Recall the remembered screen.
  static void PopToOldScreen();

  // Make the display display the screen and update all VPs in it.
  static void GotoScreen(DGUSLCD_Screens screen);

  static void UpdateScreenVPData();

  // Helpers to convert and transfer data to the display.
  static void DGUSLCD_SendWordValueToDisplay(DGUS_VP_Variable &ref_to_this);
  static void DGUSLCD_SendStringToDisplay(DGUS_VP_Variable &ref_to_this);
  static void DGUSLCD_SendStringToDisplayPGM(DGUS_VP_Variable &ref_to_this);
  static void DGUSLCD_SendPercentageToDisplay(DGUS_VP_Variable &ref_to_this);

  // Send a value from 0..100 to a variable with a range from 0..255
  static void DGUSLCD_PercentageToUint8(DGUS_VP_Variable &ref_to_this, void *ptr_to_new_value);

  template<typename T>
  static void DGUSLCD_SetValueDirectly(DGUS_VP_Variable &ref_to_this, void *ptr_to_new_value) {
    DGUS_ECHOLN(__FUNCTION__);
    DGUS_ECHOPAIR("value", *(T*)ptr_to_new_value);

    if (!ref_to_this.memadr) return;

    union {
      unsigned char tmp[sizeof(T)];
      T t;
    } x;
    unsigned char *ptr = (unsigned char*)ptr_to_new_value;
    for(uint8_t i=0; i < sizeof(T); i++) {
       x.tmp[i] = ptr[sizeof(T) - i - 1 ];
    }
    *(T*)ref_to_this.memadr = x.t;
  }

  // Send a float value to the display.
  // Display will get a 4-byte integer scaled to the number of digits:
  // Tell the display the number of digits and it cheats by displaying a dot between...
  template<unsigned int decimals>
  static void DGUSLCD_SendFloatAsLongValueToDisplay(DGUS_VP_Variable &ref_to_this) {
    if (ref_to_this.memadr) {
      DGUS_ECHOPAIR(" DGUS_LCD_SendWordValueToDisplay ", ref_to_this.VP);
      //SERIAL_ECHO(" ");
      //SERIAL_ECHO_F(*(float*)ref_to_this.memadr, 2);
      float f = *(float *)ref_to_this.memadr;
      f *= cpow(10,decimals);
      union {
        long l;
        char lb[4];
      } endian;

      char tmp[4];
      endian.l = f;
      tmp[0] = endian.lb[3];
      tmp[1] = endian.lb[2];
      tmp[2] = endian.lb[1];
      tmp[3] = endian.lb[0];
      dgusdisplay.WriteVariable(ref_to_this.VP, tmp, 4);
    }
  }



  // Force an update of all VP on the current screen.
  static inline void ForceCompleteUpdate() { update_ptr = 0; ScreenComplete = false; }

  // Has all VPs sent to the screen
  static inline bool IsScreenComplete() { return ScreenComplete; }

private:

  static DGUSLCD_Screens current_screen;  //< currently on screen
  static constexpr uint8_t NUM_PAST_SCREENS = 4;
  static DGUSLCD_Screens past_screens[NUM_PAST_SCREENS]; //< LIFO with past screens for the "back" button.

  static uint8_t update_ptr;    //< Last sent entry in the VPList for the actual screen.
  static uint16_t skipVP;       //< When updating the screen data, skip this one, because the user is interacting with it.

  static bool ScreenComplete;   //< All VPs sent to screen?
};

extern DGUSScreenVariableHandler ScreenHandler;

/// Find the flash address of a DGUS_VP_Variable for the VP.
extern const DGUS_VP_Variable* DGUSLCD_FindVPVar(uint16_t vp);

/// Helper to populae a DGUS_VP_Variable for a given VP. returns false if not found.
extern bool populate_VPVar(uint16_t VP,  DGUS_VP_Variable *ramcopy);
