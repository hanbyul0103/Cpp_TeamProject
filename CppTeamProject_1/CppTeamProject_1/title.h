#pragma once

void MyMoveWindow(int x, int y);
void ShakeWindow(int count);
void ShowTitle();

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