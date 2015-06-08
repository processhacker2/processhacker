#ifndef PHAPP_H
#define PHAPP_H

#ifdef PHAPP_EXPORT
#define PHAPPAPI __declspec(dllexport)
#else
#define PHAPPAPI
#endif

#include <phgui.h>
#include <treenew.h>
#include <graph.h>
#include <circbuf.h>
#include <phnet.h>
#include <providers.h>
#include <colmgr.h>
#include <uimodels.h>
#include "../resource.h"

#define KPH_ERROR_MESSAGE (L"KProcessHacker does not support your operating system " \
            L"or could not be loaded. Make sure Process Hacker is running " \
            L"with administrative privileges.")

#ifndef PH_MAIN_PRIVATE

extern PPH_STRING PhApplicationDirectory;
extern PPH_STRING PhApplicationFileName;
extern HFONT PhApplicationFont;
extern PPH_STRING PhCurrentUserName;
extern HINSTANCE PhInstanceHandle;
extern PPH_STRING PhLocalSystemName;
extern BOOLEAN PhPluginsEnabled;
extern PPH_STRING PhProcDbFileName;
extern PPH_STRING PhSettingsFileName;
extern PH_INTEGER_PAIR PhSmallIconSize;
extern PH_STARTUP_PARAMETERS PhStartupParameters;

extern PH_PROVIDER_THREAD PhPrimaryProviderThread;
extern PH_PROVIDER_THREAD PhSecondaryProviderThread;

extern COLORREF PhSysWindowColor;

#endif

// main

typedef struct _PH_STARTUP_PARAMETERS
{
    union
    {
        struct
        {
            ULONG NoKph : 1;
            ULONG NoSettings : 1;
            ULONG NoPlugins : 1;
            ULONG ShowHidden : 1;
            ULONG ShowVisible : 1;
            ULONG ShowOptions : 1;
            ULONG NewInstance : 1;
            ULONG Elevate : 1;
            ULONG Silent : 1;
            ULONG CommandMode : 1;
            ULONG PhSvc : 1;
            ULONG InstallKph : 1;
            ULONG UninstallKph : 1;
            ULONG Debug : 1;
            ULONG Help : 1;
            ULONG Spare : 17;
        };
        ULONG Flags;
    };

    PPH_STRING SettingsFileName;

    HWND WindowHandle;
    POINT Point;

    PPH_STRING CommandType;
    PPH_STRING CommandObject;
    PPH_STRING CommandAction;
    PPH_STRING CommandValue;

    PPH_STRING RunAsServiceMode;

    ULONG SelectPid;
    ULONG PriorityClass;

    PPH_LIST PluginParameters;
    PPH_STRING SelectTab;
} PH_STARTUP_PARAMETERS, *PPH_STARTUP_PARAMETERS;

PHAPPAPI
VOID PhRegisterDialog(
    _In_ HWND DialogWindowHandle
    );

PHAPPAPI
VOID PhUnregisterDialog(
    _In_ HWND DialogWindowHandle
    );

typedef BOOLEAN (NTAPI *PPH_MESSAGE_LOOP_FILTER)(
    _In_ PMSG Message,
    _In_ PVOID Context
    );

typedef struct _PH_MESSAGE_LOOP_FILTER_ENTRY
{
    PPH_MESSAGE_LOOP_FILTER Filter;
    PVOID Context;
} PH_MESSAGE_LOOP_FILTER_ENTRY, *PPH_MESSAGE_LOOP_FILTER_ENTRY;

PHAPPAPI
struct _PH_MESSAGE_LOOP_FILTER_ENTRY *PhRegisterMessageLoopFilter(
    _In_ PPH_MESSAGE_LOOP_FILTER Filter,
    _In_opt_ PVOID Context
    );

PHAPPAPI
VOID PhUnregisterMessageLoopFilter(
    _In_ struct _PH_MESSAGE_LOOP_FILTER_ENTRY *FilterEntry
    );

VOID PhInitializeFont(
    _In_ HWND hWnd
    );

// appsup

extern GUID XP_CONTEXT_GUID;
extern GUID VISTA_CONTEXT_GUID;
extern GUID WIN7_CONTEXT_GUID;
extern GUID WIN8_CONTEXT_GUID;
extern GUID WINBLUE_CONTEXT_GUID;

typedef struct PACKAGE_ID PACKAGE_ID;

PHAPPAPI
BOOLEAN PhGetProcessIsSuspended(
    _In_ PSYSTEM_PROCESS_INFORMATION Process
    );

NTSTATUS PhGetProcessSwitchContext(
    _In_ HANDLE ProcessHandle,
    _Out_ PGUID Guid
    );

PPH_STRING PhGetProcessPackageFullName(
    _In_ HANDLE ProcessHandle
    );

PACKAGE_ID *PhPackageIdFromFullName(
    _In_ PWSTR PackageFullName
    );

PPH_STRING PhGetPackagePath(
    _In_ PACKAGE_ID *PackageId
    );

VOID PhEnumChildWindows(
    _In_opt_ HWND hWnd,
    _In_ ULONG Limit,
    _In_ WNDENUMPROC Callback,
    _In_ LPARAM lParam
    );

typedef enum _PH_KNOWN_PROCESS_TYPE
{
    UnknownProcessType,
    SystemProcessType, // ntoskrnl/ntkrnlpa/...
    SessionManagerProcessType, // smss
    WindowsSubsystemProcessType, // csrss
    WindowsStartupProcessType, // wininit
    ServiceControlManagerProcessType, // services
    LocalSecurityAuthorityProcessType, // lsass
    LocalSessionManagerProcessType, // lsm
    WindowsLogonProcessType, // winlogon
    ServiceHostProcessType, // svchost
    RunDllAsAppProcessType, // rundll32
    ComSurrogateProcessType, // dllhost
    TaskHostProcessType, // taskeng, taskhost, taskhostex
    ExplorerProcessType, // explorer
    UmdfHostProcessType, // wudfhost
    MaximumProcessType,
    KnownProcessTypeMask = 0xffff,

    KnownProcessWow64 = 0x20000
} PH_KNOWN_PROCESS_TYPE;

PHAPPAPI
NTSTATUS PhGetProcessKnownType(
    _In_ HANDLE ProcessHandle,
    _Out_ PH_KNOWN_PROCESS_TYPE *KnownProcessType
    );

typedef union _PH_KNOWN_PROCESS_COMMAND_LINE
{
    struct
    {
        PPH_STRING GroupName;
    } ServiceHost;
    struct
    {
        PPH_STRING FileName;
        PPH_STRING ProcedureName;
    } RunDllAsApp;
    struct
    {
        GUID Guid;
        PPH_STRING Name; // optional
        PPH_STRING FileName; // optional
    } ComSurrogate;
} PH_KNOWN_PROCESS_COMMAND_LINE, *PPH_KNOWN_PROCESS_COMMAND_LINE;

PHAPPAPI
BOOLEAN PhaGetProcessKnownCommandLine(
    _In_ PPH_STRING CommandLine,
    _In_ PH_KNOWN_PROCESS_TYPE KnownProcessType,
    _Out_ PPH_KNOWN_PROCESS_COMMAND_LINE KnownCommandLine
    );

PPH_STRING PhEscapeStringForDelimiter(
    _In_ PPH_STRING String,
    _In_ WCHAR Delimiter
    );

PPH_STRING PhUnescapeStringForDelimiter(
    _In_ PPH_STRING String,
    _In_ WCHAR Delimiter
    );

typedef struct mxml_node_s mxml_node_t;

PPH_STRING PhGetOpaqueXmlNodeText(
    _In_ mxml_node_t *node
    );

PHAPPAPI
VOID PhSearchOnlineString(
    _In_ HWND hWnd,
    _In_ PWSTR String
    );

PHAPPAPI
VOID PhShellExecuteUserString(
    _In_ HWND hWnd,
    _In_ PWSTR Setting,
    _In_ PWSTR String,
    _In_ BOOLEAN UseShellExecute,
    _In_opt_ PWSTR ErrorMessage
    );

