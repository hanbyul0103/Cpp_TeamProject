#include<Windows.h>
#include<conio.h>
#include<iostream>
#include<algorithm>
#include<fstream>
#include<string>
#include<map>
#include<vector>
#include<queue>
using namespace std;

const int MAP_WIDTH = 50;
const int MAP_HEIGHT = 13;
const int MAG_RANGE = 3;
const int MAG_SPEED = 5;
const time_t ITEM_DURATION = 5000;
const int INF = 1e9;
const string PLAYER_STR = "¡Ù ";	

const int dx[]{ -1,1,0,0,1,1,-1,-1 };
const int dy[]{ 0,0,-1,1,1,-1,1,-1 };

typedef struct _tagpos
{
	int x;
	int y;

	bool operator==(const _tagpos& ref) const {
		return x==ref.x && y==ref.y;
	}
	bool operator!=(const _tagpos& ref) const {
		return !(*this==ref);
	}
	int Distance(const _tagpos& ref) const {
		return abs(x - ref.x) + abs(y - ref.y);
	}
}POS, *PPOS;

typedef struct _tagplayer
{
	POS tPos;
	POS tNewPos;
	bool speedUp = false;
	int speedcnt = 0;
	int magnetcnt = 0;

	float surv_percentage = 0;
	int value = 0;
}PLAYER, *PPLAYER;

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

enum class OBJ_TYPE {
	ROAD = '0', SIT, WALL, START, PLAYER, LDOOR, RDOOR, MAGNET, SPEED, MIRROR, END
};

enum class COLOR
{
	BLACK, BLUE, GREEN, SKYBLUE, RED,
	VOILET, YELLOW, LIGHT_GRAY, GRAY, LIGHT_BLUE,
	LIGHT_GREEN, MINT, LIGHT_RED, LIGHT_VIOLET,
	LIGHT_YELLOW, WHITE, END
};

map<OBJ_TYPE, string> obj_icon;
map<OBJ_TYPE, float> obj_surv;
map<OBJ_TYPE, int> obj_value;

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

void Render(vector<char> _map[MAP_HEIGHT][MAP_WIDTH],PPLAYER _pPlayer) {
	Gotoxy(0, 0);
	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++)
			cout << ((_pPlayer->tPos==POS{j,i}) ? PLAYER_STR : obj_icon[(OBJ_TYPE)_map[i][j][_map[i][j].size() - 1]]);
		cout << '\n';
	}

	cout << "¡â:" << _pPlayer->speedcnt << ' ';
	cout << "¡ú:" << _pPlayer->magnetcnt << ' ';
	cout << "SURV:" << _pPlayer->surv_percentage << ' ';
	cout << "VAL:" << _pPlayer->value << ' ';
}

bool Moveable(OBJ_TYPE obj) {
	return obj != OBJ_TYPE::WALL && obj != OBJ_TYPE::SIT;
}

bool Pickable(OBJ_TYPE obj) {
	return Moveable(obj) && obj != OBJ_TYPE::ROAD;
}

bool isItem(OBJ_TYPE obj) {
	return Pickable(obj)&&obj!=OBJ_TYPE::MAGNET&&obj!=OBJ_TYPE::SPEED;
}

float EaseOutQuad(float x) {
	return 1 - (1 - x) * (1 - x);
}

bool isDGMove(POS x, POS y) {
	return x.x != y.x && x.y != y.y;
}

