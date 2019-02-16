/*************
 * dummy.cpp *
 *************/

/****************************************************************************
 *   Written By Marcio Teixeira 2018 - Aleph Objects, Inc.                  *
 *                                                                          *
 *   This program is free software: you can redistribute it and/or modify   *
 *   it under the terms of the GNU General Public License as published by   *
 *   the Free Software Foundation, either version 3 of the License, or      *
 *   (at your option) any later version.                                    *
 *                                                                          *
 *   This program is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU General Public License for more details.                           *
 *                                                                          *
 *   To view a copy of the GNU General Public License, go to the following  *
 *   location: <http://www.gnu.org/licenses/>.                              *
 ****************************************************************************/

#include "../../../inc/MarlinConfigPre.h"

#if ENABLED(DGUS_LCD)

// #include "../ui_api.h"

// To implement a new UI, complete the functions below and
// read or update Marlin's state using the methods in the
// ExtUI methods in "../ui_api.h"
//
// Although it may be possible to access other state
// variables from Marlin, using the API here possibly
// helps ensure future compatibility.

// We borrow this namespace for the moment as we can convenitently hook into stuff here
namespace ExtUI {


 // void onMediaInserted() {};
//  void onMediaError() {};
//  void onMediaRemoved() {};
  void onPlayTone(const uint16_t frequency, const uint16_t duration) {}
  void onPrintTimerStarted() {}
  void onPrintTimerPaused() {}
  void onPrintTimerStopped() {}
  void onFilamentRunout() {}
//  void onStatusChanged(const char * const msg) {}
  void onFactoryReset() {}
  void onLoadSettings() {}
  void onStoreSettings() {}
}

#endif // DGUS_LCD
