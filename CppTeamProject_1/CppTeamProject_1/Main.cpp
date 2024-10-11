#include<Windows.h>
#include<conio.h>
#include<iostream>
#include<algorithm>
#include<fstream>
#include<string>
#include<map>
#include<vector>
#include<queue>
#include<random>
#include<fcntl.h>
#include<io.h>
#include"mci.h"
#include"title.h"
using namespace std;

enum class OBJ_TYPE {
	ROAD = '0', SIT, WALL, START, PLAYER, LDOOR, RDOOR, MAGNET, SPEED,
	MIRROR, LONGCOAT, WATER, FLASHLIGHT, PARACHUTE, KNIFE, RAINCOAT, PISTOL,
	SUNGLASS, BANDAID, AXE, GOLDRING, SILVERRING, MACBOOK, IPHONE, AIRPODS,
	MAX, IPAD, CHANDELIER, TRASH, FILTHBAG, BALLPEN, ERASER, NOTEPAD,
	KEYBOARD, BOOK, GLASSES, TISSUE, RADIOS, END
};

enum class COLOR
{
	BLACK, BLUE, GREEN, SKYBLUE, RED,
	VOILET, YELLOW, LIGHT_GRAY, GRAY, LIGHT_BLUE,
	LIGHT_GREEN, MINT, LIGHT_RED, LIGHT_VIOLET,
	LIGHT_YELLOW, WHITE, END
};

typedef struct _tagpos
{
	int x;
	int y;

	bool operator==(const _tagpos& ref) const {
		return x == ref.x && y == ref.y;
	}
	bool operator!=(const _tagpos& ref) const {
		return !(*this == ref);
	}
	int Distance(const _tagpos& ref) const {
		return abs(x - ref.x) + abs(y - ref.y);
	}
}POS, * PPOS;

typedef struct _tagplayer
{
	POS tPos;
	POS tNewPos;
	bool speedUp = false;
	int speedcnt = 0;
	int magnetcnt = 0;

	float surv_percentage = 0;
	int value = 0;
}PLAYER, * PPLAYER;

struct NODE {
	int cost;
	POS pos;

	bool operator<(const NODE& x) const {
		return cost > x.cost;
	}
};

struct MOVE {
	int time;
	int quantity;
	POS from;
	POS to;

	bool operator<(const MOVE& x) const {
		return time < x.time;
	}
};

struct SYNERGY {
	string name;
	int surv_plus;
	vector<OBJ_TYPE> needing_item;
	vector<bool> item_collected;
	int collected_cnt = 0;
	bool completed = false;
	COLOR color;

	SYNERGY(string name, int surv_plus, vector<OBJ_TYPE> needing_item, COLOR color) :name(name)
		, surv_plus(surv_plus), needing_item(needing_item), color(color) {
		item_collected = vector<bool>(needing_item.size());
	}

	bool operator<(const SYNERGY& x) const {
		return (completed ? 0 : (float)collected_cnt / needing_item.size()) >
			(x.completed ? 0 : (float)x.collected_cnt / x.needing_item.size());
	}
};

const int GAME_PLAYTIME = 45;
const int MAP_WIDTH = 114;
const int MAP_HEIGHT = 13;
const int MAG_RANGE = 3;
const int SIDE_SIZE = 19;
const int MAG_SPEED = 5;
const int ITEM_SPECIES = 86;
const int SYNERGY_SPECIES = 8;
const int ITEM_CNT = 30;
const int USABLE_CNT = 2;
const int ITEM_FIRST = 57;
const POS TIMER_POS = { 40,1 };
const POS SYNERGY_POS = { 1,15 };
const time_t ITEM_DURATION = 5000;
const int INF = 1e9;
const string PLAYER_STR = "★";

const string ENDMSG_FIRE = "당신은 불 타 죽었습니다.";
const string ENDMSG_FAIL = "당신은 목이 말라 죽었습니다.";
const string ENDMSG_SUCCESS = "당신은 생존에 성공했습니다!!";

const int dx[]{ -1,1,0,0,1,1,-1,-1 };
const int dy[]{ 0,0,-1,1,1,-1,1,-1 };

const string time_art[10][5]{
	{"■■■■"
	,"■    ■"
	,"■□□■"
	,"■    ■"
	,"■■■■"},

	{"□□□■"
	,"□    ■"
	,"□□□■"
	,"□    ■"
	,"□□□■"},

	{"■■■■"
	,"□    ■"
	,"■■■■"
	,"■    □"
	,"■■■■"},

	{"■■■■"
	,"□    ■"
	,"■■■■"
	,"□    ■"
	,"■■■■"},

	{"■□□■"
	,"■    ■"
	,"■■■■"
	,"□    ■"
	,"□□□■"},

	{"■■■■"
	,"■    □"
	,"■■■■"
	,"□    ■"
	,"■■■■"},

	{"■■■■"
	,"■    □"
	,"■■■■"
	,"■    ■"
	,"■■■■"},

	{"■■■■"
	,"□    ■"
	,"□□□■"
	,"□    ■"
	,"□□□■"},

	{"■■■■"
	,"■    ■"
	,"■■■■"
	,"■    ■"
	,"■■■■"},

	{"■■■■"
	,"■    ■"
	,"■■■■"
	,"□    ■"
	,"□□□■"}
};

map<OBJ_TYPE, string> obj_icon;
map<OBJ_TYPE, COLOR> obj_bgcolor;
map<OBJ_TYPE, COLOR> obj_txtcolor;
map<OBJ_TYPE, float> obj_surv;
map<OBJ_TYPE, int> obj_value;
map<OBJ_TYPE, float> obj_popout;
vector<SYNERGY> synergy_vec;

bool collected[ITEM_SPECIES];
int start_time, left_time;

int best_score;

BOOL Gotoxy(int _x, int _y)
{
	COORD Cur = { _x, _y };
	return SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE)
		, Cur);
}

void SetCursorVis(bool _vis, DWORD _size) {
	CONSOLE_CURSOR_INFO curInfo;
	curInfo.bVisible = _vis; // True: On, false: Off
	curInfo.dwSize = _size; // 커서 굵기 (1~100)
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}

