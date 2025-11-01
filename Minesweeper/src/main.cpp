#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Windowsx.h>
#include <string>
#include <array>
#include <dwmapi.h>

#include "Game.h"

#pragma comment(lib, "dwmapi.lib")

// A define to use if we want dark mode or not, 1=true, 0=false. The menubar cannot easily be drawn in dark mode
#define DARK_MODE 1

//Global variabler-------------------------------------------------
Game game;

HDC hdc;
HPEN gameplanPen;
HPEN flagPen;
HFONT mainFont;
HFONT normalTextFont;

int gameWindowSize = 500;
LPCWSTR WINDOW_CLASS_NAME = L"MinesweeperGame";

//Funktionsdeklarationer-----------------------------------------------------
ATOM RegisterWindowClass(HINSTANCE);
BOOL InitializeWindow(HINSTANCE hInstance);
LRESULT CALLBACK winproc(HWND, UINT, WPARAM, LPARAM);

void Init(HWND);
void Cleanup(HWND);


#ifdef MS_DIST
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lp, _In_ int n)
#else
int main()
#endif
{
	HINSTANCE hi = GetModuleHandle(nullptr);
	if (!RegisterWindowClass(hi) || !InitializeWindow(hi))
	{
		MessageBoxEx(NULL, L"Program can not be started", L"ERROR", MB_ICONERROR | MB_OK, 0);
		return 0;
	}

	MSG msg; 
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

ATOM RegisterWindowClass(HINSTANCE hi)
{
	WNDCLASSEX wcex{};

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.hInstance = hi; 
	wcex.lpszClassName = WINDOW_CLASS_NAME; // Window class name
	wcex.lpfnWndProc = winproc; 
	wcex.style = /*CS_DBLCLKS | */ CS_HREDRAW | CS_VREDRAW; 
	wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION); // Window icon 32*32
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION); // Window icon 16*16
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW); // Cursor appearance 
	wcex.lpszMenuName = NULL; // No menu
	wcex.cbClsExtra = 0; 
	wcex.cbWndExtra = 0; 
#if DARK_MODE 1
	wcex.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(32, 32, 32)); 
#else 
	wcex.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH); 
#endif

	return RegisterClassEx(&wcex);
}

BOOL InitializeWindow(HINSTANCE hInstance)
{
	DWORD windowStyle = WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME | WS_MAXIMIZEBOX);// Set the style of the window, meaning, a window with non-draggable borders and no maximize button
	int appWidth = gameWindowSize + 300; // Set the size of the window
	int appHeight = gameWindowSize;

	RECT wndRect{ 0, 0, appWidth, appHeight };
	AdjustWindowRect(&wndRect, windowStyle, FALSE); // Client width and height to be the values above

	appWidth = wndRect.right - wndRect.left;
	appHeight = wndRect.bottom - wndRect.top;

	HWND hWnd = CreateWindowEx(
		0, 
		WINDOW_CLASS_NAME, 
		L"Minesweeper", 
		windowStyle, 
		((GetSystemMetrics(SM_CXSCREEN) - appWidth) >> 1), //Sets the window in the middle of the monitor 
		((GetSystemMetrics(SM_CYSCREEN) - appHeight) >> 1),
		appWidth,
		appHeight,
		NULL, 
		NULL, 
		hInstance, 
		NULL // Window creation data
	);

	if (!hWnd)
		return FALSE;

	ShowWindow(hWnd, SW_NORMAL);
	UpdateWindow(hWnd);

	return TRUE;
}