PHAPPAPI
VOID PhLoadSymbolProviderOptions(
    _Inout_ PPH_SYMBOL_PROVIDER SymbolProvider
    );

PWSTR PhMakeContextAtom(
    VOID
    );

PHAPPAPI
VOID PhCopyListViewInfoTip(
    _Inout_ LPNMLVGETINFOTIP GetInfoTip,
    _In_ PPH_STRINGREF Tip
    );

PHAPPAPI
VOID PhCopyListView(
    _In_ HWND ListViewHandle
    );

PHAPPAPI
VOID PhHandleListViewNotifyForCopy(
    _In_ LPARAM lParam,
    _In_ HWND ListViewHandle
    );

#define PH_LIST_VIEW_CTRL_C_BEHAVIOR 0x1
#define PH_LIST_VIEW_CTRL_A_BEHAVIOR 0x2
#define PH_LIST_VIEW_DEFAULT_1_BEHAVIORS (PH_LIST_VIEW_CTRL_C_BEHAVIOR | PH_LIST_VIEW_CTRL_A_BEHAVIOR)

VOID PhHandleListViewNotifyBehaviors(
    _In_ LPARAM lParam,
    _In_ HWND ListViewHandle,
    _In_ ULONG Behaviors
    );

PHAPPAPI
BOOLEAN PhGetListViewContextMenuPoint(
    _In_ HWND ListViewHandle,
    _Out_ PPOINT Point
    );

HFONT PhDuplicateFontWithNewWeight(
    _In_ HFONT Font,
    _In_ LONG NewWeight
    );

PHAPPAPI
VOID PhLoadWindowPlacementFromSetting(
    _In_opt_ PWSTR PositionSettingName,
    _In_opt_ PWSTR SizeSettingName,
    _In_ HWND WindowHandle
    );

PHAPPAPI
VOID PhSaveWindowPlacementToSetting(
    _In_opt_ PWSTR PositionSettingName,
    _In_opt_ PWSTR SizeSettingName,
    _In_ HWND WindowHandle
    );

PHAPPAPI
VOID PhLoadListViewColumnsFromSetting(
    _In_ PWSTR Name,
    _In_ HWND ListViewHandle
    );

PHAPPAPI
VOID PhSaveListViewColumnsToSetting(
    _In_ PWSTR Name,
    _In_ HWND ListViewHandle
    );

PHAPPAPI
PPH_STRING PhGetPhVersion(
    VOID
    );

PHAPPAPI
VOID PhGetPhVersionNumbers(
    _Out_opt_ PULONG MajorVersion,
    _Out_opt_ PULONG MinorVersion,
    _Reserved_ PULONG Reserved,
    _Out_opt_ PULONG RevisionNumber
    );

PHAPPAPI
VOID PhWritePhTextHeader(
    _Inout_ PPH_FILE_STREAM FileStream
    );

#define PH_SHELL_APP_PROPAGATE_PARAMETERS 0x1
#define PH_SHELL_APP_PROPAGATE_PARAMETERS_IGNORE_VISIBILITY 0x2
#define PH_SHELL_APP_PROPAGATE_PARAMETERS_FORCE_SETTINGS 0x4

PHAPPAPI
BOOLEAN PhShellProcessHacker(
    _In_opt_ HWND hWnd,
    _In_opt_ PWSTR Parameters,
    _In_ ULONG ShowWindowType,
    _In_ ULONG Flags,
    _In_ ULONG AppFlags,
    _In_opt_ ULONG Timeout,
    _Out_opt_ PHANDLE ProcessHandle
    );

BOOLEAN PhShellProcessHackerEx(
    _In_opt_ HWND hWnd,
    _In_opt_ PWSTR FileName,
    _In_opt_ PWSTR Parameters,
    _In_ ULONG ShowWindowType,
    _In_ ULONG Flags,
    _In_ ULONG AppFlags,
    _In_opt_ ULONG Timeout,
    _Out_opt_ PHANDLE ProcessHandle
    );

BOOLEAN PhCreateProcessIgnoreIfeoDebugger(
    _In_ PWSTR FileName
    );

typedef struct _PH_TN_COLUMN_MENU_DATA
{
    HWND TreeNewHandle;
    PPH_TREENEW_HEADER_MOUSE_EVENT MouseEvent;
    ULONG DefaultSortColumn;
    PH_SORT_ORDER DefaultSortOrder;

    struct _PH_EMENU_ITEM *Menu;
    struct _PH_EMENU_ITEM *Selection;
    ULONG ProcessedId;
} PH_TN_COLUMN_MENU_DATA, *PPH_TN_COLUMN_MENU_DATA;

#define PH_TN_COLUMN_MENU_HIDE_COLUMN_ID ((ULONG)-1)
#define PH_TN_COLUMN_MENU_CHOOSE_COLUMNS_ID ((ULONG)-2)
#define PH_TN_COLUMN_MENU_SIZE_COLUMN_TO_FIT_ID ((ULONG)-3)
#define PH_TN_COLUMN_MENU_SIZE_ALL_COLUMNS_TO_FIT_ID ((ULONG)-4)

PHAPPAPI
VOID PhInitializeTreeNewColumnMenu(
    _Inout_ PPH_TN_COLUMN_MENU_DATA Data
    );

#define PH_TN_COLUMN_MENU_NO_VISIBILITY 0x1

VOID PhInitializeTreeNewColumnMenuEx(
    _Inout_ PPH_TN_COLUMN_MENU_DATA Data,
    _In_ ULONG Flags
    );

PHAPPAPI
BOOLEAN PhHandleTreeNewColumnMenu(
    _Inout_ PPH_TN_COLUMN_MENU_DATA Data
    );

PHAPPAPI
VOID PhDeleteTreeNewColumnMenu(
    _In_ PPH_TN_COLUMN_MENU_DATA Data
    );

typedef struct _PH_TN_FILTER_SUPPORT
{
    PPH_LIST FilterList;
    HWND TreeNewHandle;
    PPH_LIST NodeList;
} PH_TN_FILTER_SUPPORT, *PPH_TN_FILTER_SUPPORT;

typedef BOOLEAN (NTAPI *PPH_TN_FILTER_FUNCTION)(
    _In_ PPH_TREENEW_NODE Node,
    _In_opt_ PVOID Context
    );

typedef struct _PH_TN_FILTER_ENTRY
{
    PPH_TN_FILTER_FUNCTION Filter;
    PVOID Context;
} PH_TN_FILTER_ENTRY, *PPH_TN_FILTER_ENTRY;

PHAPPAPI
VOID PhInitializeTreeNewFilterSupport(
    _Out_ PPH_TN_FILTER_SUPPORT Support,
    _In_ HWND TreeNewHandle,
    _In_ PPH_LIST NodeList
    );

PHAPPAPI
VOID PhDeleteTreeNewFilterSupport(
    _In_ PPH_TN_FILTER_SUPPORT Support
    );

PHAPPAPI
PPH_TN_FILTER_ENTRY PhAddTreeNewFilter(
    _In_ PPH_TN_FILTER_SUPPORT Support,
    _In_ PPH_TN_FILTER_FUNCTION Filter,
    _In_opt_ PVOID Context
    );

PHAPPAPI
VOID PhRemoveTreeNewFilter(
    _In_ PPH_TN_FILTER_SUPPORT Support,
    _In_ PPH_TN_FILTER_ENTRY Entry
    );

PHAPPAPI
BOOLEAN PhApplyTreeNewFiltersToNode(
    _In_ PPH_TN_FILTER_SUPPORT Support,
    _In_ PPH_TREENEW_NODE Node
    );

PHAPPAPI
VOID PhApplyTreeNewFilters(
    _In_ PPH_TN_FILTER_SUPPORT Support
    );

typedef struct _PH_COPY_CELL_CONTEXT
{
    HWND TreeNewHandle;
    ULONG Id; // column ID
    PPH_STRING MenuItemText;
} PH_COPY_CELL_CONTEXT, *PPH_COPY_CELL_CONTEXT;

BOOLEAN PhInsertCopyCellEMenuItem(
    _In_ struct _PH_EMENU_ITEM *Menu,
    _In_ ULONG InsertAfterId,
    _In_ HWND TreeNewHandle,
    _In_ PPH_TREENEW_COLUMN Column
    );

