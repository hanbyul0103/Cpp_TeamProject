#pragma once

const int MAP_WIDTH  = 21;
const int MAP_HEIGHT = 20;

typedef struct _pos
{
	int x;
	int y;
}POS, *PPOS;

typedef struct _player
{
	POS tPos; // 위치
	POS tNewPos; // 위치
}PLAYER, *PPLAYER;

enum class OBJ_TYPE
{
	WALL = '0', ROAD, DOOR, START, END
};

#include <vector>
using std::vector;
void FrameSync(unsigned int _framerate);
void Init(char _arrMap[MAP_HEIGHT][MAP_WIDTH], PPLAYER _pPlayer, PPOS _pStartPos, PPOS _pDoorPos);
void Update(char _arrMap[MAP_HEIGHT][MAP_WIDTH], PPLAYER _pPlayer);
void MoveUpdate(char _arrMap[MAP_HEIGHT][MAP_WIDTH], PPLAYER _pPlayer);