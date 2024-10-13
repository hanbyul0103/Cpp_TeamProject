#pragma once

void MyMoveWindow(int x, int y);
void ShakeWindow(int count);
void ShowTitle();

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 600;

int Resolutionx = GetSystemMetrics(SM_CXSCREEN);
int Resolutiony = GetSystemMetrics(SM_CYSCREEN);

int WinposX = Resolutionx / 2 - SCREEN_WIDTH / 2;
int WinposY = Resolutiony / 2 - SCREEN_HEIGHT / 2;

enum class Key {
	SHAKE = 49, // 1
	CLS = 50, // 2
	REST = 51, // 3
	NARR = 52, // 4
	DANGER = 53, // 5
	WARNING = 54, // 6

	ANNOUNCE = 55, // 7
	ANNOUNCE_DANGER = 56, // 8
	ANNOUNCE_WARNING = 57, // 9
	TAKE_OFF = 58, // :
};