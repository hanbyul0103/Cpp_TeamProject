#include <Windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <conio.h>
#include "title.h"
#include "mci.h";

using std::cin;
using std::cout;

const int WIDTH = 1920;
const int HEIGHT = 1080;

void MyMoveWindow(int x, int y) {
	HWND hwnd = GetConsoleWindow();
	MoveWindow(hwnd, x, y, WIDTH / 2, HEIGHT / 2, TRUE);
}

void ShakeWindow(int count) {
	int shakeCount = count;

	for (int i = 0; i < shakeCount; i++)
	{
		MyMoveWindow(WIDTH / 7 + pow(-1, i) * 2, HEIGHT / 7 + pow(-1, i) * 2);
		Sleep(50);
	}

	for (int i = 0; i < shakeCount; i++)
	{
		MyMoveWindow(WIDTH / 7 + pow(-1, i), HEIGHT / 7 + pow(-1, i));
		Sleep(50);
	}
}

void ShowTitle() {
	system("cls");
	UINT originalCP = GetConsoleOutputCP();
	SetConsoleOutputCP(65001);

	MyMoveWindow(WIDTH / 7, HEIGHT / 7);

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
				Sleep(1);
			}

			Sleep(600);
			cout << "\n";
		}

		story.close();
	}
	SetConsoleOutputCP(originalCP);
}