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

/* DGUS implementation written by coldtobi in 2019 for Marlin */

#include "../../../../inc/MarlinConfigPre.h"

#if ENABLED(DGUS_LCD)

#include "DGUSDisplayDefinition.h"
#include "DGUSDisplay.h"

#include "../../../../module/temperature.h"
#include "../../../../module/motion.h"

#include "../../../ultralcd.h"

const uint16_t VPList_Boot[] PROGMEM = {
  VP_MARLIN_VERSION,
  0x0000
};

const uint16_t VPList_Main[] PROGMEM = {
  /* VP_M117, for completeness, but it cannot be auto-uploaded.*/
  0x0000
};

const uint16_t VPList_Temp[] PROGMEM = {
  #if HOTENDS >= 1
    VP_T_E1_Is, VP_T_E1_Set,
  #endif
  #if HOTENDS >= 2
    VP_T_E2_I, VP_T_E2_S,
  #endif
  #if HAS_HEATED_BED
    VP_T_Bed_Is, VP_T_Bed_Set,
  #endif
  0x0000
};

const uint16_t VPList_Status[] PROGMEM = {
  /* VP_M117, for completeness, but it cannot be auto-uploaded because.*/
  #if HOTENDS >= 1
    VP_T_E1_Is, VP_T_E1_Set,
  #endif
  #if HOTENDS >= 2
    VP_T_E2_I, VP_T_E2_S,
  #endif
  #if HAS_HEATED_BED
    VP_T_Bed_Is, VP_T_Bed_Set,
  #endif
  #if FAN_COUNT > 0
    VP_Fan_Percentage,
  #endif
  VP_XPos, VP_YPos, VP_ZPos,
  VP_Fan_Percentage,
  VP_Feedrate_Percentage,
  VP_PrintProgress_Percentage,
  0x0000
};

const uint16_t VPList_ManualMove[] PROGMEM = {
  VP_XPos, VP_YPos, VP_ZPos,
  0x0000
};

const uint16_t VPList_FanAndFeedrate[] PROGMEM = {
  VP_Feedrate_Percentage, VP_Fan_Percentage,
  0x0000
};

const struct VPMapping VPMap[] PROGMEM = {
  { DGUSLCD_SCREEN_BOOT , VPList_Boot },
  { DGUSLCD_SCREEN_MAIN , VPList_Main },
  { DGUSLCD_SCREEN_TEMPERATURE , VPList_Temp },
  { DGUSLCD_SCREEN_STATUS , VPList_Status },
  { DGUSLCD_SCREEN_MANUALMOVE , VPList_ManualMove },
  { DGUSLCD_SCREEN_FANANDFEEDRATE , VPList_FanAndFeedrate },
  { 0 , nullptr } // List is terminated with an nullptr as table entry.
};

const char MarlinVersion[] PROGMEM = SHORT_BUILD_VERSION;

// Helper to define a DGUS_VP_Variable for common use cases.
#define VPHELPER(VPADR, VPADRVAR, RXFPTR, TXFPTR ) { .VP=VPADR, .memadr=VPADRVAR, .size=sizeof(VPADRVAR), \
  .set_by_display_handler = RXFPTR, .send_to_display_handler = TXFPTR }