BOOLEAN PhHandleCopyCellEMenuItem(
    _In_ struct _PH_EMENU_ITEM *SelectedItem
    );

BOOLEAN PhShellOpenKey2(
    _In_ HWND hWnd,
    _In_ PPH_STRING KeyName
    );

PPH_STRING PhGetServiceRelevantFileName(
    _In_ PPH_STRINGREF ServiceName,
    _In_ SC_HANDLE ServiceHandle
    );

#define PH_LOAD_SHARED_IMAGE(Name, Type) LoadImage(PhInstanceHandle, (Name), (Type), 0, 0, LR_SHARED)

FORCEINLINE PVOID PhpGenericPropertyPageHeader(
    _In_ HWND hwndDlg,
    _In_ UINT uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _In_ PWSTR ContextName
    )
{
    PVOID context;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            LPPROPSHEETPAGE propSheetPage = (LPPROPSHEETPAGE)lParam;

            context = (PVOID)propSheetPage->lParam;
            SetProp(hwndDlg, ContextName, (HANDLE)context);
        }
        break;
    case WM_DESTROY:
        {
            context = (PVOID)GetProp(hwndDlg, ContextName);
            RemoveProp(hwndDlg, ContextName);
        }
        break;
    default:
        {
            context = (PVOID)GetProp(hwndDlg, ContextName);
        }
        break;
    }

    return context;
}

// mainwnd

#define PH_MAINWND_CLASSNAME L"ProcessHacker"

#ifndef PH_MAINWND_PRIVATE
extern HWND PhMainWndHandle;
extern BOOLEAN PhMainWndExiting;
#endif

#define WM_PH_FIRST (WM_APP + 99)
#define WM_PH_ACTIVATE (WM_APP + 99)
#define PH_ACTIVATE_REPLY 0x1119

#define WM_PH_PROCESS_ADDED (WM_APP + 101)
#define WM_PH_PROCESS_MODIFIED (WM_APP + 102)
#define WM_PH_PROCESS_REMOVED (WM_APP + 103)
#define WM_PH_PROCESSES_UPDATED (WM_APP + 104)

#define WM_PH_SERVICE_ADDED (WM_APP + 105)
#define WM_PH_SERVICE_MODIFIED (WM_APP + 106)
#define WM_PH_SERVICE_REMOVED (WM_APP + 107)
#define WM_PH_SERVICES_UPDATED (WM_APP + 108)

#define WM_PH_NETWORK_ITEM_ADDED (WM_APP + 109)
#define WM_PH_NETWORK_ITEM_MODIFIED (WM_APP + 110)
#define WM_PH_NETWORK_ITEM_REMOVED (WM_APP + 111)
#define WM_PH_NETWORK_ITEMS_UPDATED (WM_APP + 112)

#define WM_PH_SHOW_PROCESS_PROPERTIES (WM_APP + 120)
#define WM_PH_DESTROY (WM_APP + 121)
#define WM_PH_SAVE_ALL_SETTINGS (WM_APP + 122)
#define WM_PH_PREPARE_FOR_EARLY_SHUTDOWN (WM_APP + 123)
#define WM_PH_CANCEL_EARLY_SHUTDOWN (WM_APP + 124)
#define WM_PH_DELAYED_LOAD_COMPLETED (WM_APP + 125)
#define WM_PH_NOTIFY_ICON_MESSAGE (WM_APP + 126)
#define WM_PH_TOGGLE_VISIBLE (WM_APP + 127)
#define WM_PH_SHOW_MEMORY_EDITOR (WM_APP + 128)
#define WM_PH_SHOW_MEMORY_RESULTS (WM_APP + 129)
#define WM_PH_SELECT_TAB_PAGE (WM_APP + 130)
#define WM_PH_GET_CALLBACK_LAYOUT_PADDING (WM_APP + 131)
#define WM_PH_INVALIDATE_LAYOUT_PADDING (WM_APP + 132)
#define WM_PH_SELECT_PROCESS_NODE (WM_APP + 133)
#define WM_PH_SELECT_SERVICE_ITEM (WM_APP + 134)
#define WM_PH_SELECT_NETWORK_ITEM (WM_APP + 135)
#define WM_PH_UPDATE_FONT (WM_APP + 136)
#define WM_PH_GET_FONT (WM_APP + 137)
#define WM_PH_INVOKE (WM_APP + 138)
#define WM_PH_ADD_MENU_ITEM (WM_APP + 139)
#define WM_PH_ADD_TAB_PAGE (WM_APP + 140)
#define WM_PH_REFRESH (WM_APP + 141)
#define WM_PH_GET_UPDATE_AUTOMATICALLY (WM_APP + 142)
#define WM_PH_SET_UPDATE_AUTOMATICALLY (WM_APP + 143)
#define WM_PH_ICON_CLICK (WM_APP + 144)
#define WM_PH_LAST (WM_APP + 144)

#define ProcessHacker_ShowProcessProperties(hWnd, ProcessItem) \
    SendMessage(hWnd, WM_PH_SHOW_PROCESS_PROPERTIES, 0, (LPARAM)(ProcessItem))
#define ProcessHacker_Destroy(hWnd) \
    SendMessage(hWnd, WM_PH_DESTROY, 0, 0)
#define ProcessHacker_SaveAllSettings(hWnd) \
    SendMessage(hWnd, WM_PH_SAVE_ALL_SETTINGS, 0, 0)
#define ProcessHacker_PrepareForEarlyShutdown(hWnd) \
    SendMessage(hWnd, WM_PH_PREPARE_FOR_EARLY_SHUTDOWN, 0, 0)
#define ProcessHacker_CancelEarlyShutdown(hWnd) \
    SendMessage(hWnd, WM_PH_CANCEL_EARLY_SHUTDOWN, 0, 0)
#define ProcessHacker_ToggleVisible(hWnd, AlwaysShow) \
    SendMessage(hWnd, WM_PH_TOGGLE_VISIBLE, (WPARAM)(AlwaysShow), 0)
#define ProcessHacker_ShowMemoryEditor(hWnd, ShowMemoryEditor) \
    PostMessage(hWnd, WM_PH_SHOW_MEMORY_EDITOR, 0, (LPARAM)(ShowMemoryEditor))
#define ProcessHacker_ShowMemoryResults(hWnd, ShowMemoryResults) \
    PostMessage(hWnd, WM_PH_SHOW_MEMORY_RESULTS, 0, (LPARAM)(ShowMemoryResults))
#define ProcessHacker_SelectTabPage(hWnd, Index) \
    SendMessage(hWnd, WM_PH_SELECT_TAB_PAGE, (WPARAM)(Index), 0)
#define ProcessHacker_GetCallbackLayoutPadding(hWnd) \
    ((PPH_CALLBACK)SendMessage(hWnd, WM_PH_GET_CALLBACK_LAYOUT_PADDING, 0, 0))
#define ProcessHacker_InvalidateLayoutPadding(hWnd) \
    SendMessage(hWnd, WM_PH_INVALIDATE_LAYOUT_PADDING, 0, 0)
#define ProcessHacker_SelectProcessNode(hWnd, ProcessNode) \
    SendMessage(hWnd, WM_PH_SELECT_PROCESS_NODE, 0, (LPARAM)(ProcessNode))
#define ProcessHacker_SelectServiceItem(hWnd, ServiceItem) \
    SendMessage(hWnd, WM_PH_SELECT_SERVICE_ITEM, 0, (LPARAM)(ServiceItem))
#define ProcessHacker_SelectNetworkItem(hWnd, NetworkItem) \
    SendMessage(hWnd, WM_PH_SELECT_NETWORK_ITEM, 0, (LPARAM)(NetworkItem))
#define ProcessHacker_Invoke(hWnd, Function, Parameter) \
    PostMessage(hWnd, WM_PH_INVOKE, (WPARAM)(Parameter), (LPARAM)(Function))
#define ProcessHacker_AddMenuItem(hWnd, AddMenuItem) \
    ((ULONG_PTR)SendMessage(hWnd, WM_PH_ADD_MENU_ITEM, 0, (LPARAM)(AddMenuItem)))
