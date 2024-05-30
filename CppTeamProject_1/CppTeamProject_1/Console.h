#pragma once

enum class COLOR {
	BLACK, BLUE, GREEN, SKYBLUE, RED,
	VOILET, YELLOW, LIGHT_GRAY, GRAY, LIGHT_BLUE,
	LIGHT_GREEN, MINT, LIGHT_RED, LIGHT_VIOLET,
	LIGHT_YELLOW, WHITE, END
};

void FullScreen();
void LockResize();
void GotoXY(int _x, int _y);
void SetColor(int _textColor = 15, int _backgroundColor = 0);
COORD CursorPos();
COORD GetConsoleResolution();

#include <iostream>
using std::cout;
using std::wcout;
using std::endl;