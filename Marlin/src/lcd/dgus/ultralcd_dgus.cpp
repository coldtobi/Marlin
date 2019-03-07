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

#include "../../inc/MarlinConfigPre.h"

#if ENABLED(DGUS_LCD)

#include "DGUSDisplay.h"
#include "DGUSDisplayDefinition.h"
#include "DGUSVPVariable.h"

#include "../ultralcd.h"
MarlinUI_DGUS ui;

#include "../../sd/cardreader.h"

#include "../lcdprint.h"

#include "../../sd/cardreader.h"
#include "../../module/temperature.h"
#include "../../module/planner.h"
#include "../../module/printcounter.h"
#include "../../module/motion.h"
#include "../../gcode/queue.h"

#include "../../Marlin.h"

#if ENABLED(POWER_LOSS_RECOVERY)
  #include "../feature/power_loss_recovery.h"
#endif

#if ENABLED(AUTO_BED_LEVELING_UBL)
  #include "../feature/bedlevel/bedlevel.h"
#endif

#if HAS_BUZZER
  #include "../../libs/buzzer.h"
#endif

#if HAS_TRINAMIC
  #include "../../feature/tmc_util.h"
#endif

uint8_t MarlinUI_DGUS::progress_bar_percent = 0;

#if ENABLED(SDSUPPORT) && PIN_EXISTS(SD_DETECT)
  uint8_t lcd_sd_status;
#endif

#if ENABLED(FILAMENT_LCD_DISPLAY) && ENABLED(SDSUPPORT)
  millis_t MarlinUI::next_filament_display; // = 0
#endif

#if HAS_GRAPHICAL_LCD
  bool MarlinUI::drawing_screen, MarlinUI::first_page; // = false
#endif

void MarlinUI_DGUS::init() {

  init_lcd();

  #if ENABLED(SDSUPPORT) && PIN_EXISTS(SD_DETECT)
    SET_INPUT_PULLUP(SD_DETECT_PIN);
    lcd_sd_status = 2; // UNKNOWN
  #endif
}

static void sendinfoscreen(const char* line1, const char* line2, const char* line3, const char* line4) {
  DGUS_VP_Variable ramcopy;
  if (populate_VPVar(VP_MSGSTR1, &ramcopy)) {
    ramcopy.memadr = (void*) line1;
    DGUSScreenVariableHandler::DGUSLCD_SendStringToDisplayPGM(ramcopy);
  }
  if (populate_VPVar(VP_MSGSTR2, &ramcopy)) {
    ramcopy.memadr = (void*) line2;
    DGUSScreenVariableHandler::DGUSLCD_SendStringToDisplayPGM(ramcopy);
  }
  if (populate_VPVar(VP_MSGSTR3, &ramcopy)) {
    ramcopy.memadr = (void*) line3;
    DGUSScreenVariableHandler::DGUSLCD_SendStringToDisplayPGM(ramcopy);
  }
  if (populate_VPVar(VP_MSGSTR4, &ramcopy)) {
    ramcopy.memadr = (void*) line4;
    DGUSScreenVariableHandler::DGUSLCD_SendStringToDisplayPGM(ramcopy);
  }
}

void MarlinUI_DGUS::kill_screen(PGM_P lcd_msg) {
  sendinfoscreen(PSTR(MSG_HALTED), lcd_msg, PSTR(""), PSTR(MSG_PLEASE_RESET));
  ScreenHandler.GotoScreen(DGUSLCD_SCREEN_KILL);
  while(!ScreenHandler.loop());  // Wait if there is something still to be sent to the display.
}

void MarlinUI::quick_feedback(const bool clear_buttons/*=true*/) {
  UNUSED(clear_buttons);
  buzz(LCD_FEEDBACK_FREQUENCY_DURATION_MS, LCD_FEEDBACK_FREQUENCY_HZ);
}

