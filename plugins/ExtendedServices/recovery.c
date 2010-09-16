/*
 * Process Hacker Extended Services - 
 *   recovery information
 * 
 * Copyright (C) 2010 wj32
 * 
 * This file is part of Process Hacker.
 * 
 * Process Hacker is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Process Hacker is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Process Hacker.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <phdk.h>
#include <windowsx.h>
#include "extsrv.h"
#include "resource.h"

typedef struct _SERVICE_RECOVERY_CONTEXT
{
    PPH_SERVICE_ITEM ServiceItem;

    BOOLEAN EnableFlagCheckBox;
    ULONG RebootAfter; // in ms
    PPH_STRING RebootMessage;
} SERVICE_RECOVERY_CONTEXT, *PSERVICE_RECOVERY_CONTEXT;

#define SIP(String, Integer) { (String), (PVOID)(Integer) }

static PH_KEY_VALUE_PAIR ServiceActionPairs[] =
{
    SIP(L"Take no action", SC_ACTION_NONE),
    SIP(L"Restart the service", SC_ACTION_RESTART),
    SIP(L"Run a program", SC_ACTION_RUN_COMMAND),
    SIP(L"Restart the computer", SC_ACTION_REBOOT)
};

INT_PTR CALLBACK RestartComputerDlgProc(      
    __in HWND hwndDlg,
    __in UINT uMsg,
    __in WPARAM wParam,
    __in LPARAM lParam
    );

VOID EspAddServiceActionStrings(
    __in HWND ComboBoxHandle
    )
{
    ULONG i;

    for (i = 0; i < sizeof(ServiceActionPairs) / sizeof(PH_KEY_VALUE_PAIR); i++)
        ComboBox_AddString(ComboBoxHandle, (PWSTR)ServiceActionPairs[i].Key);

    ComboBox_SelectString(ComboBoxHandle, -1, (PWSTR)ServiceActionPairs[i].Key);
}

SC_ACTION_TYPE EspStringToServiceAction(
    __in PWSTR String
    )
{
    ULONG integer;

    if (PhFindIntegerSiKeyValuePairs(ServiceActionPairs, sizeof(ServiceActionPairs), String, &integer))
        return integer;
    else
        return 0;
}

PWSTR EspServiceActionToString(
    __in SC_ACTION_TYPE ActionType
    )
{
    PWSTR string;

    if (PhFindStringSiKeyValuePairs(ServiceActionPairs, sizeof(ServiceActionPairs), ActionType, &string))
        return string;
    else
        return NULL;
}

static SC_ACTION_TYPE ComboBoxToServiceAction(
    __in HWND ComboBoxHandle
    )
{
    SC_ACTION_TYPE actionType;
    PPH_STRING string;

    string = PhGetComboBoxString(ComboBoxHandle, ComboBox_GetCurSel(ComboBoxHandle));

    if (!string)
        return SC_ACTION_NONE;

    actionType = EspStringToServiceAction(string->Buffer);
    PhDereferenceObject(string);

    return actionType;
}

static VOID ServiceActionToComboBox(
    __in HWND ComboBoxHandle,
    __in SC_ACTION_TYPE ActionType
    )
{
    PWSTR string;

    if (string = EspServiceActionToString(ActionType))
        ComboBox_SelectString(ComboBoxHandle, -1, string);
    else
        ComboBox_SelectString(ComboBoxHandle, -1, (PWSTR)ServiceActionPairs[0].Key);
}

static VOID EspFixControls(
    __in HWND hwndDlg,
    __in PSERVICE_RECOVERY_CONTEXT Context
    )
{
    SC_ACTION_TYPE action1;
    SC_ACTION_TYPE action2;
    SC_ACTION_TYPE actionS;
    BOOLEAN enableRestart;
    BOOLEAN enableCommand;
    BOOLEAN enableReboot;

    action1 = ComboBoxToServiceAction(GetDlgItem(hwndDlg, IDC_FIRSTFAILURE));
    action2 = ComboBoxToServiceAction(GetDlgItem(hwndDlg, IDC_SECONDFAILURE));
    actionS = ComboBoxToServiceAction(GetDlgItem(hwndDlg, IDC_SUBSEQUENTFAILURES));

    EnableWindow(GetDlgItem(hwndDlg, IDC_ENABLEFORERRORSTOPS), Context->EnableFlagCheckBox);

    enableRestart = action1 == SC_ACTION_RESTART || action2 == SC_ACTION_RESTART || actionS == SC_ACTION_RESTART;
    enableCommand = action1 == SC_ACTION_RUN_COMMAND || action2 == SC_ACTION_RUN_COMMAND || actionS == SC_ACTION_RUN_COMMAND;
    enableReboot = action1 == SC_ACTION_REBOOT || action2 == SC_ACTION_REBOOT || actionS == SC_ACTION_REBOOT;

    EnableWindow(GetDlgItem(hwndDlg, IDC_RESTARTSERVICEAFTER_LABEL), enableRestart);
    EnableWindow(GetDlgItem(hwndDlg, IDC_RESTARTSERVICEAFTER), enableRestart);
    EnableWindow(GetDlgItem(hwndDlg, IDC_RESTARTSERVICEAFTER_MINUTES), enableRestart);

    EnableWindow(GetDlgItem(hwndDlg, IDC_RUNPROGRAM_GROUP), enableCommand);
    EnableWindow(GetDlgItem(hwndDlg, IDC_RUNPROGRAM_LABEL), enableCommand);
    EnableWindow(GetDlgItem(hwndDlg, IDC_RUNPROGRAM), enableCommand);
    EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSE), enableCommand);
    EnableWindow(GetDlgItem(hwndDlg, IDC_RUNPROGRAM_INFO), enableCommand);

    EnableWindow(GetDlgItem(hwndDlg, IDC_RESTARTCOMPUTEROPTIONS), enableReboot);
}

NTSTATUS EspLoadRecoveryInfo(
    __in HWND hwndDlg,
    __in PSERVICE_RECOVERY_CONTEXT Context
    )
{
    SC_HANDLE serviceHandle;
    LPSERVICE_FAILURE_ACTIONS failureActions;
    SERVICE_FAILURE_ACTIONS_FLAG failureActionsFlag;
    ULONG returnLength;
    ULONG i;

    if (!(serviceHandle = PhOpenService(Context->ServiceItem->Name->Buffer, SERVICE_QUERY_CONFIG)))
        return NTSTATUS_FROM_WIN32(GetLastError());

    if (!(failureActions = PhQueryServiceVariableSize(serviceHandle, SERVICE_CONFIG_FAILURE_ACTIONS)))
    {
        CloseServiceHandle(serviceHandle);
        return NTSTATUS_FROM_WIN32(GetLastError());
    }

    // Failure action types

    ServiceActionToComboBox(GetDlgItem(hwndDlg, IDC_FIRSTFAILURE),
        failureActions->cActions >= 1 ? failureActions->lpsaActions[0].Type : SC_ACTION_NONE);
    ServiceActionToComboBox(GetDlgItem(hwndDlg, IDC_SECONDFAILURE),
        failureActions->cActions >= 2 ? failureActions->lpsaActions[1].Type : SC_ACTION_NONE);
    ServiceActionToComboBox(GetDlgItem(hwndDlg, IDC_SUBSEQUENTFAILURES),
        failureActions->cActions >= 3 ? failureActions->lpsaActions[2].Type : SC_ACTION_NONE);

    // Reset fail count after

    SetDlgItemInt(hwndDlg, IDC_RESETFAILCOUNT, failureActions->dwResetPeriod / (60 * 60 * 24), FALSE); // s to days

    // Restart service after

    SetDlgItemText(hwndDlg, IDC_RESTARTSERVICEAFTER, L"1");

    for (i = 0; i < failureActions->cActions; i++)
    {
        if (failureActions->lpsaActions[i].Type == SC_ACTION_RESTART)
        {
            if (failureActions->lpsaActions[i].Delay != 0)
            {
                SetDlgItemInt(hwndDlg, IDC_RESTARTSERVICEAFTER,
                    failureActions->lpsaActions[i].Delay / (1000 * 60), FALSE); // ms to min
            }

            break;
        }
    }

    // Enable actions for stops with errors

    if (WindowsVersion >= WINDOWS_VISTA && QueryServiceConfig2(
        serviceHandle,
        SERVICE_CONFIG_FAILURE_ACTIONS_FLAG,
        (BYTE *)&failureActionsFlag,
        sizeof(SERVICE_FAILURE_ACTIONS_FLAG),
        &returnLength
        ))
    {
        Button_SetCheck(GetDlgItem(hwndDlg, IDC_ENABLEFORERRORSTOPS),
            failureActionsFlag.fFailureActionsOnNonCrashFailures ? BST_CHECKED : BST_UNCHECKED);
        Context->EnableFlagCheckBox = TRUE;
    }
    else
    {
        Context->EnableFlagCheckBox = FALSE;
    }

    // Restart computer options

    Context->RebootAfter = 1 * 1000 * 60;

    for (i = 0; i < failureActions->cActions; i++)
    {
        if (failureActions->lpsaActions[i].Type == SC_ACTION_REBOOT)
        {
            if (failureActions->lpsaActions[i].Delay != 0)
                Context->RebootAfter = failureActions->lpsaActions[i].Delay;

            break;
        }
    }

    if (failureActions->lpRebootMsg && failureActions->lpRebootMsg[0] != 0)
        PhSwapReference2(&Context->RebootMessage, PhCreateString(failureActions->lpRebootMsg));
    else
        PhSwapReference2(&Context->RebootMessage, NULL);

    // Run program

    SetDlgItemText(hwndDlg, IDC_RUNPROGRAM, failureActions->lpCommand);

    PhFree(failureActions);
    CloseServiceHandle(serviceHandle);

    return STATUS_SUCCESS;
}

INT_PTR CALLBACK EspServiceRecoveryDlgProc(      
    __in HWND hwndDlg,
    __in UINT uMsg,
    __in WPARAM wParam,
    __in LPARAM lParam
    )
{
    PSERVICE_RECOVERY_CONTEXT context;

    if (uMsg == WM_INITDIALOG)
    {
        context = PhAllocate(sizeof(SERVICE_RECOVERY_CONTEXT));
        memset(context, 0, sizeof(SERVICE_RECOVERY_CONTEXT));

        SetProp(hwndDlg, L"Context", (HANDLE)context);
    }
    else
    {
        context = (PSERVICE_RECOVERY_CONTEXT)GetProp(hwndDlg, L"Context");

        if (uMsg == WM_DESTROY)
            RemoveProp(hwndDlg, L"Context");
    }

    if (!context)
        return FALSE;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            NTSTATUS status;
            LPPROPSHEETPAGE propSheetPage = (LPPROPSHEETPAGE)lParam;
            PPH_SERVICE_ITEM serviceItem = (PPH_SERVICE_ITEM)propSheetPage->lParam;

            context->ServiceItem = serviceItem;

            EspAddServiceActionStrings(GetDlgItem(hwndDlg, IDC_FIRSTFAILURE));
            EspAddServiceActionStrings(GetDlgItem(hwndDlg, IDC_SECONDFAILURE));
            EspAddServiceActionStrings(GetDlgItem(hwndDlg, IDC_SUBSEQUENTFAILURES));

            status = EspLoadRecoveryInfo(hwndDlg, context);

            if (!NT_SUCCESS(status))
                PhShowStatus(hwndDlg, L"Unable to query recovery information", status, 0);

            EspFixControls(hwndDlg, context);
        }
        break;
    case WM_DESTROY:
        {
            PhSwapReference2(&context->RebootMessage, NULL);
            PhFree(context);
        }
        break;
    case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
            case IDC_FIRSTFAILURE:
            case IDC_SECONDFAILURE:
            case IDC_SUBSEQUENTFAILURES:
                {
                    if (HIWORD(wParam) == CBN_SELCHANGE)
                    {
                        EspFixControls(hwndDlg, context);
                    }
                }
                break;
            case IDC_RESTARTCOMPUTEROPTIONS:
                {
                    DialogBoxParam(
                        PluginInstance->DllBase,
                        MAKEINTRESOURCE(IDD_RESTARTCOMP),
                        hwndDlg,
                        RestartComputerDlgProc,
                        (LPARAM)context
                        );
                }
                break;
            }
        }
        break;
    }

    return FALSE;
}

static INT_PTR CALLBACK RestartComputerDlgProc(      
    __in HWND hwndDlg,
    __in UINT uMsg,
    __in WPARAM wParam,
    __in LPARAM lParam
    )
{
    PSERVICE_RECOVERY_CONTEXT context;

    if (uMsg == WM_INITDIALOG)
    {
        context = (PSERVICE_RECOVERY_CONTEXT)lParam;
        SetProp(hwndDlg, L"Context", (HANDLE)context);
    }
    else
    {
        context = (PSERVICE_RECOVERY_CONTEXT)GetProp(hwndDlg, L"Context");

        if (uMsg == WM_DESTROY)
            RemoveProp(hwndDlg, L"Context");
    }

    if (!context)
        return FALSE;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            SetDlgItemInt(hwndDlg, IDC_RESTARTCOMPAFTER, context->RebootAfter / (1000 * 60), FALSE); // ms to min
            Button_SetCheck(GetDlgItem(hwndDlg, IDC_ENABLERESTARTMESSAGE), context->RebootMessage ? BST_CHECKED : BST_UNCHECKED);
            SetDlgItemText(hwndDlg, IDC_RESTARTMESSAGE, PhGetString(context->RebootMessage));
        }
        break;
    case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
            case IDCANCEL:
                EndDialog(hwndDlg, IDCANCEL);
                break;
            case IDOK:
                {
                    context->RebootAfter = GetDlgItemInt(hwndDlg, IDC_RESTARTCOMPAFTER, NULL, FALSE) * 1000 * 60;

                    if (Button_GetCheck(GetDlgItem(hwndDlg, IDC_ENABLERESTARTMESSAGE)) == BST_CHECKED)
                        PhSwapReference2(&context->RebootMessage, PhGetWindowText(GetDlgItem(hwndDlg, IDC_RESTARTMESSAGE)));
                    else
                        PhSwapReference2(&context->RebootMessage, NULL);

                    EndDialog(hwndDlg, IDOK);
                }
                break;
            case IDC_USEDEFAULTMESSAGE:
                {
                    PPH_STRING message;
                    PWSTR computerName;
                    ULONG bufferSize;
                    BOOLEAN allocated = TRUE;

                    // Get the computer name.

                    bufferSize = 64;
                    computerName = PhAllocate((bufferSize + 1) * sizeof(WCHAR));

                    if (!GetComputerName(computerName, &bufferSize))
                    {
                        PhFree(computerName);
                        computerName = PhAllocate((bufferSize + 1) * sizeof(WCHAR));

                        if (!GetComputerName(computerName, &bufferSize))
                        {
                            PhFree(computerName);
                            computerName = L"(unknown)";
                            allocated = FALSE;
                        }
                    }

                    message = PhFormatString(
                        L"Your computer is connected to the computer named %s. "
                        L"The %s service on %s has ended unexpectedly. "
                        L"%s will restart automatically, and then you can reestablish the connection.",
                        computerName,
                        context->ServiceItem->Name->Buffer,
                        computerName,
                        computerName
                        );
                    SetDlgItemText(hwndDlg, IDC_RESTARTMESSAGE, message->Buffer);
                    PhDereferenceObject(message);

                    if (allocated)
                        PhFree(computerName);

                    Button_SetCheck(GetDlgItem(hwndDlg, IDC_ENABLERESTARTMESSAGE), BST_CHECKED);
                }
                break;
            case IDC_RESTARTMESSAGE:
                {
                    if (HIWORD(wParam) == EN_CHANGE)
                    {
                        Button_SetCheck(GetDlgItem(hwndDlg, IDC_ENABLERESTARTMESSAGE),
                            GetWindowTextLength(GetDlgItem(hwndDlg, IDC_RESTARTMESSAGE)) != 0 ? BST_CHECKED : BST_UNCHECKED);
                    }
                }
                break;
            }
        }
        break;
    }

    return FALSE;
}
