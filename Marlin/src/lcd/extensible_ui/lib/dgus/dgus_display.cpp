
#include "../../../../inc/MarlinConfigPre.h"

#if ENABLED(DGUS_LCD)

#include "../../ui_api.h"

#include "DGUSDisplay.h"
#include "DGUSDisplayDefinition.h"

namespace ExtUI {

void onStartup() {
    dgusdisplay.InitDisplay();
    ScreenHandler.UpdateScreenVPData();
  }

void OnShowBootscreen() { /* handled by dgusdisplay.InitDisplay() already */};

void OnExitBootscreen() {
  ScreenHandler.GotoScreen(DGUSLCD_SCREEN_MAIN);
};

 void onIdle() {
   ScreenHandler.loop();
 }

 void onPrinterKilled(const char* msg) {
   ScreenHandler.sendinfoscreenPGM(PSTR(MSG_HALTED), msg, PSTR(""), PSTR(MSG_PLEASE_RESET));
   ScreenHandler.GotoScreen(DGUSLCD_SCREEN_KILL);
   while(!ScreenHandler.loop());  // Wait if there is something still to be sent to the display.
 }

 void onMediaInserted() {
   ScreenHandler.SDCardInserted();
 }

 void onMediaError() { }

 void onMediaRemoved() {
   ScreenHandler.SDCardRemoved();
 }

 void onPlayTone(const uint16_t frequency, const uint16_t duration) {}
 void onPrintTimerStarted() {}
 void onPrintTimerPaused() {}
 void onPrintTimerStopped() {}
 void onFilamentRunout() {}

 void onStatusChanged(const char * const msg) {
     ScreenHandler.setstatusmessage(msg);
 }

 void onFactoryReset() {}
 void onLoadSettings() {}
 void onStoreSettings() {}
}

#endif // DGUS_LCD