void SetColor(int _textcolor, int _bgcolor)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE)
		, (_bgcolor << 4) | _textcolor);
}

void SetFontSize(UINT _weight, UINT _fontx, UINT _fonty)
{
	CONSOLE_FONT_INFOEX font;
	font.cbSize = sizeof(font);

	HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);

	GetCurrentConsoleFontEx(hout, false, &font);
	font.FontWeight = _weight;
	font.dwFontSize.X = _fontx;
	font.dwFontSize.Y = _fonty;
	SetCurrentConsoleFontEx(hout, false, &font);
}

int current_side = 0;
void Render(vector<char> _map[MAP_HEIGHT][MAP_WIDTH], PPLAYER _pPlayer) {
	Gotoxy(0, 0);
	for (int i = 0; i < MAP_HEIGHT; i++) {		//Map Render
		for (int j = current_side * SIDE_SIZE; j < (current_side + 1) * SIDE_SIZE; j++) {
			OBJ_TYPE obj = (OBJ_TYPE)_map[i][j][_map[i][j].size() - 1];
			if ((_pPlayer->tPos == POS{ j,i })) {
				SetColor((int)COLOR::BLACK, (int)obj_bgcolor[OBJ_TYPE::ROAD]);
				cout << PLAYER_STR;
			}
			else {
				SetColor(int(obj_txtcolor.find(obj) != obj_txtcolor.end() ? obj_txtcolor[obj] : COLOR::BLACK),
					int(obj_bgcolor.find(obj) != obj_bgcolor.end() ? obj_bgcolor[obj] : obj_bgcolor[OBJ_TYPE::ROAD]));
				cout << ((_pPlayer->tPos == POS{ j,i }) ? PLAYER_STR : obj_icon[obj]);
			}
		}
		cout << '\n';
	}

	SetColor((int)COLOR::WHITE, (int)COLOR::BLACK);
	for (int i = 0; i < 5; i++) {		//Time Render
		Gotoxy(TIMER_POS.x, TIMER_POS.y + i);
		cout << time_art[left_time / 10][i] << "  " << time_art[left_time % 10][i];
	}

	Gotoxy(TIMER_POS.x, TIMER_POS.y + 5 + 2);		//clear
	cout << "생존 확률:         ";
	Gotoxy(TIMER_POS.x, TIMER_POS.y + 5 + 2);		//Survival Percentage Render
	cout << "생존 확률: " << _pPlayer->surv_percentage << '%';
	Gotoxy(TIMER_POS.x, TIMER_POS.y + 5 + 3);		//Item Value Render
	cout << "아이템 가치: " << _pPlayer->value;

	Gotoxy(SYNERGY_POS.x, SYNERGY_POS.y - 2);
	cout << obj_icon[OBJ_TYPE::SPEED] << ':' << _pPlayer->speedcnt << ' ';
	cout << obj_icon[OBJ_TYPE::MAGNET] << ':' << _pPlayer->magnetcnt << ' ';

	int i = 0;
	for (auto synergy : synergy_vec) {
		if (synergy.collected_cnt == 0)
			continue;

		if (i % 3 == 0) {
			Gotoxy(SYNERGY_POS.x, SYNERGY_POS.y + i / 3);
			cout << "                                    \n";		//clear
			Gotoxy(SYNERGY_POS.x, SYNERGY_POS.y + i / 3);
		}

		if (synergy.completed)
			SetColor((int)synergy.color, (int)COLOR::BLACK);
		else
			SetColor((int)COLOR::WHITE, (int)COLOR::BLACK);
		cout << synergy.name << ":";

		int j = 0;
		for (auto obj : synergy.needing_item) {
			if (synergy.item_collected[j])
				SetColor((int)synergy.color, (int)COLOR::BLACK);
			else
				SetColor((int)COLOR::WHITE, (int)COLOR::BLACK);
			cout << obj_icon[obj];
			j++;
		}
		cout << "  ";

		i++;
	}
}

bool Moveable(OBJ_TYPE obj) {
	return obj != OBJ_TYPE::WALL && obj != OBJ_TYPE::SIT;
}

bool Pickable(OBJ_TYPE obj) {
	return Moveable(obj) && obj != OBJ_TYPE::ROAD;
}

bool isItem(OBJ_TYPE obj) {
	return Pickable(obj) && obj != OBJ_TYPE::MAGNET && obj != OBJ_TYPE::SPEED;
}

float EaseOutQuad(float x) {
	return 1 - (1 - x) * (1 - x);
}

bool isDGMove(POS x, POS y) {
	return x.x != y.x && x.y != y.y;
}

vector<POS> SearchPath(vector<char> _arrmap[MAP_HEIGHT][MAP_WIDTH], PPLAYER _pPlayer, NODE stNode) {
	static priority_queue<NODE> pq;
	vector<vector<int>> dis(MAP_HEIGHT, vector<int>(MAP_WIDTH, INF));
	vector<vector<POS>> prev(MAP_HEIGHT, vector<POS>(MAP_WIDTH, { -1,-1 }));
	dis[stNode.pos.y][stNode.pos.x] = 0;
	pq.push(stNode);

	while (pq.size()) {
		auto cur = pq.top(); pq.pop();
		if (cur.cost != dis[cur.pos.y][cur.pos.x])
			continue;
		for (int dir = 0; dir < 8; dir++) {
			int nx = cur.pos.x + dx[dir];
			int ny = cur.pos.y + dy[dir];
			int move_time = isDGMove(cur.pos, { nx,ny }) ? 1400 : 1000;

			if (nx < 0 || nx >= MAP_WIDTH || ny < 0 || ny >= MAP_HEIGHT) continue;
			if (dis[ny][nx] <= dis[cur.pos.y][cur.pos.x] + move_time) continue;
			if (_arrmap[ny][nx][0] == (char)OBJ_TYPE::SIT || _arrmap[ny][nx][0] == (char)OBJ_TYPE::WALL) continue;
			dis[ny][nx] = dis[cur.pos.y][cur.pos.x] + move_time;
			pq.push({ dis[ny][nx], {nx, ny} });
			prev[ny][nx] = cur.pos;
		}
	}

	vector<POS> path;
	POS cur = _pPlayer->tPos;
	while (cur != stNode.pos)
		path.push_back(cur), cur = prev[cur.y][cur.x];
	path.push_back(cur);
	reverse(path.begin(), path.end());
	return path;
}

