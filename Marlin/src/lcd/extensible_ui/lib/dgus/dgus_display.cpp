
#include "../../../../inc/MarlinConfigPre.h"

#if ENABLED(DGUS_LCD)

#include "../../ui_api.h"

#include "DGUSDisplay.h"
#include "DGUSDisplayDefinition.h"

namespace ExtUI {

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


void onStartup() {
    dgusdisplay.InitDisplay();
    ScreenHandler.UpdateScreenVPData();
  }

 void onIdle() {
   ScreenHandler.loop();
 }

 void onPrinterKilled(const char* msg) {
   sendinfoscreen(PSTR(MSG_HALTED), msg, PSTR(""), PSTR(MSG_PLEASE_RESET));
   ScreenHandler.GotoScreen(DGUSLCD_SCREEN_KILL);
   while(!ScreenHandler.loop());  // Wait if there is something still to be sent to the display.
 }

 void onMediaInserted() {};
 void onMediaError() {};
 void onMediaRemoved() {};
 void onPlayTone(const uint16_t frequency, const uint16_t duration) {}
 void onPrintTimerStarted() {}
 void onPrintTimerPaused() {}
 void onPrintTimerStopped() {}
 void onFilamentRunout() {}

 void onStatusChanged(const char * const msg) {
    DGUS_VP_Variable ramcopy;
    if (populate_VPVar(VP_M117, &ramcopy)) {
      ramcopy.memadr = (void*) msg;
      if (ramcopy.send_to_display_handler) {
        ramcopy.send_to_display_handler(ramcopy);
      }
    }
 }

 void onFactoryReset() {}
 void onLoadSettings() {}
 void onStoreSettings() {}
}

#endif // DGUS_LCD