#define ProcessHacker_AddTabPage(hWnd, TabPage) \
    ((PPH_ADDITIONAL_TAB_PAGE)SendMessage(hWnd, WM_PH_ADD_TAB_PAGE, 0, (LPARAM)(TabPage)))
#define ProcessHacker_Refresh(hWnd) \
    SendMessage(hWnd, WM_PH_REFRESH, 0, 0)
#define ProcessHacker_GetUpdateAutomatically(hWnd) \
    ((BOOLEAN)SendMessage(hWnd, WM_PH_GET_UPDATE_AUTOMATICALLY, 0, 0))
#define ProcessHacker_SetUpdateAutomatically(hWnd, Value) \
    SendMessage(hWnd, WM_PH_SET_UPDATE_AUTOMATICALLY, (WPARAM)(Value), 0)
#define ProcessHacker_IconClick(hWnd) \
    SendMessage(hWnd, WM_PH_ICON_CLICK, 0, 0)

typedef struct _PH_SHOWMEMORYEDITOR
{
    HANDLE ProcessId;
    PVOID BaseAddress;
    SIZE_T RegionSize;
    ULONG SelectOffset;
    ULONG SelectLength;
    PPH_STRING Title;
    ULONG Flags;
} PH_SHOWMEMORYEDITOR, *PPH_SHOWMEMORYEDITOR;

typedef struct _PH_SHOWMEMORYRESULTS
{
    HANDLE ProcessId;
    PPH_LIST Results;
} PH_SHOWMEMORYRESULTS, *PPH_SHOWMEMORYRESULTS;

typedef struct _PH_LAYOUT_PADDING_DATA
{
    RECT Padding;
} PH_LAYOUT_PADDING_DATA, *PPH_LAYOUT_PADDING_DATA;

typedef struct _PH_ADDMENUITEM
{
    _In_ PVOID Plugin;
    _In_ ULONG Location;
    _In_opt_ PWSTR InsertAfter;
    _In_ ULONG Flags;
    _In_ ULONG Id;
    _In_ PWSTR Text;
    _In_opt_ PVOID Context;
} PH_ADDMENUITEM, *PPH_ADDMENUITEM;

typedef HWND (NTAPI *PPH_TAB_PAGE_CREATE_FUNCTION)(
    _In_ PVOID Context
    );

typedef VOID (NTAPI *PPH_TAB_PAGE_CALLBACK_FUNCTION)(
    _In_ PVOID Parameter1,
    _In_ PVOID Parameter2,
    _In_ PVOID Parameter3,
    _In_ PVOID Context
    );

typedef struct _PH_ADDITIONAL_TAB_PAGE
{
    PWSTR Text;
    PVOID Context;
    PPH_TAB_PAGE_CREATE_FUNCTION CreateFunction;
    HWND WindowHandle;
    INT Index;
    PPH_TAB_PAGE_CALLBACK_FUNCTION SelectionChangedCallback;
    PPH_TAB_PAGE_CALLBACK_FUNCTION SaveContentCallback;
    PPH_TAB_PAGE_CALLBACK_FUNCTION FontChangedCallback;
    PPH_TAB_PAGE_CALLBACK_FUNCTION InitializeSectionMenuItemsCallback;
    PVOID Reserved[3];
} PH_ADDITIONAL_TAB_PAGE, *PPH_ADDITIONAL_TAB_PAGE;

#define PH_NOTIFY_MINIMUM 0x1
#define PH_NOTIFY_PROCESS_CREATE 0x1
#define PH_NOTIFY_PROCESS_DELETE 0x2
#define PH_NOTIFY_SERVICE_CREATE 0x4
#define PH_NOTIFY_SERVICE_DELETE 0x8
#define PH_NOTIFY_SERVICE_START 0x10
#define PH_NOTIFY_SERVICE_STOP 0x20
#define PH_NOTIFY_MAXIMUM 0x40
#define PH_NOTIFY_VALID_MASK 0x3f

BOOLEAN PhMainWndInitialization(
    _In_ INT ShowCommand
    );

VOID PhLoadDbgHelpFromPath(
    _In_ PWSTR DbgHelpPath
    );

VOID PhShowIconContextMenu(
    _In_ POINT Location
    );

PHAPPAPI
VOID PhShowIconNotification(
    _In_ PWSTR Title,
    _In_ PWSTR Text,
    _In_ ULONG Flags
    );

VOID PhShowDetailsForIconNotification(
    VOID
    );

VOID PhShowProcessContextMenu(
    _In_ PPH_TREENEW_CONTEXT_MENU ContextMenu
    );

VOID PhShowServiceContextMenu(
    _In_ PPH_TREENEW_CONTEXT_MENU ContextMenu
    );

VOID PhShowNetworkContextMenu(
    _In_ PPH_TREENEW_CONTEXT_MENU ContextMenu
    );

// plugin

extern PH_AVL_TREE PhPluginsByName;

VOID PhPluginsInitialization(
    VOID
    );

BOOLEAN PhIsPluginDisabled(
    _In_ PPH_STRINGREF BaseName
    );

VOID PhSetPluginDisabled(
    _In_ PPH_STRINGREF BaseName,
    _In_ BOOLEAN Disable
    );

VOID PhLoadPlugins(
    VOID
    );

VOID PhUnloadPlugins(
    VOID
    );

struct _PH_PLUGIN *PhFindPlugin2(
    _In_ PPH_STRINGREF Name
    );

// procprp

#define PH_PROCESS_PROPCONTEXT_MAXPAGES 20

typedef struct _PH_PROCESS_PROPCONTEXT
{
    PPH_PROCESS_ITEM ProcessItem;
    HWND WindowHandle;
    PH_EVENT CreatedEvent;
    PPH_STRING Title;
    PROPSHEETHEADER PropSheetHeader;
    HPROPSHEETPAGE *PropSheetPages;

    HANDLE SelectThreadId;
} PH_PROCESS_PROPCONTEXT, *PPH_PROCESS_PROPCONTEXT;

typedef struct _PH_PROCESS_PROPPAGECONTEXT
{
    PPH_PROCESS_PROPCONTEXT PropContext;
    PVOID Context;
    PROPSHEETPAGE PropSheetPage;

    BOOLEAN LayoutInitialized;
} PH_PROCESS_PROPPAGECONTEXT, *PPH_PROCESS_PROPPAGECONTEXT;

BOOLEAN PhProcessPropInitialization(
    VOID
    );

PHAPPAPI
PPH_PROCESS_PROPCONTEXT PhCreateProcessPropContext(
    _In_ HWND ParentWindowHandle,
    _In_ PPH_PROCESS_ITEM ProcessItem
    );

VOID PhRefreshProcessPropContext(
    _Inout_ PPH_PROCESS_PROPCONTEXT PropContext
    );

PHAPPAPI
VOID PhSetSelectThreadIdProcessPropContext(
    _Inout_ PPH_PROCESS_PROPCONTEXT PropContext,
    _In_ HANDLE ThreadId
    );

PHAPPAPI
BOOLEAN PhAddProcessPropPage(
    _Inout_ PPH_PROCESS_PROPCONTEXT PropContext,
    _In_ _Assume_refs_(1) PPH_PROCESS_PROPPAGECONTEXT PropPageContext
    );

PHAPPAPI
BOOLEAN PhAddProcessPropPage2(
    _Inout_ PPH_PROCESS_PROPCONTEXT PropContext,
    _In_ HPROPSHEETPAGE PropSheetPageHandle
    );

PHAPPAPI
PPH_PROCESS_PROPPAGECONTEXT PhCreateProcessPropPageContext(
    _In_ LPCWSTR Template,
    _In_ DLGPROC DlgProc,
    _In_opt_ PVOID Context
    );

PHAPPAPI
PPH_PROCESS_PROPPAGECONTEXT PhCreateProcessPropPageContextEx(
    _In_opt_ PVOID InstanceHandle,
    _In_ LPCWSTR Template,
    _In_ DLGPROC DlgProc,
    _In_opt_ PVOID Context
    );

