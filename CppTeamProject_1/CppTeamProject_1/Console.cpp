#include <Windows.h>
#include "Console.h"

void FullScreen() {
	ShowWindow(GetConsoleWindow(), SW_MAXIMIZE);
}

void LockResize() {
	HWND console = GetConsoleWindow();

	if (console != nullptr) {
		LONG style = GetWindowLong(console, GWL_STYLE);
		style &= ~WS_MAXIMIZEBOX & ~WS_SIZEBOX;
		SetWindowLong(console, GWL_STYLE, style);
	}
}

void GotoXY(int _x, int _y) {
	COORD cursor = { _x * 2, _y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cursor);
}

void SetColor(int _textColor, int _backgroundColor) {
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), (_backgroundColor << 4) | _textColor);
}

void SetCursorVis(bool _visible, DWORD _size)
{
	CONSOLE_CURSOR_INFO curinfo;
	curinfo.dwSize = _size;  // Ä¿¼­ ±½±â(1~100)
	curinfo.bVisible = _visible; // True: On, False: Off
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curinfo);
}

COORD CursorPos() {
	CONSOLE_SCREEN_BUFFER_INFO buffer;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &buffer);
	return buffer.dwCursorPosition;
}

COORD GetConsoleResolution() {
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
	short width = info.srWindow.Right - info.srWindow.Left + 1;
	short height = info.srWindow.Bottom - info.srWindow.Top + 1;
	return COORD{ width, height };
}
