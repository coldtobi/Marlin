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

#include "../../inc/MarlinConfigPre.h"

#if ENABLED(DGUS_LCD)

/* DGUS implementation written by coldtobi in 2019 for Marlin */

  #include "../../Marlin.h"
  #include "DGUSVPVariable.h"

  enum DGUSLCD_Screens : uint8_t ;

  #if ENABLED(DEBUG_DGUSLCD)
    // default debug behaviour. true is reallly noisy.
    // (otherwise, use DGUSLCD_ScopeDebug)
    constexpr bool DEBUG_DGUSLCD_DEFAULTON = true;

    extern bool dguslcd_local_debug;

    // Instanciate one of those to locally tweak debugging. The level will be valid in the current scope.
    // on destruction, previous behaviour will be set again.
    class DGUSLCD_ScopeDebug {
    public:
        DGUSLCD_ScopeDebug(bool enable=true) : before(dguslcd_local_debug) {
            dguslcd_local_debug = enable;
        }

        ~DGUSLCD_ScopeDebug() {
            dguslcd_local_debug = before;
        }

    private:
        bool before;
    };

    #define DBG_DGUSLCD_ENABLE_DBG()           do { local_debug=true;} while(0)
    #define DBG_DGUSLCD_DGUS_DISBLE_DBG()      do { local_debug=false;} while(0)

    //#define DBG_DGUSLCD_SERIAL_ECHO_START()            if (dguslcd_local_debug) serialprintPGM(echomagic)
    #define DBG_DGUSLCD_SERIAL_ECHO(x)                 if (dguslcd_local_debug) SERIAL_ECHO(x)
    #define DBG_DGUSLCD_SERIAL_ECHOPGM(x)              if (dguslcd_local_debug) SERIAL_ECHOPGM(x)
    #define DBG_DGUSLCD_SERIAL_ECHOLN(x)               if (dguslcd_local_debug) SERIAL_ECHOLN(x)
    #define DBG_DGUSLCD_SERIAL_ECHOLNPGM(x)            if (dguslcd_local_debug) SERIAL_ECHOLNPGM(x)
    #define DBG_DGUSLCD_SERIAL_ECHOPAIR(pre,value)     if (dguslcd_local_debug) SERIAL_ECHOPAIR(pre, value)
    #define DBG_DGUSLCD_SERIAL_ECHOLNPAIR(pre,value)   if (dguslcd_local_debug) SERIAL_ECHOLNPAIR(pre, value)
    #define DBG_DGUSLCD_SERIAL_ECHO_F(x,y)             if (dguslcd_local_debug) SERIAL_ECHO_F(x,y)
  #else
    class DGUSLCD_ScopeDebug {public: DGUSLCD_ScopeDebug(bool ) {} DGUSLCD_ScopeDebug() {} }; // will be optimized out.
    #define DBG_DGUSLCD_ENABLE_DBG()
    #define DBG_DGUSLCD_DGUS_DISBLE_DBG()
    //#define DBG_DGUSLCD_SERIAL_ECHO_START()
    #define DBG_DGUSLCD_SERIAL_ECHO(x)
    #define DBG_DGUSLCD_SERIAL_ECHOPGM(x)
    #define DBG_DGUSLCD_SERIAL_ECHOLN(x)
    #define DBG_DGUSLCD_SERIAL_ECHOLNPGM(x)
    #define DBG_DGUSLCD_SERIAL_ECHOPAIR(pre,value)
    #define DBG_DGUSLCD_SERIAL_ECHOLNPAIR(pre,value)
    #define DBG_DGUSLCD_SERIAL_ECHO_F(x,y)
  #endif

  // Low-Level access to the display.
  class DGUSDisplay {
  public:

    DGUSDisplay() = default;

    void InitDisplay();

    // Variable access.
    void WriteVariable(uint16_t adr, const void* values, uint8_t valueslen, bool isstr = false);
    void WriteVariablePGM(uint16_t adr, const void* values, uint8_t valueslen, bool isstr = false);
    template<typename T>
    void WriteVariable(uint16_t adr, T value) {
      WriteVariable(adr, static_cast<const void*>(&value), sizeof(T));
    }

    // until now I did not have the need to actively read from the display thats why there is no ReadVariable
    // (I extensively use the auto upload of the display)

    /// Force display into another screen.
    /// (And trigger update of containing VPs)
    // (to implement an pop up message, which may not be nested)
  void RequestScreen(DGUSLCD_Screens screen);

  /// periodic tasks, eg. Rx-Queue handling.
  void loop();

public:
    // Helper for users of this class to estimate if an interaction would be blocking.
    size_t GetFreeTxBuffer() const;

    // Checks two things: Can we confirm the presence of the display and has we initiliazed it.
    // (both boils down that the display answered to our chatting)
    inline bool isInitialized() const { return Initialized;}

  private:
    void WriteHeader(uint16_t adr, uint8_t cmd, uint8_t payloadlen);
    void WritePGM(const char str[], uint8_t len);

    void ProcessRx();

  private:
    enum state {
      IDLE,  ///< waiting for DGUS_HEADER1.
      HEADER1_SEEN, ///< DGUS_HEADER1 received
      HEADER2_SEEN, ///< DGUS_HEADER2 received
      WAIT_TELEGRAM, ///< LEN received, Waiting for to receive all bytes.
    } rx_telegram_state = IDLE;

    uint8_t rx_telegram_len = 0;
    bool Initialized = false;
    bool no_reentrance = false;
  };

  extern DGUSDisplay dgusdisplay;

  // compile-time x^y
  constexpr float cpow(const float x, const int y) { return y == 0 ? 1.0 : x * cpow(x, y-1); }

  class DGUSScreenVariableHandler {
  public:
    // Gets callback from RX when screen has changed
    // Schedules updates for Variables
    DGUSScreenVariableHandler() = default;

    bool loop();

    /// Callback for VP "Screen has been changed"
    static void ScreenChangeHook(DGUS_VP_Variable &ref_to_this, void *ptr_to_new_value);
    /// Callback for VP "All Heaters Off"
    static void HandleAllHeatersOff(DGUS_VP_Variable &ref_to_this, void *ptr_to_new_value);
    /// Hook for "Change this temperature"
    static void HandleTemperatureChanged(DGUS_VP_Variable &ref_to_this, void *ptr_to_new_value);
    // Hook for manual move.
    static void HandleManualMove(DGUS_VP_Variable &ref_to_this, void *ptr_to_new_value);

    /// Update data after went to new screen (by display or by GotoScreen)
    /// remember: store the last displayed screen, so that one can get back
    /// to it. (e.g for pop up messages)
    void UpdateNewScreen(DGUSLCD_Screens newscreen, bool popup=false);

    /// Recall the remembered screen.
    void PopToOldScreen();

    /// Make the display display the screen and update all VPs in it.
    void GotoScreen(DGUSLCD_Screens screen);


    void UpdateScreenVPData();

    // Helpers to convert and transfer data to the display.
    static void DGUSLCD_SendWordValueToDisplay(DGUS_VP_Variable &ref_to_this);
    static void DGUSLCD_SendStringToDisplay(DGUS_VP_Variable &ref_to_this);
    static void DGUSLCD_SendStringToDisplayPGM(DGUS_VP_Variable &ref_to_this);
    static void DGUSLCD_SendPercentageToDisplay(DGUS_VP_Variable &ref_to_this);

    // Send a float value to the display.
    // Display will get an 4-byte integer scalled to the number of digits:
    // You tell the display the number of digits and it cheats by displaying a dot
    // between...
    template<unsigned int decimals>
    static void DGUSLCD_SendFloatAsLongValueToDisplay(DGUS_VP_Variable &ref_to_this) {
      if(ref_to_this.memadr) {
        DBG_DGUSLCD_SERIAL_ECHOPAIR(" DGUS_LCD_SendWordValueToDisplay ", ref_to_this.VP);
        //SERIAL_ECHO(" ");
        //SERIAL_ECHO_F(*(float*)ref_to_this.memadr, 2);
        float f= *(float *)ref_to_this.memadr;
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

    /// force an update of all VP on the current screen.
    inline void ForceCompleteUpdate() { update_ptr = 0; ScreenComplete = false; }

    /// has all VPs sent to the screen
    inline bool IsScreenComplete() { return ScreenComplete; }
  private:
    void privateScreenChangeHook(DGUS_VP_Variable &ref_to_this, void *ptr_to_new_value);

    DGUSLCD_Screens current_screen; ///< currently on screen
    static constexpr uint8_t NUM_PAST_SCREENS = 4;
    DGUSLCD_Screens past_screens[NUM_PAST_SCREENS]; ///< LIFO with past screens for the "back" button.

    uint8_t update_ptr = 0; ///< last sent entry in the VPList for the actual screen.
    uint16_t skipVP = 0; ///< when updating the screen data, skip this one, because the user is interacting with it.

    bool ScreenComplete = false; ///< All VPs sent to screen?
  };

  extern DGUSScreenVariableHandler ScreenHandler;

  /// Find the flash address of a DGUS_VP_Variable for the VP.
  extern const DGUS_VP_Variable* DGUSLCD_FindVPVar(uint16_t vp);

  /// Helper to populae a DGUS_VP_Variable for a given VP. returns false if not found.
  extern bool populate_VPVar(uint16_t VP,  DGUS_VP_Variable *ramcopy);

#endif
