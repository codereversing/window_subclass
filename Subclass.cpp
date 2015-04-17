#include <Windows.h>

typedef struct _PROCESSWNDINFO {
    HWND hWnd;
    HMENU hMenuBar;
    HMENU hAddedMenu;
    LONG_PTR PrevWndProc;
} PROCESSWNDINFO, *LPPROCESSWNDINFO;

const DWORD MENUITEM_ID = 1234;
PROCESSWNDINFO g_WindowInfo;

LRESULT CALLBACK SublassWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    switch(Msg) {
        case WM_COMMAND:
            switch(wParam) {
            case MENUITEM_ID:
                MessageBox(NULL, L"Added Handler!", L"New item", MB_ICONASTERISK);
                break;
            }
        break;

    default: break;
    }
    return CallWindowProc((WNDPROC)g_WindowInfo.PrevWndProc, hWnd, Msg, wParam, lParam);
}

BOOL CALLBACK EnumWindowProc(HWND hWnd, LPARAM processId) {
    const INT WINDOW_LENGTH = 32;
    WCHAR windowClass[WINDOW_LENGTH] = {0};
    GetClassName(hWnd, windowClass, sizeof(windowClass));
    if(wcscmp(windowClass, L"GDI+ Hook Window Class") == 0)
        return TRUE;
    DWORD windowProcessId = 0;
    (VOID)GetWindowThreadProcessId(hWnd, &windowProcessId);
    if(windowProcessId == (DWORD)processId) {
        g_WindowInfo.hWnd = hWnd;
        return FALSE;
    }
    return TRUE;
}

BOOL GetWindowProperties(LPPROCESSWNDINFO windowInfo) {
    EnumWindows((WNDENUMPROC)EnumWindowProc, (LPARAM)GetCurrentProcessId());
    if(windowInfo->hWnd != NULL) {
        windowInfo->hMenuBar = GetMenu(g_WindowInfo.hWnd);
        windowInfo->PrevWndProc = GetWindowLongPtr(windowInfo->hWnd, GWLP_WNDPROC);
        return TRUE;
    }
    return FALSE;
}

HMENU AddNewMenu(LPPROCESSWNDINFO windowInfo, LPCWSTR title) {
    HMENU hNewMenu = CreateMenu();
    if(hNewMenu != NULL && windowInfo->hMenuBar != NULL)
        if(AppendMenu(windowInfo->hMenuBar, MF_STRING | MF_POPUP, (UINT_PTR)hNewMenu, title) != 0)
            return hNewMenu;
    return NULL;
}

BOOL AddNewMenuItem(LPPROCESSWNDINFO windowInfo, HMENU hMenu, LPCWSTR title, const DWORD id) {
    BOOL ret = FALSE;
    if(hMenu != NULL)
        ret = AppendMenu(hMenu, MF_STRING, id, title);
    if(windowInfo->hMenuBar != NULL)
        DrawMenuBar(windowInfo->hWnd);
    return ret;
}

int APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    if(reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        if(GetWindowProperties(&g_WindowInfo) == TRUE) {
            g_WindowInfo.hAddedMenu = AddNewMenu(&g_WindowInfo, L"Test");
            AddNewMenuItem(&g_WindowInfo, g_WindowInfo.hAddedMenu, L"Hello", MENUITEM_ID);
            g_WindowInfo.PrevWndProc = SetWindowLongPtr(g_WindowInfo.hWnd, GWLP_WNDPROC, (LONG_PTR)SublassWndProc);
            }
        }
    return TRUE;
}