PHAPPAPI
BOOLEAN PhPropPageDlgProcHeader(
    _In_ HWND hwndDlg,
    _In_ UINT uMsg,
    _In_ LPARAM lParam,
    _Out_ LPPROPSHEETPAGE *PropSheetPage,
    _Out_ PPH_PROCESS_PROPPAGECONTEXT *PropPageContext,
    _Out_ PPH_PROCESS_ITEM *ProcessItem
    );

PHAPPAPI
VOID PhPropPageDlgProcDestroy(
    _In_ HWND hwndDlg
    );

#define PH_PROP_PAGE_TAB_CONTROL_PARENT ((PPH_LAYOUT_ITEM)0x1)

PHAPPAPI
PPH_LAYOUT_ITEM PhAddPropPageLayoutItem(
    _In_ HWND hwnd,
    _In_ HWND Handle,
    _In_ PPH_LAYOUT_ITEM ParentItem,
    _In_ ULONG Anchor
    );

PHAPPAPI
VOID PhDoPropPageLayout(
    _In_ HWND hwnd
    );

FORCEINLINE PPH_LAYOUT_ITEM PhBeginPropPageLayout(
    _In_ HWND hwndDlg,
    _In_ PPH_PROCESS_PROPPAGECONTEXT PropPageContext
    )
{
    if (!PropPageContext->LayoutInitialized)
    {
        return PhAddPropPageLayoutItem(hwndDlg, hwndDlg,
            PH_PROP_PAGE_TAB_CONTROL_PARENT, PH_ANCHOR_ALL);
    }
    else
    {
        return NULL;
    }
}

FORCEINLINE VOID PhEndPropPageLayout(
    _In_ HWND hwndDlg,
    _In_ PPH_PROCESS_PROPPAGECONTEXT PropPageContext
    )
{
    PhDoPropPageLayout(hwndDlg);
    PropPageContext->LayoutInitialized = TRUE;
}

PHAPPAPI
BOOLEAN PhShowProcessProperties(
    _In_ PPH_PROCESS_PROPCONTEXT Context
    );

// sysinfo

typedef enum _PH_SYSINFO_VIEW_TYPE
{
    SysInfoSummaryView,
    SysInfoSectionView
} PH_SYSINFO_VIEW_TYPE;

typedef VOID (NTAPI *PPH_SYSINFO_COLOR_SETUP_FUNCTION)(
    _Out_ PPH_GRAPH_DRAW_INFO DrawInfo,
    _In_ COLORREF Color1,
    _In_ COLORREF Color2
    );

typedef struct _PH_SYSINFO_PARAMETERS
{
    HWND SysInfoWindowHandle;
    HWND ContainerWindowHandle;

    HFONT Font;
    HFONT MediumFont;
    HFONT LargeFont;
    ULONG FontHeight;
    ULONG FontAverageWidth;
    ULONG MediumFontHeight;
    ULONG MediumFontAverageWidth;
    COLORREF GraphBackColor;
    COLORREF PanelForeColor;
    PPH_SYSINFO_COLOR_SETUP_FUNCTION ColorSetupFunction;

    ULONG MinimumGraphHeight;
    ULONG SectionViewGraphHeight;
    ULONG PanelWidth;
} PH_SYSINFO_PARAMETERS, *PPH_SYSINFO_PARAMETERS;

typedef enum _PH_SYSINFO_SECTION_MESSAGE
{
    SysInfoCreate,
    SysInfoDestroy,
    SysInfoTick,
    SysInfoViewChanging, // PH_SYSINFO_VIEW_TYPE Parameter1, PPH_SYSINFO_SECTION Parameter2
    SysInfoCreateDialog, // PPH_SYSINFO_CREATE_DIALOG Parameter1
    SysInfoGraphGetDrawInfo, // PPH_GRAPH_DRAW_INFO Parameter1
    SysInfoGraphGetTooltipText, // PPH_SYSINFO_GRAPH_GET_TOOLTIP_TEXT Parameter1
    SysInfoGraphDrawPanel, // PPH_SYSINFO_DRAW_PANEL Parameter1
    MaxSysInfoMessage
} PH_SYSINFO_SECTION_MESSAGE;

typedef BOOLEAN (NTAPI *PPH_SYSINFO_SECTION_CALLBACK)(
    _In_ struct _PH_SYSINFO_SECTION *Section,
    _In_ PH_SYSINFO_SECTION_MESSAGE Message,
    _In_opt_ PVOID Parameter1,
    _In_opt_ PVOID Parameter2
    );

typedef struct _PH_SYSINFO_CREATE_DIALOG
{
    BOOLEAN CustomCreate;

    // Parameters for default create
    PVOID Instance;
    PWSTR Template;
    DLGPROC DialogProc;
    PVOID Parameter;
} PH_SYSINFO_CREATE_DIALOG, *PPH_SYSINFO_CREATE_DIALOG;

typedef struct _PH_SYSINFO_GRAPH_GET_TOOLTIP_TEXT
{
    ULONG Index;
    PH_STRINGREF Text;
} PH_SYSINFO_GRAPH_GET_TOOLTIP_TEXT, *PPH_SYSINFO_GRAPH_GET_TOOLTIP_TEXT;

typedef struct _PH_SYSINFO_DRAW_PANEL
{
    HDC hdc;
    RECT Rect;
    BOOLEAN CustomDraw;

    // Parameters for default draw
    PPH_STRING Title;
    PPH_STRING SubTitle;
    PPH_STRING SubTitleOverflow;
} PH_SYSINFO_DRAW_PANEL, *PPH_SYSINFO_DRAW_PANEL;

typedef struct _PH_SYSINFO_SECTION
{
    // Public

    // Initialization
    PH_STRINGREF Name;
    ULONG Flags;
    PPH_SYSINFO_SECTION_CALLBACK Callback;
    PVOID Context;
    PVOID Reserved[3];

    // State
    HWND GraphHandle;
    PH_GRAPH_STATE GraphState;
    PPH_SYSINFO_PARAMETERS Parameters;
    PVOID Reserved2[3];

    // Private

    struct
    {
        ULONG GraphHot : 1;
        ULONG PanelHot : 1;
        ULONG HasFocus : 1;
        ULONG HideFocus : 1;
        ULONG SpareFlags : 28;
    };
    HWND DialogHandle;
    HWND PanelHandle;
    ULONG PanelId;
    WNDPROC GraphOldWndProc;
    WNDPROC PanelOldWndProc;
} PH_SYSINFO_SECTION, *PPH_SYSINFO_SECTION;

VOID PhSiNotifyChangeSettings(
    VOID
    );

PHAPPAPI
VOID PhSiSetColorsGraphDrawInfo(
    _Out_ PPH_GRAPH_DRAW_INFO DrawInfo,
    _In_ COLORREF Color1,
    _In_ COLORREF Color2
    );

VOID PhShowSystemInformationDialog(
    _In_opt_ PWSTR SectionName
    );

// miniinfo

typedef struct _PH_MINIINFO_PARAMETERS
{
    HWND ContainerWindowHandle;
    HWND MiniInfoWindowHandle;

    HFONT Font;
    HFONT MediumFont;
    ULONG FontHeight;
    ULONG FontAverageWidth;
    ULONG MediumFontHeight;
    ULONG MediumFontAverageWidth;
} PH_MINIINFO_PARAMETERS, *PPH_MINIINFO_PARAMETERS;

typedef enum _PH_MINIINFO_SECTION_MESSAGE
{
    MiniInfoCreate,
    MiniInfoDestroy,
    MiniInfoTick,
    MiniInfoViewChanging, // PPH_MINIINFO_SECTION Parameter1
    MiniInfoCreateDialog, // PPH_MINIINFO_CREATE_DIALOG Parameter1
    MaxMiniInfoMessage
} PH_MINIINFO_SECTION_MESSAGE;

typedef BOOLEAN (NTAPI *PPH_MINIINFO_SECTION_CALLBACK)(
    _In_ struct _PH_MINIINFO_SECTION *Section,
    _In_ PH_MINIINFO_SECTION_MESSAGE Message,
    _In_opt_ PVOID Parameter1,
    _In_opt_ PVOID Parameter2
    );