const struct DGUS_VP_Variable ListOfVP[] PROGMEM = {
  // Helper to detect touch events
  VPHELPER(VP_SCREENCHANGE_ASK, nullptr, &DGUSScreenVariableHandler::ScreenChangeHookIfIdle, nullptr),
  VPHELPER(VP_SCREENCHANGE, nullptr, &DGUSScreenVariableHandler::ScreenChangeHook, nullptr),
  VPHELPER(VP_TEMP_ALL_OFF, nullptr, &DGUSScreenVariableHandler::HandleAllHeatersOff, nullptr),

  VPHELPER(VP_MOVE_X, nullptr, &DGUSScreenVariableHandler::HandleManualMove, nullptr),
  VPHELPER(VP_MOVE_Y, nullptr, &DGUSScreenVariableHandler::HandleManualMove, nullptr),
  VPHELPER(VP_MOVE_Z, nullptr, &DGUSScreenVariableHandler::HandleManualMove, nullptr),
  VPHELPER(VP_HOME_ALL, nullptr, &DGUSScreenVariableHandler::HandleManualMove, nullptr),

  { .VP = VP_MARLIN_VERSION, .memadr = (void*)MarlinVersion, .size = VP_MARLIN_VERSION_LEN, .set_by_display_handler = nullptr, .send_to_display_handler =&DGUSScreenVariableHandler::DGUSLCD_SendStringToDisplayPGM },
  // M117 LCD String (We don't need the string in memory but "just" push it to the display on demand, hence the nullptr
  { .VP = VP_M117, .memadr = nullptr, .size = VP_M117_LEN, .set_by_display_handler = nullptr, .send_to_display_handler =&DGUSScreenVariableHandler::DGUSLCD_SendStringToDisplay },

  // Temperature Data
  #if HOTENDS >= 1
    VPHELPER(VP_T_E1_Is, &thermalManager.current_temperature[0], nullptr, &DGUSScreenVariableHandler::DGUSLCD_SendFloatAsLongValueToDisplay<0>),
    VPHELPER(VP_T_E1_Set, &thermalManager.target_temperature[0], &DGUSScreenVariableHandler::HandleTemperatureChanged, &DGUSScreenVariableHandler::DGUSLCD_SendWordValueToDisplay),
  #endif
  #if HOTENDS > 2
    VPHELPER(VP_T_E2_I, &thermalManager.current_temperature[1], nullptr, &DGUSLCD_SendFloatAsLongValueToDisplay<0>),
    VPHELPER(VP_T_E2_S, &thermalManager.target_temperature[1], &DGUSScreenVariableHandler::HandleTemperatureChanged, &DGUSScreenVariableHandler::DGUSLCD_SendWordValueToDisplay),
  #endif
  #if HOTENDS > 3
    #warning More than 2 Hotends currently not implemented on the Display UI design.
  #endif
  #if HAS_HEATED_BED
    VPHELPER(VP_T_Bed_Is, &thermalManager.current_temperature_bed, nullptr, &DGUSScreenVariableHandler::DGUSLCD_SendFloatAsLongValueToDisplay<0>),
    VPHELPER(VP_T_Bed_Set, &thermalManager.target_temperature_bed, &DGUSScreenVariableHandler::HandleTemperatureChanged, &DGUSScreenVariableHandler::DGUSLCD_SendWordValueToDisplay),
  #endif

  // Fan Data.
  #if FAN_COUNT > 0
    VPHELPER(VP_Fan_Percentage, &thermalManager.fan_speed[0], &DGUSScreenVariableHandler::DGUSLCD_PercentageToUint8, &DGUSScreenVariableHandler::DGUSLCD_SendPercentageToDisplay),
  #endif

  // Feedrate.
  VPHELPER(VP_Feedrate_Percentage, &feedrate_percentage, DGUSScreenVariableHandler::DGUSLCD_SetValueDirectly<int16_t>, &DGUSScreenVariableHandler::DGUSLCD_SendWordValueToDisplay ),

  // Position Data.
  VPHELPER(VP_XPos, &current_position[0], nullptr, &DGUSScreenVariableHandler::DGUSLCD_SendFloatAsLongValueToDisplay<2>),
  VPHELPER(VP_YPos, &current_position[1], nullptr, &DGUSScreenVariableHandler::DGUSLCD_SendFloatAsLongValueToDisplay<2>),
  VPHELPER(VP_ZPos, &current_position[2], nullptr, &DGUSScreenVariableHandler::DGUSLCD_SendFloatAsLongValueToDisplay<2>),

  // Print Progress.
  VPHELPER(VP_PrintProgress_Percentage, &MarlinUI::progress_bar_percent, nullptr, &DGUSScreenVariableHandler::DGUSLCD_SendPercentageToDisplay ),


  // Messages for the User, shared by the popup and the kill screen.
  { .VP = VP_MSGSTR1, .memadr = nullptr, .size = VP_MSGSTR1_LEN, .set_by_display_handler = nullptr, .send_to_display_handler = &DGUSScreenVariableHandler::DGUSLCD_SendStringToDisplayPGM },
  { .VP = VP_MSGSTR2, .memadr = nullptr, .size = VP_MSGSTR2_LEN, .set_by_display_handler = nullptr, .send_to_display_handler = &DGUSScreenVariableHandler::DGUSLCD_SendStringToDisplayPGM },
  { .VP = VP_MSGSTR3, .memadr = nullptr, .size = VP_MSGSTR3_LEN, .set_by_display_handler = nullptr, .send_to_display_handler = &DGUSScreenVariableHandler::DGUSLCD_SendStringToDisplayPGM },
  { .VP = VP_MSGSTR4, .memadr = nullptr, .size = VP_MSGSTR4_LEN, .set_by_display_handler = nullptr, .send_to_display_handler = &DGUSScreenVariableHandler::DGUSLCD_SendStringToDisplayPGM },

  VPHELPER(0, 0, 0, 0)  // must be last entry.
};

#endif // DGUS_LCD