bool ItemUsing(PPLAYER _pPlayer) {
	return _pPlayer->speedUp;
}

bool TimeUpdate() {
	if ((left_time = GAME_PLAYTIME - (clock() - start_time) / 1000) >= 0)
		return true;
	else
		return false;
}

void Pick(vector<char>& vec, PPLAYER _pPlayer) {
	for (auto it = vec.begin(); it != vec.end();) {
		if (!Pickable((OBJ_TYPE)*it)) {
			it++;
			continue;
		}

		OBJ_TYPE obj = (OBJ_TYPE)*it;
		switch (obj) {
		case OBJ_TYPE::SPEED:
			_pPlayer->speedcnt++;
			break;
		case OBJ_TYPE::MAGNET:
			_pPlayer->magnetcnt++;
			break;
		default:
			_pPlayer->value += obj_value[obj];
			if (collected[(int)obj])
				break;
			_pPlayer->surv_percentage += obj_surv[obj];
			collected[(int)obj] = true;

			int i = 0;
			for (auto& synergy : synergy_vec) {
				if (!synergy.completed) {
					int cnt = 0;
					int j = 0;
					for (auto obj : synergy.needing_item) {
						if (collected[(int)obj])
							synergy.item_collected[j] = true, cnt++;
						j++;
					}
					synergy.collected_cnt = cnt;
					if (cnt == synergy.needing_item.size()) {
						synergy.completed = true;
						_pPlayer->surv_percentage += synergy.surv_plus;
					}
				}
				i++;
			}
			sort(synergy_vec.begin(), synergy_vec.end());

			break;
		}

		it = vec.erase(it);
	}
}

void UseMagnatic(vector<char> _arrmap[MAP_HEIGHT][MAP_WIDTH], PPLAYER _pPlayer) {
	int px = _pPlayer->tPos.x, py = _pPlayer->tPos.y;
	int sum = 0;

	vector<MOVE> move_vec;
	for (int i = py - MAG_RANGE; i <= py + MAG_RANGE; i++) {
		for (int j = px - MAG_RANGE; j <= px + MAG_RANGE; j++) {
			int cnt = 0;
			if (_pPlayer->tPos.Distance({ j,i }) >= 5) continue;
			if (i < 0 || i >= MAP_HEIGHT || j < 0 || j >= MAP_WIDTH) continue;
			for (auto x : _arrmap[i][j]) {
				if (Pickable((OBJ_TYPE)x)) {
					cnt++;
					vector<POS> path = SearchPath(_arrmap, _pPlayer, { 0, {j,i} });
					int sum = 0;
					for (int k = 1; k < path.size(); k++) {
						int move_time = sum += (isDGMove(path[k - 1], path[k]) ? 1400 : 1000)
							* EaseOutQuad(_pPlayer->tPos.Distance({ path[k - 1] }) / (float)5) / MAG_SPEED;
						move_vec.push_back({ move_time,cnt,path[k - 1],path[k] });
					}
				}
			}
		}
	}

	sort(move_vec.begin(), move_vec.end());
	time_t start_time = clock();
	//cout << move_vec.size();
	for (auto& move : move_vec) {
		while (clock() - start_time < move.time)
			Sleep(1);

		for (int i = 0; i < move.quantity; i++) {
			_arrmap[move.to.y][move.to.x].push_back(_arrmap[move.from.y][move.from.x].back());
			_arrmap[move.from.y][move.from.x].pop_back();
			Pick(_arrmap[_pPlayer->tPos.y][_pPlayer->tPos.x], _pPlayer);
			TimeUpdate();
			Render(_arrmap, _pPlayer);
		}
	}

	//Pick(_arrmap[_pPlayer->tPos.y][_pPlayer->tPos.x], _pPlayer);
}

bool MoveUpdate(vector<char> _arrmap[MAP_HEIGHT][MAP_WIDTH], PPLAYER _pPlayer)
{
	_pPlayer->tNewPos = _pPlayer->tPos;

	static bool used = 0;
	bool pressed = 0;

	if (GetAsyncKeyState(VK_UP) & 0x8000)
		--_pPlayer->tNewPos.y, pressed = true;
	if (GetAsyncKeyState(VK_DOWN) & 0x8000)
		++_pPlayer->tNewPos.y, pressed = true;
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
		--_pPlayer->tNewPos.x, pressed = true;
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
		++_pPlayer->tNewPos.x, pressed = true;

	if (!pressed)
		used = 0;

	if ((_pPlayer->tNewPos.x == -1 || Moveable((OBJ_TYPE)_arrmap[_pPlayer->tNewPos.y][_pPlayer->tNewPos.x][0]))
		&& (!used || _pPlayer->speedUp))
	{
		if (_pPlayer->tPos.x != _pPlayer->tNewPos.x || _pPlayer->tPos.y != _pPlayer->tNewPos.y) {
			_pPlayer->tPos = _pPlayer->tNewPos;
			current_side = _pPlayer->tPos.x / SIDE_SIZE;
			used = 1;
		}
		if (_pPlayer->speedUp)
			Sleep(15);
	}
	if (_pPlayer->tPos.x == -1)
		return false;
	return true;
}

void PickUpdate(vector<char> _arrmap[MAP_HEIGHT][MAP_WIDTH], PPLAYER _pPlayer) {
	auto& vec = _arrmap[_pPlayer->tPos.y][_pPlayer->tPos.x];
	if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
		Pick(vec, _pPlayer);
	}
}

void ItemUpdate(vector<char> _arrmap[MAP_HEIGHT][MAP_WIDTH], PPLAYER _pPlayer) {
	static time_t speedStart;

	if ((GetAsyncKeyState(0x53) & 0x8000) && !ItemUsing(_pPlayer) && _pPlayer->speedcnt) {
		_pPlayer->speedcnt--;
		_pPlayer->speedUp = true;
		speedStart = clock();
	}

	if (clock() - speedStart > ITEM_DURATION)
		_pPlayer->speedUp = false;

	if ((GetAsyncKeyState(0x44) & 0x8000) && !ItemUsing(_pPlayer) && _pPlayer->magnetcnt) {
		_pPlayer->magnetcnt--;
		UseMagnatic(_arrmap, _pPlayer);
	}
}

