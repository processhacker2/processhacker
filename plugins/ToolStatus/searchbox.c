/*
 * Process Hacker -
 *   Subclassed Edit control
 *
 * Copyright (C) 2012-2013 dmex
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
 *
 */

#include "toolstatus.h"

//NONCLIENTMETRICS metrics = { sizeof(NONCLIENTMETRICS) };
//metrics.lfMenuFont.lfHeight = 14;
//metrics.lfMenuFont.lfWeight = FW_NORMAL;
//metrics.lfMenuFont.lfQuality = CLEARTYPE_QUALITY | ANTIALIASED_QUALITY;

#ifdef _UXTHEME_ENABLED_
#include <uxtheme.h>
#include <vsstyle.h>
#include <vssym32.h>

typedef HRESULT (WINAPI *_GetThemeColor)(
    _In_ HTHEME hTheme,
    _In_ INT iPartId,
    _In_ INT iStateId,
    _In_ INT iPropId,
    _Out_ COLORREF *pColor
    );
typedef HRESULT (WINAPI *_SetWindowTheme)(
    _In_ HWND hwnd,
    _In_ LPCWSTR pszSubAppName,
    _In_ LPCWSTR pszSubIdList
    );

typedef HRESULT (WINAPI *_GetThemeFont)(
    _In_ HTHEME hTheme,
    _In_ HDC hdc,
    _In_ INT iPartId,
    _In_ INT iStateId,
    _In_ INT iPropId,
    _Out_ LOGFONTW *pFont
    );

typedef HRESULT (WINAPI *_GetThemeSysFont)(
    _In_ HTHEME hTheme,
    _In_ INT iFontId,
    _Out_ LOGFONTW *plf
    );

typedef BOOL (WINAPI *_IsThemeBackgroundPartiallyTransparent)(
    _In_ HTHEME hTheme,
    _In_ INT iPartId,
    _In_ INT iStateId
    );

typedef HRESULT (WINAPI *_DrawThemeParentBackground)(
    _In_ HWND hwnd,
    _In_ HDC hdc,
    _In_opt_ const RECT* prc
    );

typedef HRESULT (WINAPI *_GetThemeBackgroundContentRect)(
    _In_ HTHEME hTheme,
    _In_ HDC hdc,
    _In_ INT iPartId,
    _In_ INT iStateId,
    _Inout_ LPCRECT pBoundingRect,
    _Out_ LPRECT pContentRect
    );

static _IsThemeActive IsThemeActive_I;
static _OpenThemeData OpenThemeData_I;
static _SetWindowTheme SetWindowTheme_I;
static _CloseThemeData CloseThemeData_I;
static _IsThemePartDefined IsThemePartDefined_I;
static _DrawThemeBackground DrawThemeBackground_I;
static _DrawThemeParentBackground DrawThemeParentBackground_I;
static _IsThemeBackgroundPartiallyTransparent IsThemeBackgroundPartiallyTransparent_I;
static _GetThemeColor GetThemeColor_I;
static _GetThemeInt GetThemeInt_I;