typedef struct _PH_MINIINFO_CREATE_DIALOG
{
    BOOLEAN CustomCreate;

    // Parameters for default create
    PVOID Instance;
    PWSTR Template;
    DLGPROC DialogProc;
    PVOID Parameter;
} PH_MINIINFO_CREATE_DIALOG, *PPH_MINIINFO_CREATE_DIALOG;

typedef struct _PH_MINIINFO_SECTION
{
    // Public

    // Initialization
    PH_STRINGREF Name;
    ULONG Flags;
    PPH_MINIINFO_SECTION_CALLBACK Callback;
    PVOID Context;
    PPH_MINIINFO_PARAMETERS Parameters;
    PVOID Reserved[2];

    // Private

    struct
    {
        ULONG SpareFlags : 32;
    };
    HWND DialogHandle;
} PH_MINIINFO_SECTION, *PPH_MINIINFO_SECTION;

typedef enum _PH_MINIINFO_PIN_TYPE
{
    MiniInfoManualPinType, // User pin
    MiniInfoIconPinType, // Notification icon
    MiniInfoActivePinType, // Window is active
    MiniInfoHoverPinType, // Cursor is over mini info window
    MiniInfoChildControlPinType, // Interacting with child control
    MaxMiniInfoPinType
} PH_MINIINFO_PIN_TYPE;

#define PH_MINIINFO_ACTIVATE_WINDOW 0x1
#define PH_MINIINFO_LOAD_POSITION 0x2

VOID PhPinMiniInformation(
    _In_ PH_MINIINFO_PIN_TYPE PinType,
    _In_ LONG PinCount,
    _In_opt_ ULONG PinDelayMs,
    _In_ ULONG Flags,
    _In_opt_ PWSTR SectionName,
    _In_opt_ PPOINT SourcePoint
    );

// log

#define PH_LOG_ENTRY_PROCESS_FIRST 1
#define PH_LOG_ENTRY_PROCESS_CREATE 1
#define PH_LOG_ENTRY_PROCESS_DELETE 2
#define PH_LOG_ENTRY_PROCESS_LAST 2

#define PH_LOG_ENTRY_SERVICE_FIRST 3
#define PH_LOG_ENTRY_SERVICE_CREATE 3
#define PH_LOG_ENTRY_SERVICE_DELETE 4
#define PH_LOG_ENTRY_SERVICE_START 5
#define PH_LOG_ENTRY_SERVICE_STOP 6
#define PH_LOG_ENTRY_SERVICE_CONTINUE 7
#define PH_LOG_ENTRY_SERVICE_PAUSE 8
#define PH_LOG_ENTRY_SERVICE_LAST 8

#define PH_LOG_ENTRY_MESSAGE 9

typedef struct _PH_LOG_ENTRY
{
    UCHAR Type;
    UCHAR Reserved1;
    USHORT Flags;
    LARGE_INTEGER Time;
    union
    {
        struct
        {
            HANDLE ProcessId;
            PPH_STRING Name;
            HANDLE ParentProcessId;
            PPH_STRING ParentName;
        } Process;
        struct
        {
            PPH_STRING Name;
            PPH_STRING DisplayName;
        } Service;
        PPH_STRING Message;
    };
    UCHAR Buffer[1];
} PH_LOG_ENTRY, *PPH_LOG_ENTRY;

#ifndef PH_LOG_PRIVATE
PH_CIRCULAR_BUFFER_PVOID PhLogBuffer;
PH_CALLBACK PhLoggedCallback;
#endif

VOID PhLogInitialization(
    VOID
    );

VOID PhClearLogEntries(
    VOID
    );

VOID PhLogProcessEntry(
    _In_ UCHAR Type,
    _In_ HANDLE ProcessId,
    _In_ PPH_STRING Name,
    _In_opt_ HANDLE ParentProcessId,
    _In_opt_ PPH_STRING ParentName
    );

VOID PhLogServiceEntry(
    _In_ UCHAR Type,
    _In_ PPH_STRING Name,
    _In_ PPH_STRING DisplayName
    );

PHAPPAPI
VOID PhLogMessageEntry(
    _In_ UCHAR Type,
    _In_ PPH_STRING Message
    );

PHAPPAPI
PPH_STRING PhFormatLogEntry(
    _In_ PPH_LOG_ENTRY Entry
    );

// dbgcon

VOID PhShowDebugConsole(
    VOID
    );

// actions

typedef enum _PH_ACTION_ELEVATION_LEVEL
{
    NeverElevateAction = 0,
    PromptElevateAction = 1,
    AlwaysElevateAction = 2
} PH_ACTION_ELEVATION_LEVEL;

typedef enum _PH_PHSVC_MODE
{
    ElevatedPhSvcMode,
    Wow64PhSvcMode
} PH_PHSVC_MODE;

PHAPPAPI
BOOLEAN PhUiConnectToPhSvc(
    _In_opt_ HWND hWnd,
    _In_ BOOLEAN ConnectOnly
    );

PHAPPAPI
BOOLEAN PhUiConnectToPhSvcEx(
    _In_opt_ HWND hWnd,
    _In_ PH_PHSVC_MODE Mode,
    _In_ BOOLEAN ConnectOnly
    );

PHAPPAPI
VOID PhUiDisconnectFromPhSvc(
    VOID
    );

PHAPPAPI
BOOLEAN PhUiLockComputer(
    _In_ HWND hWnd
    );

PHAPPAPI
BOOLEAN PhUiLogoffComputer(
    _In_ HWND hWnd
    );

PHAPPAPI
BOOLEAN PhUiSleepComputer(
    _In_ HWND hWnd
    );

PHAPPAPI
BOOLEAN PhUiHibernateComputer(
    _In_ HWND hWnd
    );

PHAPPAPI
BOOLEAN PhUiRestartComputer(
    _In_ HWND hWnd,
    _In_ ULONG Flags
    );

PHAPPAPI
BOOLEAN PhUiShutdownComputer(
    _In_ HWND hWnd,
    _In_ ULONG Flags
    );

PHAPPAPI
BOOLEAN PhUiConnectSession(
    _In_ HWND hWnd,
    _In_ ULONG SessionId
    );

PHAPPAPI
BOOLEAN PhUiDisconnectSession(
    _In_ HWND hWnd,
    _In_ ULONG SessionId
    );

PHAPPAPI
BOOLEAN PhUiLogoffSession(
    _In_ HWND hWnd,
    _In_ ULONG SessionId
    );

PHAPPAPI
BOOLEAN PhUiTerminateProcesses(
    _In_ HWND hWnd,
    _In_ PPH_PROCESS_ITEM *Processes,
    _In_ ULONG NumberOfProcesses
    );

PHAPPAPI
BOOLEAN PhUiTerminateTreeProcess(
    _In_ HWND hWnd,
    _In_ PPH_PROCESS_ITEM Process
    );

PHAPPAPI
BOOLEAN PhUiSuspendProcesses(
    _In_ HWND hWnd,
    _In_ PPH_PROCESS_ITEM *Processes,
    _In_ ULONG NumberOfProcesses
    );

PHAPPAPI
BOOLEAN PhUiResumeProcesses(
    _In_ HWND hWnd,
    _In_ PPH_PROCESS_ITEM *Processes,
    _In_ ULONG NumberOfProcesses
    );

PHAPPAPI
BOOLEAN PhUiRestartProcess(
    _In_ HWND hWnd,
    _In_ PPH_PROCESS_ITEM Process
    );

PHAPPAPI
BOOLEAN PhUiDebugProcess(
    _In_ HWND hWnd,
    _In_ PPH_PROCESS_ITEM Process
    );

PHAPPAPI
BOOLEAN PhUiReduceWorkingSetProcesses(
    _In_ HWND hWnd,
    _In_ PPH_PROCESS_ITEM *Processes,
    _In_ ULONG NumberOfProcesses
    );

PHAPPAPI
BOOLEAN PhUiSetVirtualizationProcess(
    _In_ HWND hWnd,
    _In_ PPH_PROCESS_ITEM Process,
    _In_ BOOLEAN Enable
    );

