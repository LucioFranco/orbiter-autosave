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

#include "StateSnapDlg.h"
#include "StateSnap.h"
#include "resource.h"
#include "Orbitersdk.h"
#include <commctrl.h>
#include <cstdio>

// Dialog handle from StateSnap.cpp
extern HWND g_hDlg;

// Store module pointer for dialog callbacks
static StateSnap* g_dlgModule = nullptr;

// Timer ID for periodic updates
#define IDT_UPDATE_TIMER 1

void UpdateStateSnapDialog(HWND hDlg, StateSnap* module)
{
    if (!hDlg || !module) return;

    // Update status text
    SetDlgItemText(hDlg, IDC_STATUS_VALUE,
        module->IsPaused() ? "Paused" : "Active");

    // Update pause/resume button text
    SetDlgItemText(hDlg, IDC_PAUSE_RESUME,
        module->IsPaused() ? "Resume" : "Pause");

    // Update next save countdown
    if (module->IsPaused()) {
        SetDlgItemText(hDlg, IDC_NEXTSAVE_VALUE, "--:--");
    } else {
        int seconds = module->GetSecondsUntilNextSave();
        if (seconds >= 0) {
            int minutes = seconds / 60;
            int secs = seconds % 60;
            char buf[16];
            sprintf(buf, "%d:%02d", minutes, secs);
            SetDlgItemText(hDlg, IDC_NEXTSAVE_VALUE, buf);
        } else {
            SetDlgItemText(hDlg, IDC_NEXTSAVE_VALUE, "Soon...");
        }
    }
}

INT_PTR CALLBACK StateSnapDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_INITDIALOG:
        {
            g_dlgModule = reinterpret_cast<StateSnap*>(lParam);
            
            // Initialize the spin control
            HWND hSpin = GetDlgItem(hDlg, IDC_INTERVAL_SPIN);
            SendMessage(hSpin, UDM_SETRANGE32, 1, 60);  // 1-60 minutes
            SendMessage(hSpin, UDM_SETPOS32, 0, g_dlgModule ? g_dlgModule->GetInterval() : 10);
            
            // Initialize notification checkbox
            CheckDlgButton(hDlg, IDC_SHOW_NOTIFICATION, 
                g_dlgModule && g_dlgModule->GetShowNotification() ? BST_CHECKED : BST_UNCHECKED);
            
            UpdateStateSnapDialog(hDlg, g_dlgModule);
            // Set up a timer to update the countdown every second
            SetTimer(hDlg, IDT_UPDATE_TIMER, 1000, NULL);
        }
        return TRUE;

    case WM_TIMER:
        if (wParam == IDT_UPDATE_TIMER) {
            UpdateStateSnapDialog(hDlg, g_dlgModule);
            return TRUE;
        }
        break;

    case WM_NOTIFY:
        {
            NMHDR* pNmhdr = reinterpret_cast<NMHDR*>(lParam);
            if (pNmhdr->idFrom == IDC_INTERVAL_SPIN && pNmhdr->code == UDN_DELTAPOS) {
                NMUPDOWN* pUpDown = reinterpret_cast<NMUPDOWN*>(lParam);
                int newVal = pUpDown->iPos + pUpDown->iDelta;
                if (newVal >= 1 && newVal <= 60 && g_dlgModule) {
                    g_dlgModule->SetInterval(newVal);
                    UpdateStateSnapDialog(hDlg, g_dlgModule);
                }
                return TRUE;
            }
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_INTERVAL_EDIT:
            if (HIWORD(wParam) == EN_KILLFOCUS && g_dlgModule) {
                // User finished editing the interval
                BOOL success;
                int val = GetDlgItemInt(hDlg, IDC_INTERVAL_EDIT, &success, FALSE);
                if (success && val >= 1 && val <= 60) {
                    g_dlgModule->SetInterval(val);
                    UpdateStateSnapDialog(hDlg, g_dlgModule);
                } else {
                    // Reset to current value if invalid
                    SetDlgItemInt(hDlg, IDC_INTERVAL_EDIT, g_dlgModule->GetInterval(), FALSE);
                }
            }
            return TRUE;

        case IDC_PAUSE_RESUME:
            if (g_dlgModule) {
                g_dlgModule->SetPaused(!g_dlgModule->IsPaused());
                UpdateStateSnapDialog(hDlg, g_dlgModule);
            }
            return TRUE;

        case IDC_SAVE_NOW:
            if (g_dlgModule) {
                g_dlgModule->SaveNow();
                UpdateStateSnapDialog(hDlg, g_dlgModule);
            }
            return TRUE;

        case IDC_SHOW_NOTIFICATION:
            if (g_dlgModule) {
                bool checked = (IsDlgButtonChecked(hDlg, IDC_SHOW_NOTIFICATION) == BST_CHECKED);
                g_dlgModule->SetShowNotification(checked);
            }
            return TRUE;

        case IDCANCEL:
            KillTimer(hDlg, IDT_UPDATE_TIMER);
            oapiCloseDialog(hDlg);
            return TRUE;
        }
        break;

    case WM_DESTROY:
        KillTimer(hDlg, IDT_UPDATE_TIMER);
        g_dlgModule = nullptr;
        g_hDlg = NULL;  // Reset so dialog can be reopened
        return TRUE;
    }

    return oapiDefDialogProc(hDlg, uMsg, wParam, lParam);
}