static VOID NcAreaInitializeUxTheme(
    _Inout_ PEDIT_CONTEXT Context
    )
{
    if (!Context->UxThemeModule)
        Context->UxThemeModule = LoadLibrary(L"uxtheme.dll");

    if (Context->UxThemeModule)
    {
        IsThemeActive_I = (_IsThemeActive)GetProcAddress(Context->UxThemeModule, "IsThemeActive");
        OpenThemeData_I = (_OpenThemeData)GetProcAddress(Context->UxThemeModule, "OpenThemeData");
        SetWindowTheme_I = (_SetWindowTheme)GetProcAddress(Context->UxThemeModule, "SetWindowTheme");
        CloseThemeData_I = (_CloseThemeData)GetProcAddress(Context->UxThemeModule, "CloseThemeData");
        GetThemeColor_I = (_GetThemeColor)GetProcAddress(Context->UxThemeModule, "GetThemeColor");
        GetThemeInt_I = (_GetThemeInt)GetProcAddress(Context->UxThemeModule, "GetThemeInt");
        IsThemePartDefined_I = (_IsThemePartDefined)GetProcAddress(Context->UxThemeModule, "IsThemePartDefined");
        DrawThemeBackground_I = (_DrawThemeBackground)GetProcAddress(Context->UxThemeModule, "DrawThemeBackground");
        DrawThemeParentBackground_I = (_DrawThemeParentBackground)GetProcAddress(Context->UxThemeModule, "DrawThemeParentBackground");
        IsThemeBackgroundPartiallyTransparent_I = (_IsThemeBackgroundPartiallyTransparent)GetProcAddress(Context->UxThemeModule, "IsThemeBackgroundPartiallyTransparent");
    }

    if (IsThemeActive_I && OpenThemeData_I && CloseThemeData_I && IsThemePartDefined_I && DrawThemeBackground_I)
    {
        INT borderSize = 0;

        if (Context->UxThemeHandle)
            CloseThemeData_I(Context->UxThemeHandle);

        Context->IsThemeActive = IsThemeActive_I();
        Context->UxThemeHandle = OpenThemeData_I(Context->WindowHandle, VSCLASS_EDIT);

        if (Context->UxThemeHandle)
        {
            Context->IsThemeBackgroundActive = IsThemePartDefined_I(
                Context->UxThemeHandle,
                EP_EDITBORDER_NOSCROLL,
                0
                );

            // Get the border size from the theme
            GetThemeInt_I(
                Context->UxThemeHandle, 
                EP_EDITBORDER_NOSCROLL, 
                EPSN_NORMAL, 
                TMT_BORDERSIZE, 
                &borderSize
                );

            //GetThemeColor_I(
            //    Context->UxThemeHandle,
            //    EP_EDITBORDER_NOSCROLL,
            //    EPSN_NORMAL,
            //    TMT_FILLCOLOR,
            //    &Context->BackgroundColorRef
            //    );
            //GetThemeColor_I(
            //    Context->UxThemeHandle,
            //    EP_EDITBORDER_NOSCROLL,
            //    EPSN_NORMAL,
            //    TMT_BORDERCOLOR,
            //    &Context->clrUxThemeBackgroundRef
            //    );

            Context->CXBorder = borderSize * 2;
            Context->CYBorder = borderSize * 2;
        }
        else
        {
            Context->IsThemeBackgroundActive = FALSE;
        }
    }
    else
    {
        Context->UxThemeHandle = NULL;
        Context->IsThemeActive = FALSE;
        Context->IsThemeBackgroundActive = FALSE;
    }
}
#endif _UXTHEME_ENABLED_

static VOID NcAreaFreeGdiTheme(
    _Inout_ PEDIT_CONTEXT Context
    )
{
    if (Context->BrushNormal)
        DeleteObject(Context->BrushNormal);

    if (Context->BrushHot)
        DeleteObject(Context->BrushHot);

    if (Context->BrushFocused)
        DeleteObject(Context->BrushFocused);

    if (Context->BrushFill)
        DeleteObject(Context->BrushFill);
}

static VOID NcAreaInitializeGdiTheme(
    _Inout_ PEDIT_CONTEXT Context
    )
{
    Context->CXBorder = GetSystemMetrics(SM_CXBORDER) * 2;
    Context->CYBorder = GetSystemMetrics(SM_CYBORDER) * 2;

    Context->BackgroundColorRef = RGB(0, 0, 0);//GetSysColor(COLOR_WINDOW);
    Context->BrushFill = GetSysColorBrush(COLOR_WINDOW);     
    Context->BrushNormal = GetStockBrush(BLACK_BRUSH);
    Context->BrushHot = WindowsVersion < WINDOWS_VISTA ? CreateSolidBrush(RGB(50, 150, 255)) : GetSysColorBrush(COLOR_HIGHLIGHT);
    Context->BrushFocused = WindowsVersion < WINDOWS_VISTA ? CreateSolidBrush(RGB(50, 150, 255)) : GetSysColorBrush(COLOR_HIGHLIGHT);
}

static VOID NcAreaInitializeImageList(
    _Inout_ PEDIT_CONTEXT Context
    )
{
    HBITMAP bitmapActive = NULL;
    HBITMAP bitmapInactive = NULL;

    Context->ImageWidth = 23;
    Context->ImageHeight = 20;
    Context->ImageList = ImageList_Create(32, 32, ILC_COLOR32 | ILC_MASK, 0, 0);

    ImageList_SetBkColor(Context->ImageList, Context->BackgroundColorRef);
    ImageList_SetImageCount(Context->ImageList, 2);

    // Add the images to the imagelist
    if (bitmapActive = LoadImageFromResources(Context->ImageWidth, Context->ImageHeight, MAKEINTRESOURCE(IDB_SEARCH_ACTIVE)))
    {
        ImageList_Replace(Context->ImageList, 0, bitmapActive, NULL);
        DeleteObject(bitmapActive);
    }
    else
    {
        PhSetImageListBitmap(Context->ImageList, 0, (HINSTANCE)PluginInstance->DllBase, MAKEINTRESOURCE(IDB_SEARCH_ACTIVE_BMP));
    }

    if (bitmapInactive = LoadImageFromResources(Context->ImageWidth, Context->ImageHeight, MAKEINTRESOURCE(IDB_SEARCH_INACTIVE)))
    {
        ImageList_Replace(Context->ImageList, 1, bitmapInactive, NULL);
        DeleteObject(bitmapInactive);
    }
    else
    {
        PhSetImageListBitmap(Context->ImageList, 1, (HINSTANCE)PluginInstance->DllBase, MAKEINTRESOURCE(IDB_SEARCH_INACTIVE_BMP));
    }
}