PHAPPAPI
BOOLEAN PhUiDetachFromDebuggerProcess(
    _In_ HWND hWnd,
    _In_ PPH_PROCESS_ITEM Process
    );

PHAPPAPI
BOOLEAN PhUiInjectDllProcess(
    _In_ HWND hWnd,
    _In_ PPH_PROCESS_ITEM Process
    );

PHAPPAPI
BOOLEAN PhUiSetIoPriorityProcesses(
    _In_ HWND hWnd,
    _In_ PPH_PROCESS_ITEM *Processes,
    _In_ ULONG NumberOfProcesses,
    _In_ ULONG IoPriority
    );

PHAPPAPI
BOOLEAN PhUiSetPagePriorityProcess(
    _In_ HWND hWnd,
    _In_ PPH_PROCESS_ITEM Process,
    _In_ ULONG PagePriority
    );

PHAPPAPI
BOOLEAN PhUiSetPriorityProcesses(
    _In_ HWND hWnd,
    _In_ PPH_PROCESS_ITEM *Processes,
    _In_ ULONG NumberOfProcesses,
    _In_ ULONG PriorityClass
    );

PHAPPAPI
BOOLEAN PhUiSetDepStatusProcess(
    _In_ HWND hWnd,
    _In_ PPH_PROCESS_ITEM Process
    );

PHAPPAPI
BOOLEAN PhUiStartService(
    _In_ HWND hWnd,
    _In_ PPH_SERVICE_ITEM Service
    );

PHAPPAPI
BOOLEAN PhUiContinueService(
    _In_ HWND hWnd,
    _In_ PPH_SERVICE_ITEM Service
    );

PHAPPAPI
BOOLEAN PhUiPauseService(
    _In_ HWND hWnd,
    _In_ PPH_SERVICE_ITEM Service
    );

PHAPPAPI
BOOLEAN PhUiStopService(
    _In_ HWND hWnd,
    _In_ PPH_SERVICE_ITEM Service
    );

PHAPPAPI
BOOLEAN PhUiDeleteService(
    _In_ HWND hWnd,
    _In_ PPH_SERVICE_ITEM Service
    );

PHAPPAPI
BOOLEAN PhUiCloseConnections(
    _In_ HWND hWnd,
    _In_ PPH_NETWORK_ITEM *Connections,
    _In_ ULONG NumberOfConnections
    );

PHAPPAPI
BOOLEAN PhUiTerminateThreads(
    _In_ HWND hWnd,
    _In_ PPH_THREAD_ITEM *Threads,
    _In_ ULONG NumberOfThreads
    );

PHAPPAPI
BOOLEAN PhUiForceTerminateThreads(
    _In_ HWND hWnd,
    _In_ HANDLE ProcessId,
    _In_ PPH_THREAD_ITEM *Threads,
    _In_ ULONG NumberOfThreads
    );

PHAPPAPI
BOOLEAN PhUiSuspendThreads(
    _In_ HWND hWnd,
    _In_ PPH_THREAD_ITEM *Threads,
    _In_ ULONG NumberOfThreads
    );

PHAPPAPI
BOOLEAN PhUiResumeThreads(
    _In_ HWND hWnd,
    _In_ PPH_THREAD_ITEM *Threads,
    _In_ ULONG NumberOfThreads
    );

PHAPPAPI
BOOLEAN PhUiSetPriorityThread(
    _In_ HWND hWnd,
    _In_ PPH_THREAD_ITEM Thread,
    _In_ ULONG ThreadPriorityWin32
    );

PHAPPAPI
BOOLEAN PhUiSetIoPriorityThread(
    _In_ HWND hWnd,
    _In_ PPH_THREAD_ITEM Thread,
    _In_ ULONG IoPriority
    );

PHAPPAPI
BOOLEAN PhUiSetPagePriorityThread(
    _In_ HWND hWnd,
    _In_ PPH_THREAD_ITEM Thread,
    _In_ ULONG PagePriority
    );

PHAPPAPI
BOOLEAN PhUiUnloadModule(
    _In_ HWND hWnd,
    _In_ HANDLE ProcessId,
    _In_ PPH_MODULE_ITEM Module
    );

PHAPPAPI
BOOLEAN PhUiFreeMemory(
    _In_ HWND hWnd,
    _In_ HANDLE ProcessId,
    _In_ PPH_MEMORY_ITEM MemoryItem,
    _In_ BOOLEAN Free
    );

PHAPPAPI
BOOLEAN PhUiCloseHandles(
    _In_ HWND hWnd,
    _In_ HANDLE ProcessId,
    _In_ PPH_HANDLE_ITEM *Handles,
    _In_ ULONG NumberOfHandles,
    _In_ BOOLEAN Warn
    );

PHAPPAPI
BOOLEAN PhUiSetAttributesHandle(
    _In_ HWND hWnd,
    _In_ HANDLE ProcessId,
    _In_ PPH_HANDLE_ITEM Handle,
    _In_ ULONG Attributes
    );

PHAPPAPI
BOOLEAN PhUiDestroyHeap(
    _In_ HWND hWnd,
    _In_ HANDLE ProcessId,
    _In_ PVOID HeapHandle
    );

// itemtips

PPH_STRING PhGetProcessTooltipText(
    _In_ PPH_PROCESS_ITEM Process,
    _Out_opt_ PULONG ValidToTickCount
    );

PPH_STRING PhGetServiceTooltipText(
    _In_ PPH_SERVICE_ITEM Service
    );

// cmdmode

NTSTATUS PhCommandModeStart(
    VOID
    );

// anawait

VOID PhUiAnalyzeWaitThread(
    _In_ HWND hWnd,
    _In_ HANDLE ProcessId,
    _In_ HANDLE ThreadId,
    _In_ PPH_SYMBOL_PROVIDER SymbolProvider
    );

// mdump

BOOLEAN PhUiCreateDumpFileProcess(
    _In_ HWND hWnd,
    _In_ PPH_PROCESS_ITEM Process
    );

// about

VOID PhShowAboutDialog(
    _In_ HWND ParentWindowHandle
    );

PPH_STRING PhGetDiagnosticsString(
    VOID
    );

// affinity

VOID PhShowProcessAffinityDialog(
    _In_ HWND ParentWindowHandle,
    _In_opt_ PPH_PROCESS_ITEM ProcessItem,
    _In_opt_ PPH_THREAD_ITEM ThreadItem
    );

PHAPPAPI
BOOLEAN PhShowProcessAffinityDialog2(
    _In_ HWND ParentWindowHandle,
    _In_ ULONG_PTR AffinityMask,
    _Out_ PULONG_PTR NewAffinityMask
    );

// chcol

#define PH_CONTROL_TYPE_TREE_NEW 1

VOID PhShowChooseColumnsDialog(
    _In_ HWND ParentWindowHandle,
    _In_ HWND ControlHandle,
    _In_ ULONG Type
    );

// chdlg

#define PH_CHOICE_DIALOG_SAVED_CHOICES 10

#define PH_CHOICE_DIALOG_CHOICE 0x0
#define PH_CHOICE_DIALOG_USER_CHOICE 0x1
#define PH_CHOICE_DIALOG_PASSWORD 0x2
#define PH_CHOICE_DIALOG_TYPE_MASK 0x3

PHAPPAPI
BOOLEAN PhaChoiceDialog(
    _In_ HWND ParentWindowHandle,
    _In_ PWSTR Title,
    _In_ PWSTR Message,
    _In_opt_ PWSTR *Choices,
    _In_opt_ ULONG NumberOfChoices,
    _In_opt_ PWSTR Option,
    _In_ ULONG Flags,
    _Inout_ PPH_STRING *SelectedChoice,
    _Inout_opt_ PBOOLEAN SelectedOption,
    _In_opt_ PWSTR SavedChoicesSettingName
    );

// chproc

PHAPPAPI
BOOLEAN PhShowChooseProcessDialog(
    _In_ HWND ParentWindowHandle,
    _In_ PWSTR Message,
    _Out_ PHANDLE ProcessId
    );

// findobj

