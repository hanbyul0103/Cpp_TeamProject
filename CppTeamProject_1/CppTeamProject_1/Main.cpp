#include<Windows.h>
#include<conio.h>
#include<iostream>
#include<algorithm>
#include<fstream>
#include<string>
#include<map>
using namespace std;

const int MAP_WIDTH = 50;
const int MAP_HEIGHT = 13;
const time_t ITEM_DURATION = 5000;

typedef struct _tagpos
{
	int x;
	int y;
}POS, *PPOS;

typedef struct _tagplayer
{
	POS tPos;
	POS tNewPos;
	bool speedUp = false;
	int speedcnt = 0;
	int magnetcnt = 0;
}PLAYER, * PPLAYER;

enum class OBJ_TYPE {
	ROAD = '0', SIT, WALL, START, PLAYER, LDOOR, RDOOR, MAGNET, SPEED, ITEM, END
};

map<OBJ_TYPE, string> obj_map;

BOOL Gotoxy(int _x, int _y)
{
	COORD Cur = { _x, _y };
	return SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE)
		, Cur);
}

void SetCursorVis(bool _vis, DWORD _size) {
	CONSOLE_CURSOR_INFO curInfo;
	curInfo.bVisible = _vis; // True: On, false: Off
	curInfo.dwSize = _size; // Ä¿¼­ ±½±â (1~100)
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}

void Render(char _map[MAP_HEIGHT][MAP_WIDTH],PPLAYER _pPlayer) {
	Gotoxy(0, 0);
	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++)
			cout << ((j==_pPlayer->tPos.x&&i==_pPlayer->tPos.y)? "¡Ù" : obj_map[(OBJ_TYPE)_map[i][j]]);
		cout << '\n';
	}

	cout << "¡â:" << _pPlayer->speedcnt << ' ';
}

bool Movable(OBJ_TYPE obj) {
	return obj != OBJ_TYPE::WALL && obj != OBJ_TYPE::SIT;
}

void MoveUpdate(char _arrmap[MAP_HEIGHT][MAP_WIDTH], PPLAYER _pPlayer)
{
	_pPlayer->tNewPos = _pPlayer->tPos;

	static bool used = 0;
	bool pressed = 0;

	if (GetAsyncKeyState(VK_UP) & 0x8000)
		--_pPlayer->tNewPos.y,pressed = true;
	if (GetAsyncKeyState(VK_DOWN) & 0x8000)
		++_pPlayer->tNewPos.y, pressed = true;
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		--_pPlayer->tNewPos.x, pressed = true;
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		++_pPlayer->tNewPos.x, pressed = true;
	
	if (!pressed)
		used = 0;

	if (Movable((OBJ_TYPE)_arrmap[_pPlayer->tNewPos.y][_pPlayer->tNewPos.x]) && (!used||_pPlayer->speedUp))
	{
		if (_pPlayer->tPos.x != _pPlayer->tNewPos.x || _pPlayer->tPos.y != _pPlayer->tNewPos.y) {
			_pPlayer->tPos = _pPlayer->tNewPos;
			used = 1;
		}
	}
}

void PickUpdate(char _arrmap[MAP_HEIGHT][MAP_WIDTH], PPLAYER _pPlayer) {
	if (_arrmap[_pPlayer->tPos.y][_pPlayer->tPos.x] == (char)OBJ_TYPE::SPEED) {
		_pPlayer->speedcnt++;
		_arrmap[_pPlayer->tPos.y][_pPlayer->tPos.x] = (char)OBJ_TYPE::ROAD;
	}
}

void ItemUpdate(char _arrmap[MAP_HEIGHT][MAP_WIDTH], PPLAYER _pPlayer) {
	static time_t speedStart;

	if ((GetAsyncKeyState(0x53) & 0x8000)&&!_pPlayer->speedUp&&_pPlayer->speedcnt) {
		_pPlayer->speedcnt--;
		_pPlayer->speedUp = true;
		speedStart = clock();
	}

	if(clock() - speedStart>ITEM_DURATION)
		_pPlayer->speedUp = false;
}

int main() {
	char map[MAP_HEIGHT][MAP_WIDTH];

	obj_map[OBJ_TYPE::ROAD] = "  ";
	obj_map[OBJ_TYPE::WALL] = "¡á";
	obj_map[OBJ_TYPE::SIT] = "¡×";
	obj_map[OBJ_TYPE::SPEED] = "¡â";

	ifstream fin;
	fin.open("GPMap.txt");

	string line;
	int row = 0;
	while (getline(fin, line) && row < MAP_HEIGHT) {
		for (int col = 0; col < line.length() && col < MAP_WIDTH; ++col) {
			map[row][col] = line[col];
		}
		++row;
	}

	fin.close();

	PLAYER player;
	player.tPos ={ 1,6 };
	
	SetCursorVis(false, 1);
	while (true) {
		MoveUpdate(map, &player);
		PickUpdate(map, &player);
		ItemUpdate(map, &player);
		Render(map,&player);
	}
}