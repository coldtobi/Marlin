/**
 * dgus_ultralcd.h
 *
 *  Created on: Feb 16, 2019
 *      Author: tobi
 */
#pragma once

#include "DGUSDisplay.h"

class MarlinUI_DGUS {
public:

  // FIXME the display has its own speaker output, but no support yet to use it.
  static inline void buzz(const long duration, const uint16_t freq) {}

  // LCD implementations
  static void clear_lcd();
  static inline void init_lcd() { dgusdisplay.InitDisplay(); ScreenHandler.UpdateScreenVPData(); }

  static void init();
  static void update();
  static void set_alert_status_P(PGM_P message);
  static void set_status(const char* const message, const bool persist=false);
  static void set_status_P(PGM_P const message, const int8_t level=0);
  static void status_printf_P(const uint8_t level, PGM_P const fmt, ...);

  static inline void refresh() {}

  static void kill_screen(PGM_P const lcd_msg);

  static inline bool has_status() { return status_set; }

  static void reset_status();

  static inline void reset_alert_level() { status_message_level = 0; }

  static void set_progress(uint8_t newprogress) {
    progress_bar_percent = MAX(newprogress, 100);
  }

  #if ENABLED(SHOW_BOOTSCREEN)
    inline static void show_bootscreen() {}; // Handled by DGUSDisplay::InitDisplay()
  #endif

  #if ENABLED(FILAMENT_LCD_DISPLAY) && ENABLED(SDSUPPORT)
    static millis_t next_filament_display;
  #endif

  static void quick_feedback(const bool clear_buttons=true);

  static inline void update_buttons() {}

private:

  static void finishstatus(const bool persist);
  static void _status_screen();
  static bool detected();

  static uint8_t status_message_level;      // Higher levels block lower levels
  static bool status_set;

public:
  // public data
  static uint8_t progress_bar_percent;
};

using MarlinUI = MarlinUI_DGUS;