void ItemInit() {
	fill(collected, collected + ITEM_SPECIES, 0);
	synergy_vec.clear();

	obj_icon[OBJ_TYPE::ROAD] = "  ";
	obj_icon[OBJ_TYPE::WALL] = "  ";
	obj_icon[OBJ_TYPE::SIT] = "■";
	obj_icon[OBJ_TYPE::SPEED] = "♨";
	obj_icon[OBJ_TYPE::MAGNET] = "∪";

	// 문자
	obj_icon[OBJ_TYPE::MIRROR] = "⊙";
	obj_icon[OBJ_TYPE::LONGCOAT] = "♀";
	obj_icon[OBJ_TYPE::WATER] = "水";
	obj_icon[OBJ_TYPE::FLASHLIGHT] = "†";
	obj_icon[OBJ_TYPE::PARACHUTE] = "⌒";
	obj_icon[OBJ_TYPE::KNIFE] = "4 ";
	obj_icon[OBJ_TYPE::RAINCOAT] = "ㅿ";
	obj_icon[OBJ_TYPE::PISTOL] = "┒ ";
	obj_icon[OBJ_TYPE::SUNGLASS] = "∞";
	obj_icon[OBJ_TYPE::BANDAID] = "ロ";
	obj_icon[OBJ_TYPE::AXE] = "Ｐ";
	obj_icon[OBJ_TYPE::GOLDRING] = "ⓖ";
	obj_icon[OBJ_TYPE::SILVERRING] = "ⓢ";
	obj_icon[OBJ_TYPE::MACBOOK] = "ㅮ";
	obj_icon[OBJ_TYPE::IPHONE] = "☏";
	obj_icon[OBJ_TYPE::AIRPODS] = "♬";
	obj_icon[OBJ_TYPE::MAX] = "Ω";
	obj_icon[OBJ_TYPE::IPAD] = "∏";
	obj_icon[OBJ_TYPE::CHANDELIER] = "Ψ";
	obj_icon[OBJ_TYPE::TRASH] = "ひ";
	obj_icon[OBJ_TYPE::FILTHBAG] = "δ";
	obj_icon[OBJ_TYPE::BALLPEN] = "¡";
	obj_icon[OBJ_TYPE::ERASER] = "Ｄ";
	obj_icon[OBJ_TYPE::NOTEPAD] = "≡";
	obj_icon[OBJ_TYPE::KEYBOARD] = "▦";
	obj_icon[OBJ_TYPE::BOOK] = "∈";
	obj_icon[OBJ_TYPE::GLASSES] = "ㆅ";
	obj_icon[OBJ_TYPE::TISSUE] = "▤";
	obj_icon[OBJ_TYPE::RADIOS] = "】";

	// 생존 확률
	obj_surv[OBJ_TYPE::MIRROR] = 12;
	obj_surv[OBJ_TYPE::LONGCOAT] = 7;
	obj_surv[OBJ_TYPE::WATER] = 16;
	obj_surv[OBJ_TYPE::FLASHLIGHT] = 8;
	obj_surv[OBJ_TYPE::PARACHUTE] = 7;
	obj_surv[OBJ_TYPE::KNIFE] = 14;
	obj_surv[OBJ_TYPE::RAINCOAT] = 2;
	obj_surv[OBJ_TYPE::PISTOL] = 14;
	obj_surv[OBJ_TYPE::SUNGLASS] = 10;
	obj_surv[OBJ_TYPE::BANDAID] = 8;
	obj_surv[OBJ_TYPE::AXE] = 3;
	obj_surv[OBJ_TYPE::GOLDRING] = 1;
	obj_surv[OBJ_TYPE::SILVERRING] = 1;
	obj_surv[OBJ_TYPE::MACBOOK] = -12;
	obj_surv[OBJ_TYPE::IPHONE] = 7;
	obj_surv[OBJ_TYPE::AIRPODS] = -11;
	obj_surv[OBJ_TYPE::MAX] = -11;
	obj_surv[OBJ_TYPE::IPAD] = -9;
	obj_surv[OBJ_TYPE::CHANDELIER] = -13;
	obj_surv[OBJ_TYPE::TRASH] = -8;
	obj_surv[OBJ_TYPE::FILTHBAG] = -10;
	obj_surv[OBJ_TYPE::BALLPEN] = 2;
	obj_surv[OBJ_TYPE::ERASER] = 1;
	obj_surv[OBJ_TYPE::NOTEPAD] = 4;
	obj_surv[OBJ_TYPE::KEYBOARD] = -9;
	obj_surv[OBJ_TYPE::BOOK] = 4;
	obj_surv[OBJ_TYPE::GLASSES] = 6;
	obj_surv[OBJ_TYPE::TISSUE] = 3;
	obj_surv[OBJ_TYPE::RADIOS] = 90;

	// 가치
	obj_value[OBJ_TYPE::MIRROR] = 5000;
	obj_value[OBJ_TYPE::LONGCOAT] = 100000;
	obj_value[OBJ_TYPE::WATER] = 800;
	obj_value[OBJ_TYPE::FLASHLIGHT] = 20000;
	obj_value[OBJ_TYPE::PARACHUTE] = 500000;
	obj_value[OBJ_TYPE::KNIFE] = 30000;
	obj_value[OBJ_TYPE::RAINCOAT] = 10000;
	obj_value[OBJ_TYPE::PISTOL] = 1000000;
	obj_value[OBJ_TYPE::SUNGLASS] = 20000;
	obj_value[OBJ_TYPE::BANDAID] = 5000;
	obj_value[OBJ_TYPE::AXE] = 40000;
	obj_value[OBJ_TYPE::GOLDRING] = 1500000;
	obj_value[OBJ_TYPE::SILVERRING] = 200000;
	obj_value[OBJ_TYPE::MACBOOK] = 2000000;
	obj_value[OBJ_TYPE::IPHONE] = 1000000;
	obj_value[OBJ_TYPE::AIRPODS] = 200000;
	obj_value[OBJ_TYPE::MAX] = 700000;
	obj_value[OBJ_TYPE::IPAD] = 800000;
	obj_value[OBJ_TYPE::CHANDELIER] = 1000000;
	obj_value[OBJ_TYPE::TRASH] = -5000;
	obj_value[OBJ_TYPE::FILTHBAG] = -10000;
	obj_value[OBJ_TYPE::BALLPEN] = 1000;
	obj_value[OBJ_TYPE::ERASER] = 500;
	obj_value[OBJ_TYPE::NOTEPAD] = 2000;
	obj_value[OBJ_TYPE::KEYBOARD] = 50000;
	obj_value[OBJ_TYPE::BOOK] = 15000;
	obj_value[OBJ_TYPE::GLASSES] = 100000;
	obj_value[OBJ_TYPE::TISSUE] = 1000;
	obj_value[OBJ_TYPE::RADIOS] = 550000;

	// 등장 확률
	obj_popout[OBJ_TYPE::MIRROR] = 49;
	obj_popout[OBJ_TYPE::LONGCOAT] = 62;
	obj_popout[OBJ_TYPE::WATER] = 82;
	obj_popout[OBJ_TYPE::FLASHLIGHT] = 52;
	obj_popout[OBJ_TYPE::PARACHUTE] = 45;
	obj_popout[OBJ_TYPE::KNIFE] = 64;
	obj_popout[OBJ_TYPE::RAINCOAT] = 45;
	obj_popout[OBJ_TYPE::PISTOL] = 44;
	obj_popout[OBJ_TYPE::SUNGLASS] = 60;
	obj_popout[OBJ_TYPE::BANDAID] = 42;
	obj_popout[OBJ_TYPE::AXE] = 37;
	obj_popout[OBJ_TYPE::GOLDRING] = 21;
	obj_popout[OBJ_TYPE::SILVERRING] = 21;
	obj_popout[OBJ_TYPE::MACBOOK] = 25;
	obj_popout[OBJ_TYPE::IPHONE] = 35;
	obj_popout[OBJ_TYPE::AIRPODS] = 46;
	obj_popout[OBJ_TYPE::MAX] = 39;
	obj_popout[OBJ_TYPE::IPAD] = 31;
	obj_popout[OBJ_TYPE::CHANDELIER] = 77;
	obj_popout[OBJ_TYPE::TRASH] = 82;
	obj_popout[OBJ_TYPE::FILTHBAG] = 90;
	obj_popout[OBJ_TYPE::BALLPEN] = 47;
	obj_popout[OBJ_TYPE::ERASER] = 26;
	obj_popout[OBJ_TYPE::NOTEPAD] = 24;
	obj_popout[OBJ_TYPE::KEYBOARD] = 41;
	obj_popout[OBJ_TYPE::BOOK] = 64;
	obj_popout[OBJ_TYPE::GLASSES] = 76;
	obj_popout[OBJ_TYPE::TISSUE] = 14;
	obj_popout[OBJ_TYPE::RADIOS] = 1;

	synergy_vec.push_back(SYNERGY("오물", 24, { OBJ_TYPE::TRASH,OBJ_TYPE::FILTHBAG,OBJ_TYPE::TISSUE }, COLOR::VOILET));
	synergy_vec.push_back(SYNERGY("모험자", 12, { OBJ_TYPE::MIRROR, OBJ_TYPE::WATER, OBJ_TYPE::FLASHLIGHT, OBJ_TYPE::KNIFE }, COLOR::RED));
	synergy_vec.push_back(SYNERGY("맥가이버", 9, { OBJ_TYPE::KNIFE, OBJ_TYPE::AXE }, COLOR::BLUE));
	synergy_vec.push_back(SYNERGY("멋쟁이", 15, { OBJ_TYPE::SUNGLASS, OBJ_TYPE::LONGCOAT, OBJ_TYPE::MIRROR }, COLOR::YELLOW));
	synergy_vec.push_back(SYNERGY("산신령", 24, { OBJ_TYPE::AXE, OBJ_TYPE::GOLDRING, OBJ_TYPE::SILVERRING }, COLOR::LIGHT_GREEN));
	synergy_vec.push_back(SYNERGY("앱등이", 50, { OBJ_TYPE::MACBOOK, OBJ_TYPE::IPHONE, OBJ_TYPE::AIRPODS, OBJ_TYPE::IPAD, OBJ_TYPE::MAX }, COLOR::MINT));
	synergy_vec.push_back(SYNERGY("홍상화", 10, { OBJ_TYPE::GLASSES, OBJ_TYPE::KEYBOARD }, COLOR::GREEN));
	synergy_vec.push_back(SYNERGY("이한별", 10, { OBJ_TYPE::GLASSES, OBJ_TYPE::IPHONE, OBJ_TYPE::AIRPODS }, COLOR::SKYBLUE));

	obj_bgcolor[OBJ_TYPE::WALL] = COLOR::GRAY;
	obj_bgcolor[OBJ_TYPE::ROAD] = COLOR::LIGHT_GRAY;
	obj_txtcolor[OBJ_TYPE::SIT] = COLOR::LIGHT_BLUE;
}

