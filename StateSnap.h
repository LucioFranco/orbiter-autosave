/*
 *     StateSnap - A module or Orbiter to periodically make scenario snapshots.
 *     Copyright (C) 2022  Thymo van Beers
 *
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License along
 *     with this program; if not, write to the Free Software Foundation, Inc.,
 *     51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef STATESNAP_STATESNAP_H
#define STATESNAP_STATESNAP_H

#include "Orbitersdk.h"
#include <chrono>

class StateSnap : public oapi::Module {
public:
    StateSnap(HINSTANCE hDLL);
    ~StateSnap() = default;

    void clbkSimulationStart(RenderMode mode) override;
    void clbkSimulationEnd() override;
    void clbkPostStep(double simt, double simdt, double mjd) override;

    // Pause/resume control
    void SetPaused(bool pause);
    bool IsPaused() const { return paused; }
    void SaveNow();

    // Accessors for dialog
    HINSTANCE GetHInstance() const { return GetModule(); }
    int GetInterval() const { return interval; }
    void SetInterval(int minutes);
    int GetSecondsUntilNextSave() const;

    // Notification control
    void SetShowNotification(bool show);
    bool GetShowNotification() const { return showNotification; }

private:
    void DoSave();
    void LoadConfig();
    void SaveConfig();
    void ShowSaveNotification(const char* savedFilename);
    void ClearNotification();

    int interval; // save interval in minutes
    bool paused = false;

    // Track time for countdown display
    std::chrono::steady_clock::time_point lastSaveTime;

    // On-screen notification
    bool showNotification = true;
    NOTEHANDLE hNote = nullptr;
    double notificationEndTime = 0;
};

#endif //STATESNAP_STATESNAP_H