/**
 * Update the LCD, read encoder buttons, etc.
 *   - Read button states
 *   - Check the SD Card slot state
 *   - Act on RepRap World keypad input
 *   - Update the encoder position
 *   - Apply acceleration to the encoder position
 *   - Do refresh(LCDVIEW_CALL_REDRAW_NOW) on controller events
 *   - Reset the Info Screen timeout if there's any input
 *   - Update status indicators, if any
 *
 *   Run the current LCD menu handler callback function:
 *   - Call the handler only if lcdDrawUpdate != LCDVIEW_NONE
 *   - Before calling the handler, LCDVIEW_CALL_NO_REDRAW => LCDVIEW_NONE
 *   - Call the menu handler. Menu handlers should do the following:
 *     - If a value changes, set lcdDrawUpdate to LCDVIEW_REDRAW_NOW and draw the value
 *       (Encoder events automatically set lcdDrawUpdate for you.)
 *     - if (should_draw()) { redraw }
 *     - Before exiting the handler set lcdDrawUpdate to:
 *       - LCDVIEW_CLEAR_CALL_REDRAW to clear screen and set LCDVIEW_CALL_REDRAW_NEXT.
 *       - LCDVIEW_REDRAW_NOW to draw now (including remaining stripes).
 *       - LCDVIEW_CALL_REDRAW_NEXT to draw now and get LCDVIEW_REDRAW_NOW on the next loop.
 *       - LCDVIEW_CALL_NO_REDRAW to draw now and get LCDVIEW_NONE on the next loop.
 *     - NOTE: For graphical displays menu handlers may be called 2 or more times per loop,
 *             so don't change lcdDrawUpdate without considering this.
 *
 *   After the menu handler callback runs (or not):
 *   - Clear the LCD if lcdDrawUpdate == LCDVIEW_CLEAR_CALL_REDRAW
 *   - Update lcdDrawUpdate for the next loop (i.e., move one state down, usually)
 *
 * This function is only called from the main thread.
 */

bool MarlinUI_DGUS::detected() {
  return dgusdisplay.isInitialized();
}

void MarlinUI_DGUS::update() {

  static millis_t next_lcd_update_ms;

  #if ENABLED(SDSUPPORT) && PIN_EXISTS(SD_DETECT)

    const uint8_t sd_status = (uint8_t)IS_SD_INSERTED();
    if (sd_status != lcd_sd_status && detected()) {

      uint8_t old_sd_status = lcd_sd_status; // prevent re-entry to this block!
      lcd_sd_status = sd_status;

      if (sd_status) {
        safe_delay(500); // Some boards need a delay to get settled
        card.initsd();
        if (old_sd_status == 2)
          card.beginautostart();  // Initial boot
        else
          set_status_P(PSTR(MSG_SD_INSERTED));
      }
      else {
        card.release();
        if (old_sd_status != 2) {
          set_status_P(PSTR(MSG_SD_REMOVED));
          // if (!on_status_screen()) return_to_status();  // FIXME -- at least leave SD menu.
        }
      }
    }

  #endif // SDSUPPORT && SD_DETECT_PIN

  const millis_t ms = millis();
  if (ELAPSED(ms, next_lcd_update_ms)
    #if HAS_GRAPHICAL_LCD
      || drawing_screen
    #endif
  ) {

    next_lcd_update_ms = ms + LCD_UPDATE_INTERVAL;

    #if ENABLED(LCD_HAS_STATUS_INDICATORS)
      update_indicators();
    #endif

  ScreenHandler.loop();

  } // ELAPSED(ms, next_lcd_update_ms)
}


////////////////////////////////////////////
/////////////// Status Line ////////////////
////////////////////////////////////////////

void MarlinUI::finishstatus(const bool persist) {

  #if !(ENABLED(LCD_PROGRESS_BAR) && (PROGRESS_MSG_EXPIRE > 0))
    UNUSED(persist);
  #endif

  #if ENABLED(LCD_PROGRESS_BAR)
    progress_bar_ms = millis();
    #if PROGRESS_MSG_EXPIRE > 0
      expire_status_ms = persist ? 0 : progress_bar_ms + PROGRESS_MSG_EXPIRE;
    #endif
  #endif

  #if ENABLED(FILAMENT_LCD_DISPLAY) && ENABLED(SDSUPPORT)
    next_filament_display = millis() + 5000UL; // Show status message for 5s
  #endif

  refresh();
}

 bool MarlinUI_DGUS::status_set = false;
 uint8_t MarlinUI_DGUS::status_message_level = 0;


