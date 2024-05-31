#include "GameLogic.h"
#include <Windows.h>
#include "Console.h"

int main() {
	char arrMap[MAP_HEIGHT][MAP_WIDTH] = {};
	PLAYER tPlayer = {};

	POS tStartPos = {};
	POS tDoorPos = {};

	Init(arrMap, &tPlayer, &tStartPos, &tDoorPos);

}