random_device rd;
mt19937 gen(rd());
void RandGen(vector<char> _arrmap[MAP_HEIGHT][MAP_WIDTH], vector<POS>& leftSpot, OBJ_TYPE obj) {
	uniform_int_distribution<int> dis2(0, leftSpot.size() - 1);
	int gen_idx = dis2(gen);
	int x = leftSpot[gen_idx].x, y = leftSpot[gen_idx].y;

	_arrmap[y][x].push_back((char)obj);
	leftSpot.erase(leftSpot.begin() + gen_idx);
}

void SpreadItem(vector<char> _arrmap[MAP_HEIGHT][MAP_WIDTH], vector<POS>& leftSpot) {
	int sum[ITEM_SPECIES] = { 0 };
	int i = ITEM_FIRST;
	for (auto x : obj_popout) {
		sum[i] += x.second;
		if (i != ITEM_FIRST)
			sum[i] += sum[i - 1];
		i++;
	}

	uniform_int_distribution<int> dis1(0, sum[ITEM_SPECIES - 1]);

	for (int i = 0; i < ITEM_CNT; i++) {
		int num = dis1(gen);
		int l = ITEM_FIRST, r = ITEM_SPECIES - 1;
		while (l < r) {
			int mid = (l + r) / 2;
			if (sum[mid] < num)
				l = mid + 1;
			else
				r = mid;
		}

		RandGen(_arrmap, leftSpot, (OBJ_TYPE)r);
	}

	for (int i = 0; i < USABLE_CNT; i++) {
		RandGen(_arrmap, leftSpot, OBJ_TYPE::SPEED);
	}
	for (int i = 0; i < USABLE_CNT; i++) {
		RandGen(_arrmap, leftSpot, OBJ_TYPE::MAGNET);
	}
}