static VOID NcAreaGetButtonRect(
    _Inout_ PEDIT_CONTEXT Context,
    _In_ RECT* rect
    )
{
    rect->left = (rect->right - Context->cxImgSize) - 2; // GetSystemMetrics(SM_CXBORDER)
    rect->bottom -= 2;
    rect->right -= 2;
    rect->top += 2;
}

static LRESULT CALLBACK NcAreaWndSubclassProc(
    _In_ HWND hwndDlg,
    _In_ UINT uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _In_ UINT_PTR uIdSubclass,
    _In_ DWORD_PTR dwRefData
    )
{
    PEDIT_CONTEXT context = (PEDIT_CONTEXT)GetProp(hwndDlg, L"EditSubclassContext");
    
    if (context == NULL)
        return DefSubclassProc(hwndDlg, uMsg, wParam, lParam);

    if (uMsg == WM_DESTROY)
    {
        NcAreaFreeGdiTheme(context);

        if (context->ImageList)
            ImageList_Destroy(context->ImageList);

#ifdef _UXTHEME_ENABLED_
        if (CloseThemeData_I && context->UxThemeHandle)
            CloseThemeData_I(context->UxThemeHandle);

        if (context->UxThemeModule)
            FreeLibrary(context->UxThemeModule);
#endif

        RemoveWindowSubclass(hwndDlg, NcAreaWndSubclassProc, 0);
        RemoveProp(hwndDlg, L"EditSubclassContext");
        PhFree(context);
        return TRUE;
    }

    switch (uMsg)
    {
    case WM_ERASEBKGND:
        return TRUE;
    case WM_SYSCOLORCHANGE:
    case WM_STYLECHANGED:
    case WM_THEMECHANGED:
        {    
        NcAreaFreeGdiTheme(context);
        NcAreaInitializeGdiTheme(context);

#ifdef _UXTHEME_ENABLED_
            NcAreaInitializeUxTheme(context);
#endif _UXTHEME_ENABLED_
        }
        break;
    case WM_NCCALCSIZE:
        {
            if (wParam)
            {
                LPNCCALCSIZE_PARAMS ncCalcSize = (NCCALCSIZE_PARAMS*)lParam;

                ncCalcSize->rgrc[0].right -= context->cxImgSize;
            }            
        }
        break;
    case WM_NCPAINT:
        {
            HDC hdc = NULL;
            RECT clientRect = { 0 };

            // Get the screen coordinates of the client window.
            GetClientRect(hwndDlg, &clientRect);
            // Adjust the coordinates (start from border edge).
            InflateRect(&clientRect, -context->CXBorder, -context->CYBorder);
   
            // Get the screen coordinates of the window.
            GetWindowRect(hwndDlg, &context->SearchButtonRect);
            // Adjust the coordinates (start from 0,0).
            OffsetRect(&context->SearchButtonRect, -context->SearchButtonRect.left, -context->SearchButtonRect.top); 
            // Get the position of the inserted button.
            NcAreaGetButtonRect(context, &context->SearchButtonRect);

            BOOL isFocused = (GetFocus() == hwndDlg);

            if (!(hdc = GetWindowDC(hwndDlg)))
                break;

            SetBkMode(hdc, TRANSPARENT);

            // Draw the themed background. 
#ifdef _UXTHEME_ENABLED_
            if (context->IsThemeActive)
            {
                if (isFocused)
                {
                    if (IsThemeBackgroundPartiallyTransparent_I(
                        context->UxThemeHandle,
                        EP_EDITBORDER_NOSCROLL,
                        EPSN_FOCUSED
                        ))
                    {
                        DrawThemeParentBackground_I(hwndDlg, hdc, NULL);
                    }

                    DrawThemeBackground_I(
                        context->UxThemeHandle,
                        hdc,
                        EP_EDITBORDER_NOSCROLL,
                        EPSN_FOCUSED,
                        &windowRect,
                        NULL
                        );
                }
#ifdef _HOTTRACK_ENABLED_
                else if (context->MouseInClient)
                {            
                    if (IsThemeBackgroundPartiallyTransparent_I(
                        context->UxThemeHandle,
                        EP_EDITBORDER_NOSCROLL,
                        EPSN_HOT
                        ))
                    {
                        DrawThemeParentBackground_I(hwndDlg, hdc, NULL);
                    }

                    DrawThemeBackground_I(
                        context->UxThemeHandle,
                        hdc,
                        EP_EDITBORDER_NOSCROLL,
                        EPSN_HOT,
                        &windowRect,
                        NULL
                        );
                }
#endif _HOTTRACK_ENABLED_
                else
                {
                    if (IsThemeBackgroundPartiallyTransparent_I(
                        context->UxThemeHandle,
                        EP_EDITBORDER_NOSCROLL,
                        EPSN_NORMAL
                        ))
                    {
                        DrawThemeParentBackground_I(hwndDlg, hdc, NULL);
                    }

                    DrawThemeBackground_I(
                        context->UxThemeHandle,
                        hdc,
                        EP_EDITBORDER_NOSCROLL,
                        EPSN_NORMAL,
                        &windowRect,
                        NULL
                        );
                }
            }
            else
#endif _UXTHEME_ENABLED_
            {
 
            }

            FillRect(hdc, &context->SearchButtonRect, context->BrushFill);

            //if (isFocused)
            //{
            //    FrameRect(hdc, &context->SearchButtonRect, context->BrushFocused);
            //}
            //else if (context->MouseInClient)
            //{
            //    FrameRect(hdc, &context->SearchButtonRect, context->BrushHot);
            //}
            //else
            //{
            //    FrameRect(hdc, &context->SearchButtonRect, context->BrushNormal);
            //}

            // Draw the image centered within the rect.
            if (SearchboxText->Length > 0)
            {
                ImageList_DrawEx(
                    context->ImageList,
                    0,
                    hdc,
                    context->SearchButtonRect.left,
                    context->SearchButtonRect.top + ((context->SearchButtonRect.bottom - context->SearchButtonRect.top) - context->ImageHeight) / 2,
                    0,
                    0,
                    context->BackgroundColorRef,
                    context->BackgroundColorRef,
                    ILD_NORMAL | ILD_TRANSPARENT
                    );
            }
            else
            {
                ImageList_DrawEx(
                    context->ImageList,
                    1,
                    hdc,
                    context->SearchButtonRect.left,
                    context->SearchButtonRect.top + ((context->SearchButtonRect.bottom - context->SearchButtonRect.top) - (context->ImageHeight - 1)) / 2, // Fix image offset by 1
                    0,
                    0,
                    context->BackgroundColorRef,
                    context->BackgroundColorRef,
                    ILD_NORMAL | ILD_TRANSPARENT
                    );
            }

            ReleaseDC(hwndDlg, hdc);
        }
        break;
    case WM_NCHITTEST:
        {
            POINT windowPoint = { 0 };
            RECT windowRect = { 0 };

            // Get the screen coordinates of the mouse.
            windowPoint.x = GET_X_LPARAM(lParam);
            windowPoint.y = GET_Y_LPARAM(lParam);

            // Get the position of the inserted button.
            GetWindowRect(hwndDlg, &windowRect);
            NcAreaGetButtonRect(context, &windowRect);

            // Check that the mouse is within the inserted button.
            if (PtInRect(&windowRect, windowPoint))
                return HTBORDER;
        }
        break;
    case WM_NCLBUTTONDOWN:
        {
            POINT windowPoint = { 0 };
            RECT windowRect = { 0 };

            // Get the screen coordinates of the mouse.
            windowPoint.x = GET_X_LPARAM(lParam);
            windowPoint.y = GET_Y_LPARAM(lParam);

            // Get the position of the inserted button.
            GetWindowRect(hwndDlg, &windowRect);
            NcAreaGetButtonRect(context, &windowRect);

            // Check that the mouse is within the inserted button.
            if (PtInRect(&windowRect, windowPoint))
            {
                SetCapture(hwndDlg);

                // Forward click notification.
                SendMessage(PhMainWndHandle, WM_COMMAND, MAKEWPARAM(context->CommandID, BN_CLICKED), 0);

                RedrawWindow(hwndDlg, NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
                return FALSE;
            }
        }
        break;
    case WM_LBUTTONUP:
        {
            if (GetCapture() == hwndDlg)
            {
                ReleaseCapture();

                RedrawWindow(hwndDlg, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
                return FALSE;
            }
        }
        break;
    case WM_KEYDOWN:
        {
            if (WindowsVersion < WINDOWS_VISTA)
            {
                // Handle CTRL+A below Vista.
                if (GetKeyState(VK_CONTROL) & VK_LCONTROL && wParam == 'A')
                {
                    Edit_SetSel(hwndDlg, 0, -1);
                    return FALSE;
                }
            }
        }
        break;
    case WM_CUT:
    case WM_CLEAR:
    case WM_PASTE:
    case WM_UNDO:
    case WM_KEYUP:
    case WM_SETTEXT:
    case WM_KILLFOCUS:
        RedrawWindow(hwndDlg, NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
        break;
#ifdef _HOTTRACK_ENABLED_
    case WM_MOUSELEAVE:
        {
            context->MouseInClient = FALSE;

            RedrawWindow(hwndDlg, NULL, NULL, RDW_FRAME | RDW_INVALIDATE);
        }
        break;
    case WM_MOUSEMOVE:
        {
            //if (!context->MouseInClient)
            TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT) };
            tme.dwFlags = TME_LEAVE | TME_NONCLIENT;
            tme.hwndTrack = hwndDlg;

            context->MouseInClient = TRUE;
            RedrawWindow(hwndDlg, NULL, NULL, RDW_FRAME | RDW_INVALIDATE);

            TrackMouseEvent(&tme);
        }
        break;
#endif _HOTTRACK_ENABLED_
    }

    return DefSubclassProc(hwndDlg, uMsg, wParam, lParam);
}

HBITMAP LoadImageFromResources(
    _In_ UINT Width,
    _In_ UINT Height,
    _In_ PCWSTR Name
    )
{
    UINT width = 0;
    UINT height = 0;
    UINT frameCount = 0;
    BOOLEAN isSuccess = FALSE;
    ULONG resourceLength = 0;
    HGLOBAL resourceHandle = NULL;
    HRSRC resourceHandleSource = NULL;
    WICInProcPointer resourceBuffer = NULL;

    BITMAPINFO bitmapInfo = { 0 };
    HBITMAP bitmapHandle = NULL;
    PBYTE bitmapBuffer = NULL;

    IWICStream* wicStream = NULL;
    IWICBitmapSource* wicBitmapSource = NULL;
    IWICBitmapDecoder* wicDecoder = NULL;
    IWICBitmapFrameDecode* wicFrame = NULL;
    
    IWICBitmapScaler* wicScaler = NULL;
    WICPixelFormatGUID pixelFormat;

    WICRect rect = { 0, 0, Width, Height };

    static PH_INITONCE initOnce = PH_INITONCE_INIT;
    static IWICImagingFactory* wicFactory = NULL;

    __try
    {
        if (PhBeginInitOnce(&initOnce))
        {
            // Create the ImagingFactory
            if (FAILED(CoCreateInstance(&CLSID_WICImagingFactory1, NULL, CLSCTX_INPROC_SERVER, &IID_IWICImagingFactory, (PVOID*)&wicFactory)))
            {
                PhEndInitOnce(&initOnce);
                __leave;
            }

            PhEndInitOnce(&initOnce);
        }

        // Find the resource
        if ((resourceHandleSource = FindResource(PluginInstance->DllBase, Name, L"PNG")) == NULL)
            __leave;

        // Get the resource length
        resourceLength = SizeofResource(PluginInstance->DllBase, resourceHandleSource);

        // Load the resource
        if ((resourceHandle = LoadResource(PluginInstance->DllBase, resourceHandleSource)) == NULL)
            __leave;

        if ((resourceBuffer = (WICInProcPointer)LockResource(resourceHandle)) == NULL)
            __leave;

        // Create the Stream
        if (FAILED(IWICImagingFactory_CreateStream(wicFactory, &wicStream)))
            __leave;

        // Initialize the Stream from Memory
        if (FAILED(IWICStream_InitializeFromMemory(wicStream, resourceBuffer, resourceLength)))
            __leave;

        if (FAILED(IWICImagingFactory_CreateDecoder(wicFactory, &GUID_ContainerFormatPng, NULL, &wicDecoder)))
            __leave;

        if (FAILED(IWICBitmapDecoder_Initialize(wicDecoder, (IStream*)wicStream, WICDecodeMetadataCacheOnLoad)))
            __leave;

        // Get the Frame count
        if (FAILED(IWICBitmapDecoder_GetFrameCount(wicDecoder, &frameCount)) || frameCount < 1)
            __leave;

        // Get the Frame
        if (FAILED(IWICBitmapDecoder_GetFrame(wicDecoder, 0, &wicFrame)))
            __leave;

        // Get the WicFrame image format
        if (FAILED(IWICBitmapFrameDecode_GetPixelFormat(wicFrame, &pixelFormat)))
            __leave;

        // Check if the image format is supported:
        if (IsEqualGUID(&pixelFormat, &GUID_WICPixelFormat32bppPBGRA)) // GUID_WICPixelFormat32bppRGB
        {
            wicBitmapSource = (IWICBitmapSource*)wicFrame;
        }
        else
        {
            // Convert the image to the correct format:
            if (FAILED(WICConvertBitmapSource(&GUID_WICPixelFormat32bppPBGRA, (IWICBitmapSource*)wicFrame, &wicBitmapSource)))
                __leave;

            IWICBitmapFrameDecode_Release(wicFrame);
        }

        bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bitmapInfo.bmiHeader.biWidth = rect.Width;
        bitmapInfo.bmiHeader.biHeight = -((LONG)rect.Height);
        bitmapInfo.bmiHeader.biPlanes = 1;
        bitmapInfo.bmiHeader.biBitCount = 32;
        bitmapInfo.bmiHeader.biCompression = BI_RGB;

        HDC hdc = CreateCompatibleDC(NULL);
        bitmapHandle = CreateDIBSection(hdc, &bitmapInfo, DIB_RGB_COLORS, (PVOID*)&bitmapBuffer, NULL, 0);
        ReleaseDC(NULL, hdc);

        // Check if it's the same rect as the requested size.
        //if (width != rect.Width || height != rect.Height)
        if (FAILED(IWICImagingFactory_CreateBitmapScaler(wicFactory, &wicScaler)))
            __leave;
        if (FAILED(IWICBitmapScaler_Initialize(wicScaler, wicBitmapSource, rect.Width, rect.Height, WICBitmapInterpolationModeFant)))
            __leave;
        if (FAILED(IWICBitmapScaler_CopyPixels(wicScaler, &rect, rect.Width * 4, rect.Width * rect.Height * 4, bitmapBuffer)))
            __leave;

        isSuccess = TRUE;
    }
    __finally
    {
        if (wicScaler)
        {
            IWICBitmapScaler_Release(wicScaler);
        }

        if (wicBitmapSource)
        {
            IWICBitmapSource_Release(wicBitmapSource);
        }

        if (wicStream)
        {
            IWICStream_Release(wicStream);
        }

        if (wicDecoder)
        {
            IWICBitmapDecoder_Release(wicDecoder);
        }

        if (wicFactory)
        {
           // IWICImagingFactory_Release(wicFactory);
        }

        if (resourceHandle)
        {
            FreeResource(resourceHandle);
        }
    }

    return bitmapHandle;
}

HWND CreateSearchControl(
    _In_ UINT CommandID
    )
{
    PEDIT_CONTEXT context = (PEDIT_CONTEXT)PhAllocate(sizeof(EDIT_CONTEXT));
    memset(context, 0, sizeof(EDIT_CONTEXT));

    context->cxImgSize = 22;
    context->CommandID = CommandID;

    SearchboxText = PhReferenceEmptyString();

    NcAreaInitializeGdiTheme(context);
    NcAreaInitializeImageList(context);

    // Create the SearchBox window.
    context->WindowHandle = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        WC_EDIT,
        NULL,
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | ES_LEFT,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        ReBarHandle,
        NULL,
        (HINSTANCE)PluginInstance->DllBase,
        NULL
        );

    // Set our window context data.
    SetProp(context->WindowHandle, L"EditSubclassContext", (HANDLE)context);

    // Subclass the Edit control window procedure.
    SetWindowSubclass(context->WindowHandle, NcAreaWndSubclassProc, 0, (ULONG_PTR)context);

#ifdef _UXTHEME_ENABLED_
    SendMessage(context->WindowHandle, WM_THEMECHANGED, 0, 0);
#endif _UXTHEME_ENABLED_

    // Force the edit control to update its non-client area.
    RedrawWindow(context->WindowHandle, NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);

    return context->WindowHandle;
}