VOID PhShowFindObjectsDialog(
    VOID
    );

// gdihndl

VOID PhShowGdiHandlesDialog(
    _In_ HWND ParentWindowHandle,
    _In_ PPH_PROCESS_ITEM ProcessItem
    );

// hidnproc

VOID PhShowHiddenProcessesDialog(
    VOID
    );

// hndlprp

VOID PhShowHandleProperties(
    _In_ HWND ParentWindowHandle,
    _In_ HANDLE ProcessId,
    _In_ PPH_HANDLE_ITEM HandleItem
    );

// hndlstat

VOID PhShowHandleStatisticsDialog(
    _In_ HWND ParentWindowHandle,
    _In_ HANDLE ProcessId
    );

// infodlg

VOID PhShowInformationDialog(
    _In_ HWND ParentWindowHandle,
    _In_ PWSTR String
    );

// jobprp

VOID PhShowJobProperties(
    _In_ HWND ParentWindowHandle,
    _In_ PPH_OPEN_OBJECT OpenObject,
    _In_opt_ PVOID Context,
    _In_opt_ PWSTR Title
    );

HPROPSHEETPAGE PhCreateJobPage(
    _In_ PPH_OPEN_OBJECT OpenObject,
    _In_opt_ PVOID Context,
    _In_opt_ DLGPROC HookProc
    );

// logwnd

VOID PhShowLogDialog(
    VOID
    );

// memedit

#define PH_MEMORY_EDITOR_UNMAP_VIEW_OF_SECTION 0x1

VOID PhShowMemoryEditorDialog(
    _In_ HANDLE ProcessId,
    _In_ PVOID BaseAddress,
    _In_ SIZE_T RegionSize,
    _In_ ULONG SelectOffset,
    _In_ ULONG SelectLength,
    _In_opt_ PPH_STRING Title,
    _In_ ULONG Flags
    );

// memlists

VOID PhShowMemoryListsDialog(
    _In_ HWND ParentWindowHandle,
    _In_opt_ VOID (NTAPI *RegisterDialog)(HWND),
    _In_opt_ VOID (NTAPI *UnregisterDialog)(HWND)
    );

// memprot

VOID PhShowMemoryProtectDialog(
    _In_ HWND ParentWindowHandle,
    _In_ PPH_PROCESS_ITEM ProcessItem,
    _In_ PPH_MEMORY_ITEM MemoryItem
    );

// memrslt

VOID PhShowMemoryResultsDialog(
    _In_ HANDLE ProcessId,
    _In_ PPH_LIST Results
    );

// memsrch

VOID PhShowMemoryStringDialog(
    _In_ HWND ParentWindowHandle,
    _In_ PPH_PROCESS_ITEM ProcessItem
    );

// netstk

VOID PhShowNetworkStackDialog(
    _In_ HWND ParentWindowHandle,
    _In_ PPH_NETWORK_ITEM NetworkItem
    );

// ntobjprp

HPROPSHEETPAGE PhCreateEventPage(
    _In_ PPH_OPEN_OBJECT OpenObject,
    _In_opt_ PVOID Context
    );

HPROPSHEETPAGE PhCreateEventPairPage(
    _In_ PPH_OPEN_OBJECT OpenObject,
    _In_opt_ PVOID Context
    );

HPROPSHEETPAGE PhCreateMutantPage(
    _In_ PPH_OPEN_OBJECT OpenObject,
    _In_opt_ PVOID Context
    );

HPROPSHEETPAGE PhCreateSectionPage(
    _In_ PPH_OPEN_OBJECT OpenObject,
    _In_opt_ PVOID Context
    );

HPROPSHEETPAGE PhCreateSemaphorePage(
    _In_ PPH_OPEN_OBJECT OpenObject,
    _In_opt_ PVOID Context
    );

HPROPSHEETPAGE PhCreateTimerPage(
    _In_ PPH_OPEN_OBJECT OpenObject,
    _In_opt_ PVOID Context
    );

// options

VOID PhShowOptionsDialog(
    _In_ HWND ParentWindowHandle
    );

// pagfiles

VOID PhShowPagefilesDialog(
    _In_ HWND ParentWindowHandle
    );

// plugman

VOID PhShowPluginsDialog(
    _In_ HWND ParentWindowHandle
    );

// procrec

PHAPPAPI
VOID PhShowProcessRecordDialog(
    _In_ HWND ParentWindowHandle,
    _In_ PPH_PROCESS_RECORD Record
    );

// runas

typedef struct _PH_RUNAS_SERVICE_PARAMETERS
{
    ULONG ProcessId;
    PWSTR UserName;
    PWSTR Password;
    ULONG LogonType;
    ULONG SessionId;
    PWSTR CurrentDirectory;
    PWSTR CommandLine;
    PWSTR FileName;
    PWSTR DesktopName;
    BOOLEAN UseLinkedToken;
    PWSTR ServiceName;
} PH_RUNAS_SERVICE_PARAMETERS, *PPH_RUNAS_SERVICE_PARAMETERS;

VOID PhShowRunAsDialog(
    _In_ HWND ParentWindowHandle,
    _In_opt_ HANDLE ProcessId
    );

NTSTATUS PhExecuteRunAsCommand(
    _In_ PPH_RUNAS_SERVICE_PARAMETERS Parameters
    );

NTSTATUS PhExecuteRunAsCommand2(
    _In_ HWND hWnd,
    _In_ PWSTR Program,
    _In_opt_ PWSTR UserName,
    _In_opt_ PWSTR Password,
    _In_opt_ ULONG LogonType,
    _In_opt_ HANDLE ProcessIdWithToken,
    _In_ ULONG SessionId,
    _In_ PWSTR DesktopName,
    _In_ BOOLEAN UseLinkedToken
    );

NTSTATUS PhRunAsServiceStart(
    _In_ PPH_STRING ServiceName
    );

NTSTATUS PhInvokeRunAsService(
    _In_ PPH_RUNAS_SERVICE_PARAMETERS Parameters
    );

// sessmsg

VOID PhShowSessionSendMessageDialog(
    _In_ HWND ParentWindowHandle,
    _In_ ULONG SessionId
    );

// sessprp

VOID PhShowSessionProperties(
    _In_ HWND ParentWindowHandle,
    _In_ ULONG SessionId
    );

// sessshad

VOID PhShowSessionShadowDialog(
    _In_ HWND ParentWindowHandle,
    _In_ ULONG SessionId
    );

// srvcr

VOID PhShowCreateServiceDialog(
    _In_ HWND ParentWindowHandle
    );

// srvctl

#define WM_PH_SET_LIST_VIEW_SETTINGS (WM_APP + 701)

PHAPPAPI
HWND PhCreateServiceListControl(
    _In_ HWND ParentWindowHandle,
    _In_ PPH_SERVICE_ITEM *Services,
    _In_ ULONG NumberOfServices
    );

// srvprp

VOID PhShowServiceProperties(
    _In_ HWND ParentWindowHandle,
    _In_ PPH_SERVICE_ITEM ServiceItem
    );

// termator

VOID PhShowProcessTerminatorDialog(
    _In_ HWND ParentWindowHandle,
    _In_ PPH_PROCESS_ITEM ProcessItem
    );

// thrdstk

VOID PhShowThreadStackDialog(
    _In_ HWND ParentWindowHandle,
    _In_ HANDLE ProcessId,
    _In_ HANDLE ThreadId,
    _In_ PPH_SYMBOL_PROVIDER SymbolProvider
    );

// tokprp

PPH_STRING PhGetGroupAttributesString(
    _In_ ULONG Attributes
    );

PWSTR PhGetPrivilegeAttributesString(
    _In_ ULONG Attributes
    );

VOID PhShowTokenProperties(
    _In_ HWND ParentWindowHandle,
    _In_ PPH_OPEN_OBJECT OpenObject,
    _In_opt_ PVOID Context,
    _In_opt_ PWSTR Title
    );

HPROPSHEETPAGE PhCreateTokenPage(
    _In_ PPH_OPEN_OBJECT OpenObject,
    _In_opt_ PVOID Context,
    _In_opt_ DLGPROC HookProc
    );

#endif