void FireEnding() {
	system("cls");
	SetCursorVis(false, 1);

	int prevmode = _setmode(_fileno(stdout), _O_U16TEXT);

	SetColor((int)COLOR::LIGHT_RED, (int)COLOR::BLACK);
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢇⠇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡘⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠄⠀⠀⠀⠀⠀⠀⠀⠀⡄⠀⠀⠀⠀⡘⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠠⡹⡄⠀⠀⠀⠀⠀⠀⠀⡜⡆⠀⡢⠂⠀⠀⡰⡘⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢣⠣⠀⢀⠀⠀⢠⠀⢐⢕⢕⡀⠀⡜⡄⡜⡜⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡇⠈⠀⠀⠇⠁⠀⢸⢱⢪⡪⡣⡫⡣⡣⡣⣣⢣⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡰⡅⠀⠀⠀⡰⣕⠀⠘⡎⡇⡇⡏⣎⢇⢏⢮⢪⡪⡣⡣⡢⡲⡀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⢜⠜⠀⢀⢆⠀⡇⡗⡄⠀⠈⢎⢞⢜⢜⢜⢜⢜⢜⢜⢜⢎⢇⢏⢎⢗⡀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠀⢀⢔⡕⡝⣆⠀⠸⡸⡱⡱⡕⡝⡆⡖⡕⡇⢇⢇⢣⢣⢣⢫⢪⢪⢣⢳⢱⢣⡃⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⡏⠀⡎⡮⡪⡎⡮⡢⡄⡏⡮⡺⡸⡪⣪⢺⢸⠸⡨⡪⡢⡣⡣⡣⡣⡳⡱⡕⡕⠁⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⢀⠁⠈⢮⢪⢪⢪⡪⡪⡎⡮⡪⡪⡪⡺⡸⡸⡨⡪⡪⠢⡪⡪⡪⡪⡪⡪⡎⡎⡎⠀⡤⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⡎⡦⣀⡸⡸⡸⡘⡜⡜⡜⡪⡪⡪⡪⡪⡪⡪⡊⡪⡨⡨⡨⡢⡪⡪⡪⡪⡪⡪⡺⡰⡀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⡇⡏⡮⡪⡺⡸⡸⡘⡜⡜⡜⡌⡪⠪⡂⡊⡪⠢⠢⠢⡂⡪⡪⡪⡪⡪⡪⡪⡺⡸⠅⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠱⡱⡕⡕⡕⡕⡕⡕⡌⡪⠪⠪⡂⡪⡨⡨⡂⡊⡊⡊⡪⠢⡂⡪⡨⡨⡪⡪⡪⡺⡸⠅⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠈⠊⠎⠎⠎⠎⠎⠎⢎⢊⠪⠢⠢⢂⠪⠨⠨⠨⠢⠪⠨⠢⠢⠢⠪⠪⠪⠺⠸⠊⠀⠀⠂⠀⠀⠀\n";

	int curmode = _setmode(_fileno(stdout), prevmode);

	Sleep(1500);
	for (char x : ENDMSG_FIRE)
		cout << x, Sleep(100);
	cout << "\n\n";

	SetColor((int)COLOR::WHITE, (int)COLOR::BLACK);
	system("pause");

}

void Survive(int value) {
	system("cls");
	SetCursorVis(false, 1);

	int prevmode = _setmode(_fileno(stdout), _O_U16TEXT);

	SetColor((int)COLOR::LIGHT_GREEN, (int)COLOR::BLACK);
	wcout << L"                                     .~.                    \n";
	wcout << L"                                     :::-.                  \n";
	wcout << L"                                    ,:.-~:~,                \n";
	wcout << L"                                   ,~-,~~-;:~.              \n";
	wcout << L"                                   -~,,,.-;;;;-.            \n";
	wcout << L"                                  -;;;~~-~:-~:;;-,          \n";
	wcout << L"                                 -;,~;::-,.  .:;;::,        \n";
	wcout << L"                               .~:.  .        :;:..;~.      \n";
	wcout << L"                             .-;~,   -   ,. . :~~::::;.     \n";
	wcout << L"                           -::~,   -., ...,-. : --~-:-      \n";
	wcout << L"                       .,;;;:    , , -~-,~,   -;;;;-:.      \n";
	wcout << L"                     --;;;;:;-, .-. .~~~:~-.   ~::;:,       \n";
	wcout << L"                   .~:;;-;~:;;:.~. --~~-~,-..  .~;;:.       \n";
	wcout << L"                 .:;;;;~::  ..~;;-:.-.-   ,~    .~:         \n";
	wcout << L"                -~:~~::-; ---~:;;;;:--  ,.,    ,~:          \n";
	wcout << L"               ~:~,. ,:-;,.-- ,----:::~ ,     ~::.          \n";
	wcout << L"              ~;. .   ,;;;;:;;;-.~,.:;:- :-~.~;:            \n";
	wcout << L"            .~:...    .~~;;;;;:~-  .~;:~:~~:;~,             \n";
	wcout << L"           .~:. -.     ,~::;;;:,,~~~:~::;:--,               \n";
	wcout << L"          .~:  :  ..     .~,:;;;;;;::;;;.                   \n";
	wcout << L"          ~:..-.,:--;-   ..-~-~;;;:;:~,                     \n";
	wcout << L"         ~;:... ~-.,-:  ..,  .~-;;;:,                       \n";
	wcout << L"        ~.,~;   -;,,:~ .,   ::;~;;..                        \n";
	wcout << L"       -~ ~~:~   -::- .,. -.~;~;:,                          \n";
	wcout << L"      .~: --:-       -.  .,  :;-.                           \n";
	wcout << L"      :;;;~,;~      ..  ~   ;;-                              \n";
	wcout << L"      .:~:::;~     .- .-. .:;-                               \n";
	wcout << L"       .-:~;;~        ., .:;-                                \n";
	wcout << L"       . .:;;;-   ~:;::~.;;-.                                \n";
	wcout << L"           ~:;;--;:;~:~:;;-                                 \n";
	wcout << L"            .~;;;;-~~-;:;-                                  \n";
	wcout << L"             ..:;;~:::.~-                                    \n";
	wcout << L"                ::-:;-~-,                                    \n";
	wcout << L"                 ,:~;:~,                                     \n";
	wcout << L"                  .,;;.                                      \n";
	wcout << L"                    ,:                                       \n";

	int curmode = _setmode(_fileno(stdout), prevmode);

	Sleep(1500);
	for (char x : ENDMSG_SUCCESS)
		cout << x, Sleep(100);
	cout << "\n\n";
	string earned_str = to_string(value);

	string str = "번 돈:";
	str+=earned_str;
	for (char x : str)
		cout << x, Sleep(100);
	cout << "\n\n";

	if (value > best_score) {
		ofstream outFile("bestscore.txt");

		outFile << earned_str;
		outFile.close();
	}

	SetColor((int)COLOR::WHITE, (int)COLOR::BLACK);
	system("pause");
}

