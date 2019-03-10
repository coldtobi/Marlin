
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

 void onMediaError() {
   ScreenHandler.SDCardError();
 }

 void onMediaRemoved() {
   ScreenHandler.SDCardRemoved();
 }

 void onPlayTone(const uint16_t frequency, const uint16_t duration) {}
 void onPrintTimerStarted() {}
 void onPrintTimerPaused() {}
 void onPrintTimerStopped() {}
 void onFilamentRunout() {}

  void onUserConfirmRequired(const char *msg) {
    if (msg) {
      ScreenHandler.sendinfoscreen("Please confirm.", "", msg, "");
      ScreenHandler.SetupConfirmAction(ExtUI::setUserConfirmed);
      ScreenHandler.GotoScreen(DGUSLCD_SCREEN_POPUP);
    } else
    if (ScreenHandler.getCurrentScreen() == DGUSLCD_SCREEN_POPUP ) {
      ScreenHandler.SetupConfirmAction(nullptr);
      ScreenHandler.PopToOldScreen();
    }
  }

  void onStatusChanged(const char * const msg) {
     ScreenHandler.setstatusmessage(msg);
  }

 void onFactoryReset() {}
 void onLoadSettings() {}
 void onStoreSettings() {}
}

#endif // DGUS_LCD