//------------------------------------------------
// En windowsprocedur som anropas fr�n winmain och tar hand om meddelanden fr�n applikationen
// hWnd		- hanterare till aktuellt f�nster
// message	- meddelandet som skall hanteras
// wParam	- 32bit WORD parameter med ytterligare information om vad som skall g�ras.
// lParam	- 32bit LONG parameter
// wParam och lParam inneh�ll varierar beroende p� vad meddelandet skall utf�ra
//------------------------------------------------
LRESULT CALLBACK winproc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps; //Anv�nds av WM_PAINT

	switch (message)
	{
	case WM_CREATE: //WM_CREATE k�rs innan applikationen visas och endast d�!
	{
		Init(hWnd);
		break;
	}
	case WM_KEYDOWN:
	{
		if (wParam == VK_ESCAPE)				// If we press escape
		{
			SendMessage(hWnd, WM_CLOSE, 0, 0);	// Send a close message
			break;
		}
		if (wParam == 'R')
			game.Reset();

		InvalidateRect(hWnd, NULL, TRUE); // Everytime we click we clear the screen.

		break;
	}
	case WM_LBUTTONDOWN: //WM_LBUTTONDOWN k�rs n�r du trycker p� v�nster musknapp
	{
		int xpos = GET_X_LPARAM(lParam);
		int ypos = GET_Y_LPARAM(lParam);

		
		if (xpos >= gameWindowSize)
			break;
		if (ypos >= gameWindowSize)
			break;
		int xCell = xpos / (gameWindowSize / CELLS_WIDTH);
		int yCell = ypos / (gameWindowSize / CELLS_WIDTH);

		game.TryExpose(xCell, yCell);
		

		InvalidateRect(hWnd, NULL, TRUE);

		break;
	}
	case WM_RBUTTONDOWN:
	{
		int xpos = GET_X_LPARAM(lParam);
		int ypos = GET_Y_LPARAM(lParam);

		if (xpos >= gameWindowSize)
			break;
		if (ypos >= gameWindowSize)
			break;
		int xCell = xpos / (gameWindowSize / CELLS_WIDTH);
		int yCell = ypos / (gameWindowSize / CELLS_WIDTH);

		game.TryPlaceFlag(xCell, yCell);


		InvalidateRect(hWnd, NULL, TRUE);

		break;
	}
	case WM_PAINT:
	{
		BeginPaint(hWnd, &ps);

		SelectObject(hdc, gameplanPen);
		int cellPixelSize = (gameWindowSize / CELLS_WIDTH);
		for (int i = 0; i <= gameWindowSize; i += cellPixelSize)	// Draw the boards vertical lines 
		{
			MoveToEx(hdc, i, 0, NULL);
			LineTo(hdc, i, gameWindowSize);
		}
		for (int i = 0; i <= gameWindowSize; i += cellPixelSize) // Draw the boards horizontal lines 
		{
			MoveToEx(hdc, 0, i, NULL);
			LineTo(hdc, gameWindowSize, i);
		}

		{
			SelectFont(hdc, normalTextFont);
			SetTextAlign(hdc, TA_CENTER);
			std::string flags = "Flags left: " + std::to_string(game.GetAmountFlagsLeft());
			TextOutA(hdc, gameWindowSize + 150, 50, flags.c_str(), flags.size());
		}

		{
			SelectFont(hdc, normalTextFont);
			SetTextAlign(hdc, TA_CENTER);
			std::string flags = "m_RevealedSquaresAmount: " + std::to_string(game.GetAmountRevealed());
			TextOutA(hdc, gameWindowSize + 150, 250, flags.c_str(), flags.size());
		}

		if (game.GetLost())
		{
			SelectFont(hdc, normalTextFont);
			SetTextAlign(hdc, TA_CENTER);
			std::string lost = "You Lost!";
			std::string restart = "Press 'R' to restart!";
			TextOutA(hdc, gameWindowSize + 150, 100, lost.c_str(), lost.size());
			TextOutA(hdc, gameWindowSize + 150, 140, restart.c_str(), restart.size());
		}
		if (game.GetWon())
		{
			SelectFont(hdc, normalTextFont);
			SetTextAlign(hdc, TA_CENTER);
			std::string lost = "You Won!";
			std::string restart = "Press 'R' to restart!";
			TextOutA(hdc, gameWindowSize + 150, 100, lost.c_str(), lost.size());
			TextOutA(hdc, gameWindowSize + 150, 140, restart.c_str(), restart.size());
		}

		
		int padding = 5;
		for (const DrawableObject& drawable : game.GetCells())
		{
			int xUpLeft = drawable.CellPosition.x * cellPixelSize;
			int yUpLeft = drawable.CellPosition.y * cellPixelSize;
			if (drawable.Type == DrawableType::NonRevealed)
			{
				SelectObject(hdc, flagPen);
				
				RECT rect{ xUpLeft, yUpLeft, xUpLeft + cellPixelSize, yUpLeft + cellPixelSize };
				int greenColor = ((drawable.CellPosition.x + drawable.CellPosition.y) % 2 == 0) ? 210 : 180;
				HBRUSH brush = CreateSolidBrush(RGB(40, greenColor, 40));
				FillRect(hdc, &rect, brush);
				DeleteBrush(brush);
			}
			if (drawable.Type == DrawableType::Flag)
			{
				RECT rect{ xUpLeft, yUpLeft, xUpLeft + cellPixelSize, yUpLeft + cellPixelSize };
				int greenColor = ((drawable.CellPosition.x + drawable.CellPosition.y) % 2 == 0) ? 210 : 180;
				HBRUSH brush = CreateSolidBrush(RGB(40, greenColor, 40));
				FillRect(hdc, &rect, brush);
				DeleteBrush(brush);

				SelectObject(hdc, flagPen);
				MoveToEx(hdc, (drawable.CellPosition.x * cellPixelSize + cellPixelSize) - padding, (drawable.CellPosition.y * cellPixelSize) + padding, NULL);
				LineTo(hdc, (drawable.CellPosition.x * cellPixelSize + cellPixelSize) - padding, (drawable.CellPosition.y * cellPixelSize + cellPixelSize) - padding);
				MoveToEx(hdc, (drawable.CellPosition.x * cellPixelSize + cellPixelSize) - padding, (drawable.CellPosition.y * cellPixelSize + cellPixelSize) - padding, NULL);
				LineTo(hdc, (drawable.CellPosition.x * cellPixelSize) + padding, (drawable.CellPosition.y * cellPixelSize + cellPixelSize / 2));
				MoveToEx(hdc, (drawable.CellPosition.x * cellPixelSize) + padding, (drawable.CellPosition.y * cellPixelSize + cellPixelSize / 2), NULL);
				LineTo(hdc, (drawable.CellPosition.x * cellPixelSize + cellPixelSize) - padding, (drawable.CellPosition.y * cellPixelSize) + padding);
			}
			else if (drawable.Type == DrawableType::Bomb)
			{
				SelectObject(hdc, flagPen);
				MoveToEx(hdc, (drawable.CellPosition.x * cellPixelSize + cellPixelSize) - padding, (drawable.CellPosition.y * cellPixelSize) + padding, NULL);
				Arc(hdc, drawable.CellPosition.x * cellPixelSize + padding, drawable.CellPosition.y * cellPixelSize + padding,
					drawable.CellPosition.x * cellPixelSize + cellPixelSize - padding, drawable.CellPosition.y * cellPixelSize + cellPixelSize - padding,
					drawable.CellPosition.x * cellPixelSize, drawable.CellPosition.y * cellPixelSize,
					drawable.CellPosition.x * cellPixelSize + cellPixelSize, drawable.CellPosition.y * cellPixelSize + cellPixelSize);

				Arc(hdc, drawable.CellPosition.x * cellPixelSize + padding, drawable.CellPosition.y * cellPixelSize + padding,
					drawable.CellPosition.x * cellPixelSize + cellPixelSize - padding, drawable.CellPosition.y * cellPixelSize + cellPixelSize - padding,
					drawable.CellPosition.x * cellPixelSize + cellPixelSize, drawable.CellPosition.y * cellPixelSize + cellPixelSize,
					drawable.CellPosition.x * cellPixelSize, drawable.CellPosition.y * cellPixelSize);
			}
			else if (drawable.Type == DrawableType::One)
			{
				SelectFont(hdc, normalTextFont);
				SetTextAlign(hdc, TA_CENTER);
				TextOutA(hdc, xUpLeft + cellPixelSize/2, yUpLeft , "1", 1);
			}
			else if (drawable.Type == DrawableType::Two)
			{
				SelectFont(hdc, normalTextFont);
				SetTextAlign(hdc, TA_CENTER);
				TextOutA(hdc, xUpLeft + cellPixelSize / 2, yUpLeft, "2", 1);
			}
			else if (drawable.Type == DrawableType::Three)
			{
				SelectFont(hdc, normalTextFont);
				SetTextAlign(hdc, TA_CENTER);
				TextOutA(hdc, xUpLeft + cellPixelSize / 2, yUpLeft, "3", 1);
			}
			else if (drawable.Type == DrawableType::Four)
			{
				SelectFont(hdc, normalTextFont);
				SetTextAlign(hdc, TA_CENTER);
				TextOutA(hdc, xUpLeft + cellPixelSize / 2, yUpLeft, "4", 1);
			}
			else if (drawable.Type == DrawableType::Five)
			{
				SelectFont(hdc, normalTextFont);
				SetTextAlign(hdc, TA_CENTER);
				TextOutA(hdc, xUpLeft + cellPixelSize / 2, yUpLeft, "5", 1);
			}
			else if (drawable.Type == DrawableType::Six)
			{
				SelectFont(hdc, normalTextFont);
				SetTextAlign(hdc, TA_CENTER);
				TextOutA(hdc, xUpLeft + cellPixelSize / 2, yUpLeft, "6", 1);
			}
			else if (drawable.Type == DrawableType::Seven)
			{
				SelectFont(hdc, normalTextFont);
				SetTextAlign(hdc, TA_CENTER);
				TextOutA(hdc, xUpLeft + cellPixelSize / 2, yUpLeft, "7", 1);
			}
			else if (drawable.Type == DrawableType::Eight)
			{
				SelectFont(hdc, normalTextFont);
				SetTextAlign(hdc, TA_CENTER);
				TextOutA(hdc, xUpLeft + cellPixelSize / 2, yUpLeft, "8", 1);
			}
		}

		EndPaint(hWnd, &ps);
		break;
	}
	case WM_CLOSE: //WM_CLOSE runs when pressing the X on the window
	{
		Cleanup(hWnd);
		PostQuitMessage(0);
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void Init(HWND hWnd)
{
	game.Init(20);
	hdc = GetDC(hWnd); 
	SetBkMode(hdc, TRANSPARENT); //Sets background for text to Transparent

	mainFont = CreateFont(50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, L""); // You can leave all values as zero and everything will be standard
	normalTextFont = CreateFont(25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, L""); // You can leave all values as zero and everything will be standard

	flagPen = CreatePen(PS_SOLID, 5, RGB(250, 40, 20));

#if DARK_MODE 1
	// Just some code i found in a stackoverflow thread.
	DWMNCRENDERINGPOLICY ncrp = DWMNCRP_ENABLED;
	DwmSetWindowAttribute(hWnd, DWMWA_NCRENDERING_POLICY, &ncrp, sizeof(ncrp));	// Enable non client rendering
	BOOL value = TRUE;
	DwmSetWindowAttribute(hWnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value)); // Enable darkmode on the titlebar

	SetTextColor(hdc, RGB(230, 230, 230));
	gameplanPen = CreatePen(PS_SOLID, 1, RGB(50, 50, 50));
#else
	SetTextColor(hdc, RGB(0, 0, 0));
	gameplanPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	winningPen = CreatePen(PS_SOLID, 8, RGB(0, 0, 0));
#endif

}

void Cleanup(HWND hWnd)
{
	game.Cleanup();

	DeleteObject(gameplanPen);
	DeleteObject(flagPen);

	DeleteObject(mainFont);
	DeleteObject(normalTextFont);

	ReleaseDC(hWnd, hdc);
}


