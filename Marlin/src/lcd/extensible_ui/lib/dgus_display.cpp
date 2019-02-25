#include "../../../inc/MarlinConfigPre.h"

#if ENABLED(DGUS_LCD)

//#include "../ui_api.h"

namespace ExtUI {
  //void onMediaInserted() {}
  //void onMediaError() {}
  //void onMediaRemoved() {}
  void onPlayTone(const uint16_t frequency, const uint16_t duration) {}
  void onPrintTimerStarted() {}
  void onPrintTimerPaused() {}
  void onPrintTimerStopped() {}
  void onFilamentRunout() {}
  //void onStatusChanged(const char * const msg) {}
  void onFactoryReset() {}
  void onLoadSettings() {}
  void onStoreSettings() {}
}

#endif // DGUS_LCD