vector<POS> SearchPath(vector<char> _arrmap[MAP_HEIGHT][MAP_WIDTH], PPLAYER _pPlayer, NODE stNode ) {
	static priority_queue<NODE> pq;
	vector<vector<int>> dis(MAP_HEIGHT, vector<int>(MAP_WIDTH, INF));
	vector<vector<POS>> prev(MAP_HEIGHT, vector<POS>(MAP_WIDTH, {-1,-1}));
	dis[stNode.pos.y][stNode.pos.x] = 0;
	pq.push(stNode);

	while (pq.size()) {
		auto cur = pq.top(); pq.pop();
		if (cur.cost != dis[cur.pos.y][cur.pos.x])
			continue;
		for (int dir = 0; dir < 8; dir++) {
			int nx = cur.pos.x + dx[dir];
			int ny = cur.pos.y + dy[dir];
			int move_time = isDGMove(cur.pos,{nx,ny}) ? 1400 : 1000;

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

void Pick(vector<char> &vec, PPLAYER _pPlayer) {
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
			_pPlayer->surv_percentage += obj_surv[obj];
			_pPlayer->value += obj_value[obj];
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
							*EaseOutQuad(_pPlayer->tPos.Distance({ path[k-1]})/(float)5)/MAG_SPEED;
						move_vec.push_back({ move_time,cnt,path[k - 1],path[k] });
					}
				}
			}
		}
	}

	sort(move_vec.begin(), move_vec.end());
	time_t start_time = clock();
	//cout << move_vec.size();
	for (auto &move : move_vec) {
		cout << "\n\n\n" << move.time;
		while (clock() - start_time < move.time)
			Sleep(1);

		for (int i=0; i < move.quantity; i++) {
			_arrmap[move.to.y][move.to.x].push_back(_arrmap[move.from.y][move.from.x].back());
			_arrmap[move.from.y][move.from.x].pop_back();
			Render(_arrmap,_pPlayer);
		}
	}

	Pick(_arrmap[_pPlayer->tPos.y][_pPlayer->tPos.x], _pPlayer);
}

void MoveUpdate(vector<char> _arrmap[MAP_HEIGHT][MAP_WIDTH], PPLAYER _pPlayer)
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

	if (Moveable((OBJ_TYPE)_arrmap[_pPlayer->tNewPos.y][_pPlayer->tNewPos.x][0]) && (!used || _pPlayer->speedUp))
	{
		if (_pPlayer->tPos.x != _pPlayer->tNewPos.x || _pPlayer->tPos.y != _pPlayer->tNewPos.y) {
			_pPlayer->tPos = _pPlayer->tNewPos;
			used = 1;
		}
	}
}

void PickUpdate(vector<char> _arrmap[MAP_HEIGHT][MAP_WIDTH], PPLAYER _pPlayer) {
	auto& vec = _arrmap[_pPlayer->tPos.y][_pPlayer->tPos.x];
	if (GetAsyncKeyState(0x41) & 0x8000) {
		Pick(vec, _pPlayer);
	}
}

void ItemUpdate(vector<char> _arrmap[MAP_HEIGHT][MAP_WIDTH], PPLAYER _pPlayer) {
	static time_t speedStart;

	if ((GetAsyncKeyState(0x53) & 0x8000)&&!ItemUsing(_pPlayer) && _pPlayer->speedcnt) {
		_pPlayer->speedcnt--;
		_pPlayer->speedUp = true;
		speedStart = clock();
	}

	if(clock() - speedStart>ITEM_DURATION)
		_pPlayer->speedUp = false;

	if ((GetAsyncKeyState(0x44) & 0x8000) && !ItemUsing(_pPlayer) && _pPlayer->magnetcnt) {
		_pPlayer->magnetcnt--;
		UseMagnatic(_arrmap,_pPlayer);
	}
}

void ItemInit() {
	obj_icon[OBJ_TYPE::ROAD] = "  ";
	obj_icon[OBJ_TYPE::WALL] = "¡á ";
	obj_icon[OBJ_TYPE::SIT] = "¤±";
	obj_icon[OBJ_TYPE::SPEED] = "¢Í ";
	obj_icon[OBJ_TYPE::MIRROR] = "¢Á ";
	obj_icon[OBJ_TYPE::MAGNET] = "¡ú ";

	obj_surv[OBJ_TYPE::MIRROR] = 12;

	obj_value[OBJ_TYPE::MIRROR] = 5000;
}

int main() {
	static vector<char> map[MAP_HEIGHT][MAP_WIDTH];

	ItemInit();

	ifstream fin;
	fin.open("GPMap.txt");

	string line;
	int row = 0;
	while (getline(fin, line) && row < MAP_HEIGHT) {
		for (int col = 0; col < line.length() && col < MAP_WIDTH; ++col) {
			if(line[col]!=(char)OBJ_TYPE::WALL&&line[col]!=(char)OBJ_TYPE::SIT)
				map[row][col].push_back((char)OBJ_TYPE::ROAD);
			map[row][col].push_back(line[col]);
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