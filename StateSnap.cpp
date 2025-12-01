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

#define ORBITER_MODULE
#include "StateSnap.h"
#include "StateSnapDlg.h"
#include "resource.h"
#include "Orbitersdk.h"
#include <cstdio>
#include <cmath>
#include <ctime>
#include <direct.h>

// Global variables
static StateSnap* g_module = nullptr;
static DWORD g_dwCmd = 0;
static HWND g_hDlg = NULL;

// Config file name
static const char* CFGFILE = "StateSnap.cfg";

// Notification display duration in seconds
static const double NOTIFICATION_DURATION = 3.0;

// Custom command callback to open dialog
void OpenDlgClbk(void* context)
{
    if (!g_module) return;

    if (g_hDlg) {
        // Dialog already open, bring to front
        SetForegroundWindow(g_hDlg);
    } else {
        g_hDlg = oapiOpenDialogEx(g_module->GetHInstance(), IDD_STATESNAP,
            StateSnapDlgProc, DLG_ALLOWMULTI, g_module);
    }
}

// Convert MJD to date/time components
static void MJDToDateTime(double mjd, int& year, int& month, int& day, int& hour, int& min, int& sec) {
    // Convert MJD to Julian Date
    double jd = mjd + 2400000.5;
    
    // Algorithm from "Astronomical Algorithms" by Jean Meeus
    int Z = static_cast<int>(jd + 0.5);
    double F = (jd + 0.5) - Z;
    
    int A;
    if (Z < 2299161) {
        A = Z;
    } else {
        int alpha = static_cast<int>((Z - 1867216.25) / 36524.25);
        A = Z + 1 + alpha - alpha / 4;
    }
    
    int B = A + 1524;
    int C = static_cast<int>((B - 122.1) / 365.25);
    int D = static_cast<int>(365.25 * C);
    int E = static_cast<int>((B - D) / 30.6001);
    
    day = B - D - static_cast<int>(30.6001 * E);
    
    if (E < 14) {
        month = E - 1;
    } else {
        month = E - 13;
    }
    
    if (month > 2) {
        year = C - 4716;
    } else {
        year = C - 4715;
    }
    
    // Time of day
    double dayFraction = F;
    hour = static_cast<int>(dayFraction * 24);
    dayFraction = dayFraction * 24 - hour;
    min = static_cast<int>(dayFraction * 60);
    dayFraction = dayFraction * 60 - min;
    sec = static_cast<int>(dayFraction * 60);
}

// Month abbreviations
static const char* MONTH_NAMES[] = {
    "", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

// Ensure directory exists (creates if needed)
static void EnsureDirectoryExists(const char* path) {
    char tmp[512];
    snprintf(tmp, sizeof(tmp), "%s", path);
    
    // Create each level of the directory path
    for (char* p = tmp + 1; *p; p++) {
        if (*p == '/' || *p == '\\') {
            *p = '\0';
            _mkdir(tmp);
            *p = '/';
        }
    }
    _mkdir(tmp);
}

// Generate save filename: Autosave/<SystemDate>/<VesselName>-<SimDateTime>
static void GenerateSaveFilename(char* buffer, size_t bufferSize) {
    // Get system (real-world) date for folder name
    time_t now = time(nullptr);
    struct tm* ltm = localtime(&now);
    char folderDate[32];
    snprintf(folderDate, sizeof(folderDate), "%04d-%02d-%02d",
             1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday);
    
    // Create folder path: Scenarios/Autosave/<date>
    char folderPath[256];
    snprintf(folderPath, sizeof(folderPath), "Scenarios/Autosave/%s", folderDate);
    EnsureDirectoryExists(folderPath);
    
    // Get focus vessel name
    char vesselName[256] = "Autosave";
    OBJHANDLE hFocus = oapiGetFocusObject();
    if (hFocus) {
        oapiGetObjectName(hFocus, vesselName, sizeof(vesselName));
    }
    
    // Get simulation date/time from MJD
    double mjd = oapiGetSimMJD();
    int year, month, day, hour, min, sec;
    MJDToDateTime(mjd, year, month, day, hour, min, sec);
    
    // Clamp month to valid range
    if (month < 1) month = 1;
    if (month > 12) month = 12;
    
    // Format: Autosave/<SystemDate>/VesselName-DD-Mon-YYYY-HH.MM.SS
    snprintf(buffer, bufferSize, "Autosave/%s/%s-%02d-%s-%04d-%02d.%02d.%02d",
             folderDate, vesselName, day, MONTH_NAMES[month], year, hour, min, sec);
}

StateSnap::StateSnap(HINSTANCE hDLL) : Module(hDLL) {
    // Set defaults
    interval = 10;
    showNotification = true;
    lastSaveTime = std::chrono::steady_clock::now();
    
    // Load saved configuration
    LoadConfig();
}

void StateSnap::LoadConfig() {
    FILEHANDLE hFile = oapiOpenFile(CFGFILE, FILE_IN, ROOT);
    if (hFile) {
        int val;
        bool bval;
        if (oapiReadItem_int(hFile, (char*)"Interval", val)) {
            if (val >= 1 && val <= 60) interval = val;
        }
        if (oapiReadItem_bool(hFile, (char*)"ShowNotification", bval)) {
            showNotification = bval;
        }
        oapiCloseFile(hFile, FILE_IN);
    }
}

void StateSnap::SaveConfig() {
    FILEHANDLE hFile = oapiOpenFile(CFGFILE, FILE_OUT, ROOT);
    if (hFile) {
        oapiWriteItem_int(hFile, (char*)"Interval", interval);
        oapiWriteItem_bool(hFile, (char*)"ShowNotification", showNotification);
        oapiCloseFile(hFile, FILE_OUT);
    }
}

void StateSnap::clbkSimulationStart(oapi::Module::RenderMode mode) {
    paused = false;
    lastSaveTime = std::chrono::steady_clock::now();
    notificationEndTime = 0;
    
    // Create annotation handle for on-screen notifications
    VECTOR3 col = {0.0, 1.0, 0.5};  // Green color
    hNote = oapiCreateAnnotation(false, 0.8, col);
    oapiAnnotationSetPos(hNote, 0.02, 0.15, 0.4, 0.2);
}

void StateSnap::clbkSimulationEnd() {
    // Clean up annotation
    if (hNote) {
        oapiDelAnnotation(hNote);
        hNote = nullptr;
    }

    // Close dialog if open
    if (g_hDlg) {
        oapiCloseDialog(g_hDlg);
        g_hDlg = NULL;
    }
}

void StateSnap::clbkPostStep(double simt, double simdt, double mjd) {
    // Check if it's time to save (based on elapsed real time)
    if (!paused) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastSaveTime).count();
        if (elapsed >= interval * 60) {
            DoSave();
        }
    }
    
    // Clear notification after duration expires
    if (notificationEndTime > 0 && simt >= notificationEndTime) {
        ClearNotification();
    }
}

