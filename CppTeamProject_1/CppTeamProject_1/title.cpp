#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>

using std::cin;
using std::cout;

void MoveWindow(int x, int y);
void ShakeWindow(int count);

int main() {
	SetConsoleOutputCP(65001);

	MoveWindow(384, 216);

	std::ifstream story;
	story.open("GPStory.txt");

	std::string line;

	if (story.is_open()) {
		while (getline(story, line))
		{
			for (int i = 0; i < line.size(); i++)
			{
				if (line[i] == '1') {
					ShakeWindow(20);
				}
				if (line[i] == '2') {
					system("cls");
					Sleep(100);
				}
				else cout << line[i];
				Sleep(20);
			}

			cout << "\n";
			Sleep(300);
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
		MoveWindow(384 + pow(-1, i), 216 + pow(-1, i));
		Sleep(50);
	}
}