#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>

using namespace std;

int main() {
	SetConsoleOutputCP(65001);


	ifstream story;
	story.open("GPStory.txt");

	string line;

	if (story.is_open()) {
		while (getline(story, line))
		{
			for (int i = 0; i < line.size(); i++)
			{
				if (line[i] == '1') {
					Sleep(100);
					system("cls");
					Sleep(3000);
				}
				else cout << line[i];
				Sleep(10);
			}

			cout << "\n";
			Sleep(300);
		}

		story.close();
	}
}