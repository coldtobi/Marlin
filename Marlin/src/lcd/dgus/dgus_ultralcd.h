/*
 * dgus_ultralcd.h
 *
 *  Created on: Feb 16, 2019
 *      Author: tobi
 */

#include "DGUSDisplay.h"

#pragma once

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

      #if HAS_SPI_LCD
        #if ENABLED(SHOW_BOOTSCREEN)
          inline static void show_bootscreen() {}; // Handled by DGUSDisplay::InitDisplay()
        #endif

        #if ENABLED(FILAMENT_LCD_DISPLAY) && ENABLED(SDSUPPORT)
          static millis_t next_filament_display;
        #endif

        static void quick_feedback(const bool clear_buttons=true);

       #endif

       #if HAS_LCD_MENU

        #if ENABLED(SDSUPPORT)
          #if ENABLED(SCROLL_LONG_FILENAMES)
            static uint8_t filename_scroll_pos, filename_scroll_max;
          #endif
          static const char * scrolled_filename(CardReader &theCard, const uint8_t maxlen, uint8_t hash, const bool doScroll);
        #endif

        #if IS_KINEMATIC
          static bool processing_manual_move;
        #else
          static constexpr bool processing_manual_move = false;
        #endif

        #if E_MANUAL > 1
          static int8_t manual_move_e_index;
        #else
          static constexpr int8_t manual_move_e_index = 0;
        #endif

        static int16_t preheat_hotend_temp[2], preheat_bed_temp[2];
        static uint8_t preheat_fan_speed[2];

        static void manage_manual_move();

        static bool lcd_clicked;
        static bool use_click();

        static void synchronize(PGM_P const msg=NULL);

        static screenFunc_t currentScreen;
        static void goto_screen(const screenFunc_t screen, const uint32_t encoder=0);
        static void save_previous_screen();
        static void goto_previous_screen();
        static void return_to_status();
        static inline bool on_status_screen() { return currentScreen == status_screen; }
        static inline void run_current_screen() { (*currentScreen)(); }

        #if ENABLED(LIGHTWEIGHT_UI)
          static void lcd_in_status(const bool inStatus);
        #endif

        static inline void defer_status_screen(const bool defer) {
          #if LCD_TIMEOUT_TO_STATUS
            defer_return_to_status = defer;
          #else
            UNUSED(defer);
          #endif
        }

        static inline void goto_previous_screen_no_defer() {
          defer_status_screen(false);
          goto_previous_screen();
        }

        #if ENABLED(SD_REPRINT_LAST_SELECTED_FILE)
          static void reselect_last_file();
        #endif

        #if ENABLED(G26_MESH_VALIDATION)
          static inline void chirp() { buzz(LCD_FEEDBACK_FREQUENCY_DURATION_MS, LCD_FEEDBACK_FREQUENCY_HZ); }
        #endif

        #if ENABLED(AUTO_BED_LEVELING_UBL)
          static void ubl_plot(const uint8_t x, const uint8_t inverted_y);
        #endif

      #endif

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