void StateSnap::DoSave() {
    // Generate dynamic filename
    char saveFilename[512];
    GenerateSaveFilename(saveFilename, sizeof(saveFilename));
    
    oapiSaveScenario(saveFilename, "StateSnap autosave");
    lastSaveTime = std::chrono::steady_clock::now();
    
    if (showNotification) {
        ShowSaveNotification(saveFilename);
    }
}

void StateSnap::ShowSaveNotification(const char* savedFilename) {
    if (hNote) {
        char msg[256];
        snprintf(msg, sizeof(msg), "StateSnap: Saved %s", savedFilename);
        oapiAnnotationSetText(hNote, msg);
        notificationEndTime = oapiGetSimTime() + NOTIFICATION_DURATION;
    }
}

void StateSnap::ClearNotification() {
    if (hNote) {
        oapiAnnotationSetText(hNote, (char*)"");
    }
    notificationEndTime = 0;
}

void StateSnap::SetPaused(bool pause) {
    paused = pause;
    if (!pause) {
        // When resuming, reset the save timer
        lastSaveTime = std::chrono::steady_clock::now();
    }
}

void StateSnap::SaveNow() {
    DoSave();
}

void StateSnap::SetInterval(int minutes) {
    if (minutes < 1) minutes = 1;
    if (minutes > 60) minutes = 60;
    interval = minutes;
    // Reset timer so next save uses the new interval
    lastSaveTime = std::chrono::steady_clock::now();
    // Persist setting
    SaveConfig();
}

void StateSnap::SetShowNotification(bool show) {
    showNotification = show;
    // Persist setting
    SaveConfig();
}

int StateSnap::GetSecondsUntilNextSave() const {
    if (paused) return -1;

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastSaveTime).count();
    int totalSeconds = interval * 60;
    int remaining = totalSeconds - static_cast<int>(elapsed);
    return remaining > 0 ? remaining : 0;
}

// DLL entry/exit points
DLLCLBK void InitModule(HINSTANCE hDLL)
{
    g_module = new StateSnap(hDLL);
    oapiRegisterModule(g_module);

    // Register custom command (accessible via Ctrl-F4)
    g_dwCmd = oapiRegisterCustomCmd(
        (char*)"StateSnap Control",
        (char*)"Open StateSnap autosave control dialog to pause/resume or trigger manual save.",
        OpenDlgClbk, nullptr);
}

DLLCLBK void ExitModule(HINSTANCE hModule)
{
    // Unregister custom command
    if (g_dwCmd) {
        oapiUnregisterCustomCmd(g_dwCmd);
        g_dwCmd = 0;
    }

    // Close dialog if still open
    if (g_hDlg) {
        oapiCloseDialog(g_hDlg);
        g_hDlg = NULL;
    }

    // Module is deleted by Orbiter
}