void Fail() {
	system("cls");
	SetCursorVis(false, 1);

	 
	int prevmode = _setmode(_fileno(stdout), _O_U16TEXT);
	SetColor((int)COLOR::LIGHT_RED, (int)COLOR::BLACK);
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡠⣀⢧⢴⢥⢼⣀⡠⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠐⢲⡪⡣⡃⢇⢇⢇⢇⢗⡖⠂⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢙⡇⢇⢪⢸⢨⢢⢣⢣⢣⢫⡋⠀⠀⠀⠀⠀⠀⠀⡀⠀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠐⢱⢇⢣⠱⡑⡌⡆⡇⡇⢧⢹⡞⠂⠀⠀⠀⠀⡤⡜⢞⢪⢳⢤⠆⠀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠐⢪⡇⡣⡃⡇⡕⢜⢜⢜⢕⢕⡗⠂⠀⠀⠀⠠⣎⢇⢣⢱⢱⢹⡆⠄⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠐⢼⡪⡸⡐⡕⢜⢸⢸⢸⢸⢸⠧⠂⠀⠀⠀⠰⡇⡇⡕⢜⢜⢜⣎⡀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠠⢼⡪⠢⡣⠪⡊⡎⢎⢎⢎⢮⠧⠂⠀⠀⠀⠤⡗⡕⢜⢌⢖⢕⡇⠁⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠠⢜⡎⡕⡌⢇⢕⢜⢕⢕⢕⢕⡧⠄⠀⠀⠀⣀⡏⡎⡢⢣⢪⢪⠧⠄⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⠀⢀⠀⠀⠀⠀⠀⠀⠀⣜⡎⢆⢣⠣⡱⢸⢸⢸⢸⢸⡣⡀⠀⠀⠀⣄⡏⡎⢜⢌⢎⢮⡇⠀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠰⡦⢞⠫⡛⡦⡤⠀⠀⠀⠀⢀⣸⢪⢊⢆⢣⢱⢑⢕⢕⢕⠵⡇⠀⡀⠀⣆⢴⢣⠱⡑⡅⣇⣳⠓⠁⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠠⢼⡪⡣⡣⡱⡸⢵⠄⠀⠀⠀⠀⢸⢕⢱⢘⢌⢆⠇⡇⡇⡇⣝⡇⡗⢝⢝⢱⢑⢅⢇⢣⢣⢲⠳⠄⠀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⣀⣸⢎⢎⢆⢕⢌⢷⠆⠀⠀⠀⠈⢹⢕⢅⢣⢱⢘⢜⢜⢜⢜⢜⡇⡎⡪⢢⠣⡱⡸⡸⡸⡼⢭⠀⠀⠀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⢱⢇⢇⢕⠜⡔⡳⠔⠀⠀⠀⠐⢱⢇⢕⢱⢘⢌⢆⢇⢇⢇⡳⣇⢇⢎⢎⣎⣎⠮⠎⠯⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠐⢾⡱⡱⡡⢣⠪⣝⡄⠀⠀⠀⠠⢮⢇⠎⡆⡣⡱⡸⡸⡸⡸⡸⡇⠉⠉⡏⠈⠀⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⢸⢎⢎⢪⠪⡸⣸⡠⠀⠀⠀⢀⢸⢕⢅⢣⢱⢘⢔⢕⢕⢕⢭⡇⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠈⠩⣇⢇⢇⢕⠕⡜⢵⣰⠀⡠⡀⢩⢇⢎⠪⡂⡇⢕⢕⢕⢕⢕⡷⠄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠐⢓⣇⢇⢇⢕⠜⡜⢌⢫⠹⡪⡫⡇⢎⢪⠪⡸⡘⡜⡜⡜⣜⣇⠄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠀⠠⡚⢮⡪⣒⢕⢱⢑⢕⠱⡑⢼⡣⢣⠱⡡⢣⠪⡪⡪⡪⡪⡆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠏⠚⢮⢎⣎⢎⡎⣎⢞⡕⢕⠱⡑⡅⡇⡇⡇⢧⢹⡍⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠀⠈⠹⠉⠉⢩⢇⢣⢃⢇⢕⢜⢜⢜⢕⢕⡗⠂⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠠⢼⢕⢅⢇⢪⠢⡱⡱⡱⡱⡱⡧⠄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡸⡕⡌⡆⡣⡱⡑⡕⡕⡕⣹⣅⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢩⡇⢎⢜⢌⢆⢣⢣⢣⢣⢣⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⢳⡣⢣⠱⡨⡢⢣⢣⢣⢣⢳⡋⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n";
	wcout << L"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣪⣪⣪⣪⣜⣬⣪⢎⣮⡪⡮⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀\n";

	int curmode = _setmode(_fileno(stdout), prevmode);

	Sleep(1500);
	for (char x : ENDMSG_FAIL)
		cout << x, Sleep(100);
	cout << "\n\n";

	SetColor((int)COLOR::WHITE, (int)COLOR::BLACK);
	system("pause");
}

