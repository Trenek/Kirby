#include <windows.h>
#include <stdbool.h>

#include "Zasoby.h"

#define ID_TIMER 1

#define KIRBYX 100
#define KIRBYY 100
#define SCREENX 800
#define SCREENY 600
#define KLATKI 2
#define CHANGE_FREQUENCY 30
#define TRANSPARENT_COLOUR RGB(255, 0, 255)

MSG Komunikat;

SHORT SpeedX = 2;
SHORT SpeedY = 2;

HWND g_hwnd;
HBRUSH background;

struct Kirbufor {
	HBITMAP hbm;
	HBITMAP hbmMaska;
	HBITMAP hbmOld;
	HDC hdcMem;
}Kirby[KLATKI], Bufor[2];

BITMAP bmKirby;

RECT rcOkno;
RECT rcKirby;

POINT upper;
POINT lower;

int i = 0;
int klatka = 0;

int buftick = 0;
bool resize = false;

HBITMAP CreateBitmapMask(HBITMAP hbmColour, COLORREF crTransparent) {
	HDC hdcMem;
	HDC hdcMem2;
	HBITMAP hbmMask;
	HBITMAP hbmOld;
	HBITMAP hbmOld2;
	BITMAP bm;

	GetObject(hbmColour, sizeof(BITMAP), &bm);
	hbmMask = CreateBitmap(bm.bmWidth, bm.bmHeight, 1, 1, NULL);

	hdcMem = CreateCompatibleDC(NULL);
	hdcMem2 = CreateCompatibleDC(NULL);

	hbmOld = (HBITMAP)SelectObject(hdcMem, hbmColour);
	hbmOld2 = (HBITMAP)SelectObject(hdcMem2, hbmMask);

	SetBkColor(hdcMem, crTransparent);

	BitBlt(hdcMem2, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);
	BitBlt(hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem2, 0, 0, SRCINVERT);

	SelectObject(hdcMem, hbmOld);
	SelectObject(hdcMem2, hbmOld2);
	DeleteDC(hdcMem);
	DeleteDC(hdcMem2);

	return hbmMask;
}

void GetKirby(HINSTANCE hInstance, struct Kirbufor *Kirby, int id) {
	HDC hdc = GetDC(g_hwnd);

	(*Kirby).hbm = LoadBitmap(hInstance, MAKEINTRESOURCE(id));
	(*Kirby).hbmMaska = CreateBitmapMask((*Kirby).hbm, TRANSPARENT_COLOUR);

	(*Kirby).hdcMem = CreateCompatibleDC(hdc);
	(*Kirby).hbmOld = (HBITMAP)SelectObject((*Kirby).hdcMem, (*Kirby).hbm);

	ReleaseDC(g_hwnd, hdc);
}

void UpdateKirby() {
	ScreenToClient(g_hwnd, &upper);
	ScreenToClient(g_hwnd, &lower);

	if (upper.x <= 0 || lower.x >= rcOkno.right - rcOkno.left) {
		SpeedX = -SpeedX;
	}
	if (upper.y <= 0 || lower.y >= rcOkno.bottom - rcOkno.top) {
		SpeedY = -SpeedY;
	}

	upper.x += SpeedX;
	upper.y += SpeedY;
	lower.x += SpeedX;
	lower.y += SpeedY;

	ClientToScreen(g_hwnd, &upper);
	ClientToScreen(g_hwnd, &lower);
}

void UpdateCords() {
	ScreenToClient(g_hwnd, &upper);
	ScreenToClient(g_hwnd, &lower);

	rcKirby = (RECT){
		.left = upper.x,
		.right = lower.x,
		.top = upper.y,
		.bottom = lower.y
	};

	ClientToScreen(g_hwnd, &upper);
	ClientToScreen(g_hwnd, &lower);
}

void Delete(struct Kirbufor*);
void CreateKirbyBuff(HWND hwnd, struct Kirbufor *Bufor) {
	GetClientRect(hwnd, &rcOkno);
	Delete(Bufor);

	HDC hdc = GetDC(hwnd);
	(*Bufor).hdcMem = CreateCompatibleDC(hdc);
	(*Bufor).hbm = CreateCompatibleBitmap(hdc, rcOkno.right, rcOkno.bottom);
	(*Bufor).hbmOld = (HBITMAP)SelectObject((*Bufor).hdcMem, (*Bufor).hbm);
	ReleaseDC(hwnd, hdc);
}

void BuffKirby(struct Kirbufor Kirby, struct Kirbufor* Buffer) {
	FillRect((*Buffer).hdcMem, &rcOkno, background);

	SelectObject(Kirby.hdcMem, Kirby.hbmMaska);
	BitBlt((*Buffer).hdcMem, rcKirby.left, rcKirby.top, bmKirby.bmWidth, bmKirby.bmHeight, Kirby.hdcMem, 0, 0, SRCAND);
	SelectObject(Kirby.hdcMem, Kirby.hbm);
	BitBlt((*Buffer).hdcMem, rcKirby.left, rcKirby.top, bmKirby.bmWidth, bmKirby.bmHeight, Kirby.hdcMem, 0, 0, SRCINVERT);
}

