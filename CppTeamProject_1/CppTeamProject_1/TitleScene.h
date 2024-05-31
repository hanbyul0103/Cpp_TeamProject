#pragma once

enum class MENU {
	START, ITEMINFO, QUIT
};

enum class KEY {
	UP, DOWN, SPACE, FALE
};

bool TitleScene();
void TitleRender();
void InfoRender();
MENU MenuRender();
KEY KeyController();
void EnterAnimation();