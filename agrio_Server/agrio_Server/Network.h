#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <winsock2.h>

#include "Protocol.h"
#include "GameObject.h"

void err_quit(const char* msg);
void err_display(const char* msg);

//���
const short BULLET_WIDTH = 11;
const short BULLET_HEIGHT = 11;
const short BLOCK_WIDTH = 20;
const short BLOCK_HEIGHT = 20;

const short WINDOW_WIDTH = 900 * 2;      //������ x������
const short WINDOW_HEIGHT = 800 * 2;
const float VELOCITY = 500.0f;
extern short CUR_WINDOW_WIDTH;      //������ x������
extern short CUR_WINDOW_HEIGHT;
extern short CUR_WINDOW_START_X;
extern short CUR_WINDOW_START_Y;
const int WALL_ID_UP = 3;
const int WALL_ID_DOWN = 4;
const int WALL_ID_LEFT = 5;
const int WALL_ID_RIGHT = 6;

class Network
{
	static Network* instance;
	char buf[BUFSIZE*4];

	int ItemSpawnTime;
	std::chrono::system_clock::time_point preItemSpawnTime;

	int WallMoveTime;
	std::chrono::system_clock::time_point preWallMoveTime;
public:

	std::vector<std::thread> threads;
	std::vector<GameObject*> GameObjects;

	SOCKET listen_sock;
	static Network* GetInstance();
	SCENE MyScene = SCENE::lobby;
	int ReplayCount = 0;

	Network() {
		assert(instance == nullptr);
		instance = this;
		preWallMoveTime = preItemSpawnTime = std::chrono::system_clock::now();
			
		ItemSpawnTime = 3;
		WallMoveTime = 200;
		for (int i = 0; i < MAX_USER; ++i) {
			GameObjects.push_back(new Player);
		}
		for (int i = MAX_USER; i < MAX_OBJECT; ++i) {
			GameObjects.push_back(new GameObject);
		}
		{
			short objlength = 100;
			// ��
			GameObjects[WALL_ID_UP]->isActive = true;
			GameObjects[WALL_ID_UP]->isMove = false;
			GameObjects[WALL_ID_UP]->direction = (int)DIR::S;
			GameObjects[WALL_ID_UP]->velocity = 0;
			GameObjects[WALL_ID_UP]->pos = Coordinate{ WINDOW_WIDTH / 2,0 };
			GameObjects[WALL_ID_UP]->width = WINDOW_WIDTH;
			GameObjects[WALL_ID_UP]->height = objlength;
			GameObjects[WALL_ID_UP]->sprite = (int)SPRITE::wallRow;
			GameObjects[WALL_ID_UP]->type = WALL;
			GameObjects[WALL_ID_UP]->id = 3;
			// �Ʒ�
			GameObjects[WALL_ID_DOWN]->isActive = true;
			GameObjects[WALL_ID_DOWN]->isMove = false;
			GameObjects[WALL_ID_DOWN]->direction = (int)DIR::N;

			GameObjects[WALL_ID_DOWN]->velocity = 0;
			GameObjects[WALL_ID_DOWN]->pos = Coordinate{ WINDOW_WIDTH / 2, WINDOW_HEIGHT };
			GameObjects[WALL_ID_DOWN]->width = WINDOW_WIDTH;
			GameObjects[WALL_ID_DOWN]->height = objlength;
			GameObjects[WALL_ID_DOWN]->sprite = (int)SPRITE::wallRow;
			GameObjects[WALL_ID_DOWN]->type = WALL;
			GameObjects[WALL_ID_DOWN]->id = 4;
			// ����
			GameObjects[WALL_ID_LEFT]->isActive = true;
			GameObjects[WALL_ID_LEFT]->isMove = false;
			GameObjects[WALL_ID_LEFT]->direction = (int)DIR::E;

			GameObjects[WALL_ID_LEFT]->velocity = 0;
			GameObjects[WALL_ID_LEFT]->pos = Coordinate{ 0, WINDOW_HEIGHT / 2 };
			GameObjects[WALL_ID_LEFT]->width = objlength;
			GameObjects[WALL_ID_LEFT]->height = WINDOW_HEIGHT;
			GameObjects[WALL_ID_LEFT]->sprite = (int)SPRITE::wallCol;
			GameObjects[WALL_ID_LEFT]->type = WALL;
			GameObjects[WALL_ID_LEFT]->id = 5;
			// ������
			GameObjects[WALL_ID_RIGHT]->isActive = true;
			GameObjects[WALL_ID_RIGHT]->isMove = false;
			GameObjects[WALL_ID_RIGHT]->direction = (int)DIR::W;
			GameObjects[WALL_ID_RIGHT]->velocity = 0;
			GameObjects[WALL_ID_RIGHT]->pos = Coordinate{ WINDOW_WIDTH, WINDOW_HEIGHT / 2 };
			GameObjects[WALL_ID_RIGHT]->width = objlength;
			GameObjects[WALL_ID_RIGHT]->height = WINDOW_HEIGHT;
			GameObjects[WALL_ID_RIGHT]->sprite = (int)SPRITE::wallCol;
			GameObjects[WALL_ID_RIGHT]->type = WALL;
			GameObjects[WALL_ID_RIGHT]->id = 6;
		}
		WSADATA wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
			return;
	}
	~Network() {
		for (auto& th : threads)
			th.join();
		closesocket(listen_sock);
		WSACleanup();

		for (auto go : GameObjects) {
			delete go;
		}
	}

	void Disconnect(int id) {
		Player* p = reinterpret_cast<Player*>(GameObjects[id]);
		p->isActive = false;
		p->isMove = false;

		for (int i = 0; i < MAX_USER; ++i) {
			if (false == GameObjects[i]->isActive) continue;
			SendRemoveObj(i, id);
		}

		closesocket(p->sock);
		threads[id].join();
	}
	char GetPlayerId() {
		for (int i = 0; i < MAX_USER; ++i) {
			if (false == GameObjects[i]->isActive) return i;
		}
		//std::cout << "can not return player id" << std::endl;
		return -1;
	}
	char GetObjID() {
		for (int i = MAX_USER; i < MAX_OBJECT; ++i) {
			if (false == GameObjects[i]->isActive) return i;
		}
		//std::cout << "can not return object id" << std::endl;
		return -1;
	}
	bool IsPlayer(int id) {
		return (id >= 0) && (id < 3);
	}
	void StartAccept() {
		threads.emplace_back(&Network::LobbyThread, this);
	}
	bool IsCollision(int a_id, int b_id) {
		GameObject* a = GameObjects[a_id];
		GameObject* b = GameObjects[b_id];

		RECT aRect{ a->pos.x - a->width / 2, a->pos.y - a->height / 2,a->pos.x + a->width / 2,  a->pos.y + a->height/2 };
		RECT bRect{ b->pos.x - b->width / 2, b->pos.y - b->height / 2,b->pos.x + b->width / 2,  b->pos.y + b->height/2 };

		RECT tmp;
		if (IntersectRect(&tmp,&aRect, &bRect))
			return true;
		else
			return false;
	}

	void SendLoginOk(int id);
	void SendPutObj(int id, int target);
	void SendMoveObj(int id, int mover);
	void SendChangeState(int id, int target);
	void SendChangeHp(int id, int target);
	void SendChangeScene(int id, char snum);
	void SendRemoveObj(int id, int victm);
	void SendRemoveObj_2(int id, int victm);
	void SendGetItem(int id, int victm);
	void SendChangeWeapon(int id, int target);
	void Update(float elapsedTime);

	void ProcessClient(int id);
	void LobbyThread();
};