void RysujKirby(struct Kirbufor Buffer) {
	HDC hdc = GetDC(g_hwnd);
	BitBlt(hdc, rcOkno.left, rcOkno.top, rcOkno.right, rcOkno.bottom, Buffer.hdcMem, 0, 0, SRCCOPY);
	ReleaseDC(g_hwnd, hdc);
}

void Delete(struct Kirbufor* Kirby) {
	SelectObject((*Kirby).hdcMem, (*Kirby).hbmOld);
	DeleteDC((*Kirby).hdcMem);
	DeleteObject((*Kirby).hbm);
	DeleteObject((*Kirby).hbmMaska);
}

LRESULT WINAPI Oknus(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		KillTimer(hwnd, ID_TIMER);
		i = 0;
		while (i < KLATKI) {
			Delete(&Kirby[i]);
			i++;
		}
		Delete(&Bufor[0]);
		Delete(&Bufor[1]);
		PostQuitMessage(0);
		break;
	case WM_TIMER:
		if (resize) {
			resize = false;
		}
		UpdateKirby();
		UpdateCords();
		BuffKirby(Kirby[klatka], &(Bufor[buftick]));
		RysujKirby(Bufor[buftick]);

		if (i == CHANGE_FREQUENCY) {
			i = 0;
			klatka = (klatka + 1) % KLATKI;
		}
		i++;

		break;
	case WM_SIZE:
		CreateKirbyBuff(hwnd, &Bufor[buftick]);
		GetClientRect(g_hwnd, &rcOkno);
		UpdateCords();
		BuffKirby(Kirby[klatka], &(Bufor[buftick]));
		RysujKirby(Bufor[buftick]);
		resize = true;
		break;
	case WM_PAINT:
		CreateKirbyBuff(hwnd, &Bufor[buftick]);
		GetClientRect(g_hwnd, &rcOkno);
		UpdateCords();
		BuffKirby(Kirby[klatka], &(Bufor[buftick]));
		RysujKirby(Bufor[buftick]);
		resize = true;
	case WM_MOVE:
		GetClientRect(g_hwnd, &rcOkno);
		UpdateCords();
		BuffKirby(Kirby[klatka], &(Bufor[buftick]));
		RysujKirby(Bufor[buftick]);
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hprevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
	background = (HBRUSH)CreateSolidBrush(RGB(255, 182, 193));
	HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));

	WNDCLASSEX wc = {
		.cbSize = sizeof(WNDCLASSEX),
		.style = 0,
		.lpfnWndProc = Oknus,
		.cbClsExtra = 0,
		.cbWndExtra = 0,
		.hInstance = hInstance,
		.hIcon = hIcon,
		.hCursor = LoadCursor(NULL, IDC_ARROW),
		.hbrBackground = background,
		.lpszMenuName = NULL,
		.lpszClassName = "Kirby",
		.hIconSm = hIcon
	};

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Wysoka Komisja odmawia rejestracji tego okna!", "Niestety...", MB_ICONEXCLAMATION | MB_OK);
		return 1;
	}

	g_hwnd = CreateWindowEx(WS_EX_WINDOWEDGE, "Kirby", "Kirby", WS_DLGFRAME | WS_OVERLAPPEDWINDOW | WS_SIZEBOX, (GetSystemMetrics(SM_CXSCREEN) - SCREENX) / 2, (GetSystemMetrics(SM_CYSCREEN) - SCREENY) / 2, SCREENX, SCREENY, NULL, NULL, hInstance, NULL);

	if (!SetTimer(g_hwnd, ID_TIMER, USER_TIMER_MINIMUM, NULL)) {
		MessageBox(g_hwnd, "Nie", "OJ", MB_ICONSTOP);
	}

	if (g_hwnd == NULL) {
		MessageBox(NULL, "Okno odmówiło przyjścia na świat!", "Ale kicha...", MB_ICONEXCLAMATION);
		return 1;
	}

	ShowWindow(g_hwnd, nCmdShow);
	UpdateWindow(g_hwnd);

	#pragma region Ładowanie Klatek

	GetClientRect(g_hwnd, &rcOkno);

	GetKirby(hInstance, &Kirby[0], IDB_KIRBY1);
	GetKirby(hInstance, &Kirby[1], IDB_KIRBY2);

	GetObject(Kirby[0].hbm, sizeof(bmKirby), &bmKirby);
	SetRect(&rcKirby, KIRBYX, KIRBYY, KIRBYX + bmKirby.bmWidth, KIRBYY + bmKirby.bmHeight);

	upper = (POINT){
		.x = rcKirby.left,
		.y = rcKirby.top
	};
	lower = (POINT){
		.x = rcKirby.right,
		.y = rcKirby.bottom
	};

	ClientToScreen(g_hwnd, &upper);
	ClientToScreen(g_hwnd, &lower);
	#pragma region bufor

	CreateKirbyBuff(g_hwnd, &Bufor[0]);
	CreateKirbyBuff(g_hwnd, &Bufor[1]);

	#pragma endregion
	#pragma endregion

	while (GetMessage(&Komunikat, NULL, 0, 0)) {
		TranslateMessage(&Komunikat);
		DispatchMessage(&Komunikat);
	}

	return 0;
}
