#include <Windows.h>
#include <algorithm>
#include "GameLogic.h"
#include "Console.h"

void FrameSync(unsigned int _framerate) {

}

void Init(char _arrMap[MAP_HEIGHT][MAP_WIDTH], PPLAYER _pPlayer, PPOS _pStartPos, PPOS _pDoorPos) {
	system("title 23Bombman | mode con cols=80 lines=40");
	SetCursorVis(true, 2);
	LockResize();
	//PlayBGM();

	// 맵 세팅하는 부분

	// Wall = 0, Road = 1, Door = 2, Start = 3
	strcpy_s(_arrMap[0], "00000000000000000000");
	strcpy_s(_arrMap[1], "01111111111111111110");
	strcpy_s(_arrMap[2], "01111111111111111110");
	strcpy_s(_arrMap[3], "01111111111111111110");
	strcpy_s(_arrMap[4], "01111111111111111110");
	strcpy_s(_arrMap[5], "01111111111111111110");
	strcpy_s(_arrMap[6], "01111111111111111110");
	strcpy_s(_arrMap[7], "01111111111111111110");
	strcpy_s(_arrMap[8], "01111111111111111110");
	strcpy_s(_arrMap[9], "03111111111111111120");
	strcpy_s(_arrMap[10], "01111111111111111110");
	strcpy_s(_arrMap[11], "01111111111111111110");
	strcpy_s(_arrMap[12], "01111111111111111110");
	strcpy_s(_arrMap[13], "01111111111111111110");
	strcpy_s(_arrMap[14], "01111111111111111110");
	strcpy_s(_arrMap[15], "01111111111111111110");
	strcpy_s(_arrMap[16], "01111111111111111110");
	strcpy_s(_arrMap[17], "01111111111111111110");
	strcpy_s(_arrMap[18], "01111111111111111110");
	strcpy_s(_arrMap[19], "00000000000000000000");

	for (int i = 0; i < MAP_HEIGHT; ++i)
	{
		for (int j = 0; j < MAP_WIDTH; ++i) {
			if (_arrMap[i][j] == (char)OBJ_TYPE::START)
				*_pStartPos = { j,i };
			else if (_arrMap[i][j] = (char)OBJ_TYPE::DOOR)
				*_pDoorPos = { j,i };
		}
	}

	*_pPlayer = { *_pStartPos, {} };
}

void Update(char _arrMap[MAP_HEIGHT][MAP_WIDTH], PPLAYER _pPlayer) {
	MoveUpdate(_arrMap, _pPlayer);

	Sleep(20);
}

void MoveUpdate(char _arrMap[MAP_HEIGHT][MAP_WIDTH], PPLAYER _pPlayer) {
	_pPlayer->tNewPos = _pPlayer->tPos;

	if (GetAsyncKeyState(VK_UP) & 0x8000)
		--_pPlayer->tNewPos.y;
	if (GetAsyncKeyState(VK_DOWN) & 0x8000)
		++_pPlayer->tNewPos.y;
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		--_pPlayer->tNewPos.x;
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		++_pPlayer->tNewPos.x;

	_pPlayer->tNewPos.x = std::clamp(_pPlayer->tNewPos.x, 0, MAP_WIDTH - 2);
	_pPlayer->tNewPos.y = std::clamp(_pPlayer->tNewPos.y, 0, MAP_HEIGHT - 1);

	if (_arrMap[_pPlayer->tNewPos.y][_pPlayer->tNewPos.x]
		!= (char)OBJ_TYPE::WALL)
	{
		_pPlayer->tPos = _pPlayer->tNewPos;
	}
}