void MarlinUI_DGUS::set_status(const char * const message, const bool persist) {
  if (status_message_level > 0) return;
  if (!message) return;

  status_set = (message[0] == '\0' ? false : true);

  // The DGUS Display does not support UTF8, so we just pass it to it as it...
  DGUS_VP_Variable ramcopy;
  if (populate_VPVar(VP_M117, &ramcopy)) {
    ramcopy.memadr = (void*) message;
    if (ramcopy.send_to_display_handler) {
      ramcopy.send_to_display_handler(ramcopy);
    }
  }

  finishstatus(persist);
}

#include <stdarg.h>

void MarlinUI_DGUS::status_printf_P(const uint8_t level, PGM_P const fmt, ...) {

  if (level < status_message_level) return;
  va_list args;
  va_start(args, fmt);
  char status_message[VP_M117_LEN];
  vsnprintf_P(status_message, VP_M117_LEN, fmt, args);
  va_end(args);
  status_message_level = 0;
  set_status(status_message, level > 0);
  status_message_level = level;
}

void MarlinUI_DGUS::set_status_P(PGM_P const message, int8_t level) {
  if (level < 0) level = status_message_level = 0;
  if (level < status_message_level) return;
  status_message_level = level;

  status_set = (pgm_read_byte(message) == '\0' ? false : true);

  DGUS_VP_Variable ramcopy;
  if (!populate_VPVar(VP_M117, &ramcopy)) return;
  ramcopy.memadr = (void*) message;
  // we need to divert the handler here, as the one in the VP is the ram-based one.
  DGUSScreenVariableHandler::DGUSLCD_SendStringToDisplayPGM(ramcopy);

  finishstatus(level > 0);
}

void MarlinUI_DGUS::set_alert_status_P(PGM_P const message) {
  set_status_P(message, 1);
  ScreenHandler.GotoScreen(DGUSLCD_SCREEN_MAIN);  // or should that be a popup?
}

#include "../../module/printcounter.h"

/**
 * Reset the status message
 */
void MarlinUI_DGUS::reset_status() {
  static const char paused[] PROGMEM = MSG_PRINT_PAUSED;
  static const char printing[] PROGMEM = MSG_PRINTING;
  static const char welcome[] PROGMEM = WELCOME_MSG;
  #if SERVICE_INTERVAL_1 > 0
    static const char service1[] PROGMEM = { "> " SERVICE_NAME_1 "!" };
  #endif
  #if SERVICE_INTERVAL_2 > 0
    static const char service2[] PROGMEM = { "> " SERVICE_NAME_2 "!" };
  #endif
  #if SERVICE_INTERVAL_3 > 0
    static const char service3[] PROGMEM = { "> " SERVICE_NAME_3 "!" };
  #endif
  PGM_P msg;
  if (!IS_SD_PRINTING() && print_job_timer.isPaused())
    msg = paused;
  #if ENABLED(SDSUPPORT)
    else if (IS_SD_PRINTING())
      return set_status(card.longest_filename(), true);
  #endif
  else if (print_job_timer.isRunning())
    msg = printing;

  #if SERVICE_INTERVAL_1 > 0
    else if (print_job_timer.needsService(1)) msg = service1;
  #endif
  #if SERVICE_INTERVAL_2 > 0
    else if (print_job_timer.needsService(2)) msg = service2;
  #endif
  #if SERVICE_INTERVAL_3 > 0
    else if (print_job_timer.needsService(3)) msg = service3;
  #endif

  else
    msg = welcome;

  set_status_P(msg, -1);
}

#endif // DGUS_LCD
