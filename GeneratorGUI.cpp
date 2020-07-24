// GeneratorGUI.cpp : Определяет точку входа для приложения.
//

#define _CRT_SECURE_NO_WARNINGS
#include "framework.h"
#include "GeneratorGUI.h"
#include "stdlib.h"
#include "math.h"
#include "MyGenerator.h"
#include "sstream"
#include "Commctrl.h"
#include "Shobjidl.h"
#include <comdef.h>

#define MAX_LOADSTRING 100

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
HMENU hMenu;
HWND hWnd;
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

bool print_bits = false;
bool is_whole = true;
unsigned char number_of_steps = 7;
uint32_t params[] = { 257, 2579, 25703, 257003, 2570011, 25700033, 257000011 };
uint32_t seed_number;
uint32_t* seed = NULL;
FILE* input = NULL;
FILE* output = NULL;

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    Parameters(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    SeedFromMouse(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    SeedFromNumber(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    setlocale(0, "");

    srand(0);
    rand();
    seed_number = rand();
    uint32_t bytes_count = ceil(params[0] / (8.0 * sizeof(uint32_t)));
    seed = (uint32_t*)malloc(bytes_count * sizeof(uint32_t));
    for (uint32_t i = 0; i < bytes_count; i++) {
        seed[i] = seed_number;
    }
    mySRandFromSeed(number_of_steps, params, seed);

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Разместите код здесь.

    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDD_DIALOG1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDD_DIALOG1));

    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = DLGWINDOWEXTRA;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GENERATORGUI));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = nullptr;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW (&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной
    hWnd = CreateDialogW(hInst, MAKEINTRESOURCE(IDD_DIALOG1), nullptr, WndProc);
    hMenu = LoadMenuW(hInstance, MAKEINTRESOURCE(IDC_GENERATORGUI));
    SetMenu(hWnd, hMenu);
    if (!hWnd) {
        return FALSE;
    }
    ShowWindow(hWnd, nCmdShow);
    CheckRadioButton(hWnd, IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
    CheckRadioButton(hWnd, IDC_RADIO3, IDC_RADIO4, IDC_RADIO4);
    SetDlgItemInt(hWnd, IDC_EDIT5 , 1024*1024*500, FALSE);
    SetDlgItemInt(hWnd, IDC_EDIT2, 0, FALSE);
    SetDlgItemInt(hWnd, IDC_EDIT3, UINT32_MAX, FALSE);
    CheckMenuRadioItem(hMenu, IDM_SEED_FROM_FILE, IDM_SEED_FROM_MOUSE, IDM_SEED_FROM_NUMBER, 0);
    return TRUE;
}

static void readSeedFromInput() {
    free(seed);
    uint32_t bytes_count = ceil(params[0] / (8.0 * sizeof(uint32_t)));
    seed = (uint32_t*)malloc(bytes_count * sizeof(uint32_t));
    fseek(input, 0, SEEK_END);
    off_t fileLength = ftell(input);
    uint32_t count = 0;
    while (count < bytes_count) {
        fseek(input, 0, SEEK_SET);
        for (uint32_t i = 0; i < fileLength && count < bytes_count; i += sizeof(uint32_t)) {
            fread(&(seed[count]), sizeof(uint32_t), 1, input);
            count++;
        }
    }
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    uint32_t minimum, maximum;
    double number;
    std::wostringstream wcharStream;
    BOOL result;
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Разобрать выбор в меню:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_INSTRUCTION:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_INSTRUCTION), hWnd, About);
                break;
            case IDM_PARAMETERS:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_PARAMETERS), hWnd, Parameters);
                break;
            case IDM_SEED_FROM_FILE: {
                IFileDialog* inputFile = NULL;
                CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&inputFile));
                inputFile->Show(NULL);
                IShellItem* fileResult;
                inputFile->GetResult(&fileResult);
                if (!SUCCEEDED(fileResult)) {
                    MessageBox(hWnd, TEXT("Невозможно открыть файл"), TEXT("Ошибка"), MB_OK | MB_ICONEXCLAMATION);
                    inputFile->Release();
                    break;
                }
                PWSTR filePath;
                fileResult->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
                if (input != NULL) {
                    fclose(input);
                }
                _bstr_t b(filePath);
                const char* c = b;
                input = fopen(c, "rb");
                if (input != NULL) {
                    readSeedFromInput();
                    CheckMenuRadioItem(hMenu, IDM_SEED_FROM_FILE, IDM_SEED_FROM_MOUSE, IDM_SEED_FROM_FILE, 0);
                }
                CoTaskMemFree(filePath);
                fileResult->Release();
                inputFile->Release();
                break;
            }
            case IDM_SEED_FROM_MOUSE:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_SEED_BY_MOUSE), hWnd, SeedFromMouse);
                break;
            case IDM_SEED_FROM_NUMBER:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_SEED_BY_NUMBER), hWnd, SeedFromNumber);
                break;
            case IDC_BUTTON1: {
                IFileDialog* outputFile = NULL;
                CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&outputFile));
                outputFile->Show(NULL);
                IShellItem* fileResult;
                outputFile->GetResult(&fileResult);
                if (!SUCCEEDED(fileResult)) {
                    MessageBox(hWnd, TEXT("Невозможно открыть файл"), TEXT("Ошибка"), MB_OK | MB_ICONEXCLAMATION);
                    outputFile->Release();
                    break;
                }
                PWSTR filePath;
                fileResult->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
                _bstr_t b(filePath);
                const char* c = b;
                output = fopen(c, "wb");
                if (output != NULL) {
                    uint32_t length = GetDlgItemInt(hWnd, IDC_EDIT5, nullptr, FALSE);
                    for (uint32_t i = 0; i < length; i += sizeof(uint32_t)) {
                        uint32_t random = myRand();
                        if (!print_bits) {
                            fwrite(&random, sizeof(uint32_t), 1, output);
                        } else {
                            for (uint32_t j = 0; j < sizeof(uint32_t) * 8; j++) {
                                unsigned char bit = '0' + ((random >> (sizeof(uint32_t) * 8 - j - 1)) & 1);
                                fwrite(&bit, 1, 1, output);
                            }
                        }
                    }
                    fclose(output);
                }
                CoTaskMemFree(filePath);
                fileResult->Release();
                outputFile->Release();
                break;
            }
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case IDC_RADIO1:
                is_whole = true;
                break;
            case IDC_RADIO2:
                is_whole = false;
                break;
            case IDC_RADIO3:
                print_bits = true;
                break;
            case IDC_RADIO4:
                print_bits = false;
                break;
            case IDC_BUTTON2:
                number = myRand();
                minimum = GetDlgItemInt(hWnd, IDC_EDIT2, nullptr, FALSE);
                maximum = GetDlgItemInt(hWnd, IDC_EDIT3, nullptr, FALSE);
                number = number / (MAXUINT32) * (maximum - minimum) + minimum;
                if (is_whole) {
                    SetDlgItemInt(hWnd, IDC_EDIT4, round(number), FALSE);
                } else {
                    wcharStream << number;
                    SetDlgItemText(hWnd, IDC_EDIT4, wcharStream.str().c_str());
                }
                break;
            case IDC_EDIT2:
                minimum = GetDlgItemInt(hWnd, IDC_EDIT2, &result, FALSE);
                maximum = GetDlgItemInt(hWnd, IDC_EDIT3, nullptr, FALSE);
                if (result == 0) {
                    minimum = UINT32_MAX;
                    SetDlgItemInt(hWnd, IDC_EDIT2, minimum, FALSE);
                }
                if (minimum > maximum) {
                    SetDlgItemInt(hWnd, IDC_EDIT2, maximum, FALSE);
                }
                if (minimum < 0) {
                    SetDlgItemInt(hWnd, IDC_EDIT2, 0, FALSE);
                }
                break;
            case IDC_EDIT3:
                minimum = GetDlgItemInt(hWnd, IDC_EDIT2, nullptr, FALSE);
                maximum = GetDlgItemInt(hWnd, IDC_EDIT3, &result, FALSE);
                if (result == 0) {
                    maximum = UINT32_MAX;
                    SetDlgItemInt(hWnd, IDC_EDIT3, maximum, FALSE);
                }
                if (maximum < minimum) {
                    SetDlgItemInt(hWnd, IDC_EDIT3, minimum, FALSE);
                }
                if (maximum < 0) {
                    SetDlgItemInt(hWnd, IDC_EDIT3, 0, FALSE);
                }
                break;
            case IDC_EDIT5:
                if (GetDlgItemInt(hWnd, IDC_EDIT5, &result, FALSE) < 0) {
                    SetDlgItemInt(hWnd, IDC_EDIT5, 0, FALSE);
                }
                if (result == 0) {
                    SetDlgItemInt(hWnd, IDC_EDIT5, UINT32_MAX, FALSE);
                }
            default:
                break;
            }
        }
        break;
    case WM_CLOSE:
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        break;
    }
    return 0;
}

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

