#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <conio.h>
#include "mci.h";

#pragma comment(lib, "winmm.lib")

using std::cin;
using std::cout;

void MoveWindow(int x, int y);
void ShakeWindow(int count);

const int WIDTH = 1920;
const int HEIGHT = 1080;

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

int main() {
	SetConsoleOutputCP(65001);

	MoveWindow(WIDTH / 7, HEIGHT / 7);

	std::ifstream story;
	story.open("GPStory.txt");

	std::string line;

	if (story.is_open()) {
		int sleepTime = 0;
		while (getline(story, line))
		{
			for (int i = 0; i < line.size(); i++)
			{
				if (line[i] == (char)Key::SHAKE) {
					PlaySound(TEXT("SoundEffect/explosion.wav"), NULL, SND_FILENAME | SND_ASYNC);
					ShakeWindow(12);
				}
				else if (line[i] == (char)Key::CLS) {
					system("cls");
					Sleep(1000);
				}
				else if (line[i] == (char)Key::REST) {
					sleepTime = 120;
				}
				else if (line[i] == (char)Key::NARR) {
					sleepTime = 30;
				}
				else if (line[i] == (char)Key::DANGER) {
					sleepTime = 15;
				}
				else if (line[i] == (char)Key::WARNING) {
					sleepTime = 8;
				}
				else if (line[i] == (char)Key::ANNOUNCE) {
					PlaySound(TEXT("SoundEffect/announcement.wav"), NULL, SND_FILENAME | SND_ASYNC);
				}
				else if (line[i] == (char)Key::ANNOUNCE_DANGER) {
					PlaySound(TEXT("SoundEffect/danger_announcement.wav"), NULL, SND_FILENAME | SND_ASYNC);
				}
				else if (line[i] == (char)Key::ANNOUNCE_WARNING) {
					PlaySound(TEXT("SoundEffect/warning.wav"), NULL, SND_FILENAME | SND_ASYNC);
				}
				else if (line[i] == (char)Key::TAKE_OFF) {
					PlaySound(TEXT("SoundEffect/airplane.wav"), NULL, SND_FILENAME | SND_ASYNC);
				}
				else cout << line[i];
				Sleep(sleepTime);
			}

			Sleep(600);
			cout << "\n";
		}

		story.close();
	}
}

void MoveWindow(int x, int y) {
	HWND hwnd = GetConsoleWindow();
	MoveWindow(hwnd, x, y, WIDTH / 2, HEIGHT / 2, TRUE);
}

void ShakeWindow(int count) {
	int shakeCount = count;

	for (int i = 0; i < shakeCount; i++)
	{
		MoveWindow(WIDTH / 7 + pow(-1, i) * 2, HEIGHT / 7 + pow(-1, i) * 2);
		Sleep(50);
	}

	for (int i = 0; i < shakeCount; i++)
	{
		MoveWindow(WIDTH / 7 + pow(-1, i), HEIGHT / 7 + pow(-1, i));
		Sleep(50);
	}
}