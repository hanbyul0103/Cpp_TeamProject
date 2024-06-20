#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>
#include <conio.h>

using std::cin;
using std::cout;

void MoveWindow(int x, int y);
void ShakeWindow(int count);

enum class Key {
	SHAKE	= 49, // 1
	CLS		= 50, // 2
	REST	= 51, // 3
	NARR	= 52, // 4
	DANGER	= 53, // 5
	WARNING	= 54, // 6
};

int main() {
	SetConsoleOutputCP(65001);

	MoveWindow(384, 216);

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
					ShakeWindow(10);
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
	MoveWindow(hwnd, x, y, 960, 540, TRUE);
}

void ShakeWindow(int count) {
	int shakeCount = count;

	for (int i = 0; i < shakeCount; i++)
	{
		MoveWindow(384 + pow(-1, i) * 2, 216 + pow(-1, i) * 2);
		Sleep(50);
	}

	for (int i = 0; i < shakeCount; i++)
	{
		MoveWindow(384 + pow(-1, i), 216 + pow(-1, i));
		Sleep(50);
	}
}