UINT editBoxes[] = {IDC_EDIT1, IDC_EDIT2, IDC_EDIT3, IDC_EDIT4, IDC_EDIT6, IDC_EDIT7, IDC_EDIT8};

INT_PTR CALLBACK Parameters(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    BOOL result;
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_NOTIFY:
        if (LOWORD(wParam) == IDC_SPIN1) {
            int iDelta = ((NMUPDOWN*)lParam)->iDelta;
            uint32_t number = GetDlgItemInt(hDlg, IDC_EDIT9, nullptr, FALSE);
            if (iDelta < 0) {
                number++;
            } else if (iDelta > 0) {
                number--;
            }
            if (number > 7) {
                number = 7;
            }
            else if (number < 1) {
                number = 1;                
            }
            SetDlgItemInt(hDlg, IDC_EDIT9, number, FALSE);
            for (int i = 0; i < number; i++) {
                HWND editBox = GetDlgItem(hDlg, editBoxes[i]);
                if (GetDlgItemInt(hDlg, editBoxes[i], nullptr, FALSE) < 32) {
                    SetDlgItemInt(hDlg, editBoxes[i], 32, FALSE);
                }
                EnableWindow(editBox, TRUE);
            }
            
            for (int i = number; i < 7; i++) {
                HWND editBox = GetDlgItem(hDlg, editBoxes[i]);
                EnableWindow(editBox, FALSE);
            }

            return (INT_PTR)TRUE;
        }
    case WM_INITDIALOG:
        SetDlgItemInt(hDlg, IDC_EDIT9, number_of_steps, FALSE);
        for (int i = 0; i < number_of_steps; i++) {
            HWND editBox = GetDlgItem(hDlg, editBoxes[i]);
            SetDlgItemInt(hDlg, editBoxes[i], params[i], FALSE);
            EnableWindow(editBox, TRUE);
        }
        for (int i = number_of_steps; i < 7; i++) {
            HWND editBox = GetDlgItem(hDlg, editBoxes[i]);
            SetDlgItemInt(hDlg, editBoxes[i], 0, FALSE);
            EnableWindow(editBox, FALSE);
        }
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            if (LOWORD(wParam) == IDOK) {
                number_of_steps = GetDlgItemInt(hDlg, IDC_EDIT9, nullptr, FALSE);
                for (int i = 0; i < number_of_steps; i++) {
                    params[i] = GetDlgItemInt(hDlg, editBoxes[i], nullptr, FALSE);
                }
                if (input != NULL) {
                    readSeedFromInput();
                } else if (seed[0] == seed[1]) {
                    uint32_t seed_number = seed[0];
                    free(seed);
                    uint32_t bytes_count = ceil(params[0] / (8.0 * sizeof(uint32_t)));
                    seed = (uint32_t*)malloc(bytes_count * sizeof(uint32_t));
                    for (uint32_t i = 0; i < bytes_count; i++) {
                        seed[i] = seed_number;
                    }
                } else {
                    seed = (uint32_t*)realloc(seed, ceil(params[0] / (8.0 * sizeof(uint32_t))));
                }
                mySRandFromSeed(number_of_steps, params, seed);
            }
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        } else if (LOWORD(wParam) == IDC_EDIT9) {
            uint32_t number = GetDlgItemInt(hDlg, IDC_EDIT9, nullptr, FALSE);
            if (number > 7) {
                number = 7;
                SetDlgItemInt(hDlg, IDC_EDIT9, number, FALSE);
            } else if (number < 1) {
                number = 1;
                SetDlgItemInt(hDlg, IDC_EDIT9, number, FALSE);
            }
            for (int i = 0; i < number; i++) {
                HWND editBox = GetDlgItem(hDlg, editBoxes[i]);
                if (GetDlgItemInt(hDlg, editBoxes[i], nullptr, FALSE) < 32) {
                    SetDlgItemInt(hDlg, editBoxes[i], 32, FALSE);
                }
                EnableWindow(editBox, TRUE);
            }
            for (int i = number; i < 7; i++) {
                HWND editBox = GetDlgItem(hDlg, editBoxes[i]);
                EnableWindow(editBox, FALSE);
            }
            return (INT_PTR)TRUE;
        } else {
            uint32_t number = GetDlgItemInt(hDlg, IDC_EDIT9, nullptr, FALSE);
            for (int i = 0; i < number; i++) {
                if (LOWORD(wParam) == editBoxes[i]) {
                    if (GetDlgItemInt(hDlg, editBoxes[i], &result, FALSE) < 32) {
                        SetDlgItemInt(hDlg, editBoxes[i], 32, FALSE);
                    }
                    if (result == 0) {
                        SetDlgItemInt(hDlg, editBoxes[i], UINT32_MAX, FALSE);
                    }
                    return (INT_PTR)TRUE;
                }
            }
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK SeedFromNumber(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        SetDlgItemInt(hDlg, IDC_SEED, 0, FALSE);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            if (LOWORD(wParam) == IDOK) {
                uint32_t seed_number = GetDlgItemInt(hDlg, IDC_SEED, nullptr, FALSE);
                if (seed_number > 0) {
                    if (input != NULL) {
                        fclose(input);
                        input = NULL;
                    }
                    free(seed);
                    uint32_t bytes_count = ceil(params[0] / (8.0 * sizeof(uint32_t)));
                    seed = (uint32_t*)malloc(bytes_count * sizeof(uint32_t));
                    for (uint32_t i = 0; i < bytes_count; i++) {
                        seed[i] = seed_number;
                    }
                    mySRandFromSeed(number_of_steps, params, seed);
                    CheckMenuRadioItem(hMenu, IDM_SEED_FROM_FILE, IDM_SEED_FROM_MOUSE, IDM_SEED_FROM_NUMBER, 0);
                }
            }
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == IDC_SEED) {
            BOOL result;
            GetDlgItemInt(hDlg, IDC_SEED, &result, FALSE);
            if (result == 0) {
                SetDlgItemInt(hDlg, IDC_SEED, UINT32_MAX, FALSE);
            }
        }
        break;
    }
    return (INT_PTR)FALSE;
}

uint32_t bytes;
uint32_t index;

INT_PTR CALLBACK SeedFromMouse(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    uint32_t value = 0;
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        if (input != NULL) {
            fclose(input);
            input = NULL;
        }
        free(seed);
        index = 0;
        bytes = ceil(params[0] / (8.0 * sizeof(uint32_t)));
        seed = (uint32_t*)malloc(bytes * sizeof(uint32_t));
        return (INT_PTR)TRUE;

    case WM_MOUSEMOVE:
        value = lParam;
        break;
    case WM_SYSKEYDOWN:
        value = wParam;
        break;
    case WM_KEYDOWN:
        value = wParam;
        break;
    }
    if (value > 0) {
        seed[index] = value;
        index++;
        if (index == bytes) {
            mySRandFromSeed(number_of_steps, params, seed);
            CheckMenuRadioItem(hMenu, IDM_SEED_FROM_FILE, IDM_SEED_FROM_MOUSE, IDM_SEED_FROM_MOUSE, 0);
            EndDialog(hDlg, LOWORD(wParam));
        }
        return (INT_PTR)TRUE;
    }
    return (INT_PTR)FALSE;
}