void Escape(PPLAYER _pPlayer) {
	system("cls");
	SetColor((int)COLOR::WHITE, (int)COLOR::BLACK);
	cout << "탈출 성공~";
	Sleep(1500);
	cout << "\n\n";
	cout << "당신의 생존 확률:" << _pPlayer->surv_percentage;
	cout << "\n\n";


	for (int i = 0; i < 3; i++) {
		cout << '.';
		Sleep(1500);
	}

	uniform_int_distribution<int> dis(1, 99);
	int ran = dis(gen);

	if (ran <= _pPlayer->surv_percentage)
		Survive(_pPlayer->value);
	else
		Fail();


	Sleep(2000);
}

void FrameSync(unsigned int _Framerate)
{
	clock_t oldtime = clock();
	clock_t curtime;

	while (true)
	{
		curtime = clock();
		if (curtime - oldtime > 1000 / _Framerate)
		{
			oldtime = curtime;
			break;
		}
	}
}

void GameInfo() {

}

void GameStart() {
	system("cls");

	static vector<char> arrmap[MAP_HEIGHT][MAP_WIDTH];

	ifstream fin;
	fin.open("GPMap.txt");
	string line;
	int row = 0;
	vector<POS> leftSpot;
	while (getline(fin, line) && row < MAP_HEIGHT) {
		for (int col = 0; col < line.length() && col < MAP_WIDTH; ++col) {
			if (Moveable((OBJ_TYPE)line[col])) {
				arrmap[row][col].push_back((char)OBJ_TYPE::ROAD);
				if (row < 5 || row>7 || col % 3 == row % 3)
					leftSpot.push_back(POS{ col,row });
			}
			arrmap[row][col].push_back(line[col]);
		}
		++row;
	}

	fin.close();

	ItemInit();

	PLAYER player;
	player.tPos = { 1,6 };

	SetCursorVis(false, 1);
	SetFontSize(FW_BOLD, 45, 45);
	SpreadItem(arrmap, leftSpot);


	start_time = clock(), left_time = GAME_PLAYTIME;
	while (true) {
		if (!TimeUpdate()) {
			FireEnding();
			break;
		}
		if (!MoveUpdate(arrmap, &player)) {
			Escape(&player);
			break;
		}
		PickUpdate(arrmap, &player);
		ItemUpdate(arrmap, &player);
		Render(arrmap, &player);
		FrameSync(120);
	}
}

bool MainMenu() {
	system("cls");

	bool quit_game = false;
	Gotoxy(0, 5);
	SetFontSize(FW_BOLD, 23, 23);

	int prevmode = _setmode(_fileno(stdout), _O_U16TEXT);
	wcout << L"           ██████╗██████╗  █████╗ ███████╗██╗  ██╗    ██╗      █████╗ ███╗   ██╗██████╗ ██╗███╗   ██╗ ██████╗ \n";
	wcout << L"          ██╔════╝██╔══██╗██╔══██╗██╔════╝██║  ██║    ██║     ██╔══██╗████╗  ██║██╔══██╗██║████╗  ██║██╔════╝ \n";
	wcout << L"          ██║     ██████╔╝███████║███████╗███████║    ██║     ███████║██╔██╗ ██║██║  ██║██║██╔██╗ ██║██║  ███╗\n";
	wcout << L"          ██║     ██╔══██╗██╔══██║╚════██║██╔══██║    ██║     ██╔══██║██║╚██╗██║██║  ██║██║██║╚██╗██║██║   ██║\n";
	wcout << L"          ╚██████╗██║  ██║██║  ██║███████║██║  ██║    ███████╗██║  ██║██║ ╚████║██████╔╝██║██║ ╚████║╚██████╔╝\n";
	wcout << L"           ╚═════╝╚═╝  ╚═╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝    ╚══════╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝ ╚═╝╚═╝  ╚═══╝ ╚═════╝ \n";

	int curmode = _setmode(_fileno(stdout), prevmode);
	Sleep(10);

	ifstream fin;
	fin.open("bestscore.txt");
	string score_str;
	getline(fin, score_str);
	Gotoxy(41, 20);
	best_score = stoi(score_str);
	cout << "당신의 최고점수:" << best_score;

	int x = 45;
	int y = 25;
	int originy = y;
	Gotoxy(x, y);
	cout << "게임 시작";
	Gotoxy(x, y + 1);
	cout << "스토리 보기";
	Gotoxy(x, y + 2);
	cout << "종료 하기";

	Gotoxy(x - 2, y);
	cout << ">";
	while (true)
	{
		//cin.ignore();
		cin.clear();
		while (!_kbhit()) { 0; }
		int ch = _getch();

		if (ch == 32) {
			switch (y - originy) {
			case 0:
				GameStart();
				break;
			case 1:
				ShowTitle();
				break;
			case 2:
				quit_game = true;
				break;
			}
			break;
		}
		else if (ch == 224) {
			ch = _getch();
			switch (ch)
			{
			case 72:
			{
				if (y > originy)
				{
					Gotoxy(x - 2, y);
					cout << " ";
					Gotoxy(x - 2, --y);
					cout << ">";
				}
			}
			break;
			case 80:
			{
				if (y < originy + 2)
				{
					Gotoxy(x - 2, y);
					cout << " ";
					Gotoxy(x - 2, ++y);
					cout << ">";
				}
			}
			break;
			default:
				break;
			}
		}
	}
	if (quit_game)
		return false;
	return true;
}

int main() {
	while (MainMenu()) { 0; }
}