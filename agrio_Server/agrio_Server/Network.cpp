#include "stdfx.h"
#include "Network.h"

Network* Network::instance = nullptr;

short CUR_WINDOW_WIDTH = 900 * 0.75f;      //윈도우 x사이즈
short CUR_WINDOW_HEIGHT = 800 * 0.75f;
short CUR_WINDOW_START_X = 15;
short CUR_WINDOW_START_Y = 15;

Network* Network::GetInstance()
{
	return instance;
}
void Network::ProcessClient(int id)
{
	Player* client = reinterpret_cast<Player*>(GameObjects[id]);

	while (client->Recv());

	return;
}

void Network::LobbyThread() {
	SOCKADDR_IN ClientAddr;
	int addrlen;
	int id = 0;
	while (true) {

		id = GetPlayerId();
		if (id == -1) {

			std::this_thread::sleep_for(std::chrono::seconds(1));
			// 게임 시작 패킷 보내고
			// 게임시작
			// 게임 중에는 accept를 받지 않음, 게임이 시작하면 accpet를 다시 시작하자
			// return;
			continue;
		}
		addrlen = sizeof(ClientAddr);
		SOCKET client_sock = accept(listen_sock, (SOCKADDR*)&ClientAddr, &addrlen);

		GameObjects[id]->isActive = true;
		GameObjects[id]->id = id;
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			return;
		}
		std::cout << "\n[TCP 서버] 클라이언트 접속: IP 주소=" << inet_ntoa(ClientAddr.sin_addr)
			<< "포트 번호=" << ntohs(ClientAddr.sin_port) << std::endl;

		reinterpret_cast<Player*> (GameObjects[id])->SetSockId(client_sock, id);

		threads.emplace_back(&Network::ProcessClient, Network::GetInstance(), id);
	}
}

void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf, 0, NULL
	);
	MessageBoxA(NULL, (LPCSTR)lpMsgBuf, (LPCSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}
void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&lpMsgBuf, 0, NULL
	);
	MessageBoxA(NULL, (LPCSTR)lpMsgBuf, (LPCSTR)msg, MB_ICONERROR);

	LocalFree(lpMsgBuf);
}

void Network::SendLoginOk(int id) {
	sc_packet_login_ok sendPacket;
	sendPacket.packetSize = sizeof(sendPacket);
	sendPacket.packetType = SC_PACKET_LOGIN_OK;
	sendPacket.playerID = id;
	sendPacket.x = (short)800;
	sendPacket.y = (short)900;
	sendPacket.width = PLAYER_WIDTH;
	sendPacket.height = PLAYER_HEIGHT;

	reinterpret_cast<Player*>(GameObjects[id])->UpdateBuf(&sendPacket, sendPacket.packetSize);
}
void Network::SendPutObj(int id, const int target) {
	sc_packet_put_obj sendPutPacket;
	sendPutPacket.packetSize = sizeof(sendPutPacket);
	sendPutPacket.packetType = SC_PACKET_PUT_OBJ;
	sendPutPacket.x = GameObjects[target]->pos.x;
	sendPutPacket.y = GameObjects[target]->pos.y;
	sendPutPacket.width = GameObjects[target]->width;
	sendPutPacket.height = GameObjects[target]->height;
	sendPutPacket.objectID = target;
	sendPutPacket.sprite = GameObjects[target]->sprite;
	reinterpret_cast<Player*>(GameObjects[id])->UpdateBuf(&sendPutPacket, sendPutPacket.packetSize);
}
void Network::SendMoveObj(int id, int mover) {
	sc_packet_move_obj sendMovePacket;
	sendMovePacket.packetSize = sizeof(sendMovePacket);
	sendMovePacket.packetType = SC_PACKET_MOVE_OBJ;
	sendMovePacket.objectID = mover;
	sendMovePacket.lookDir = GameObjects[mover]->direction;

	sendMovePacket.x = GameObjects[mover]->pos.x;
	sendMovePacket.y = GameObjects[mover]->pos.y;
	reinterpret_cast<Player*>(GameObjects[id])->UpdateBuf(&sendMovePacket, sendMovePacket.packetSize);
}
void Network::SendChangeState(int id, int target) {
	sc_packet_player_state sendPacket;
	sendPacket.packetSize = sizeof(sendPacket);
	sendPacket.packetType = SC_PACKET_PLAYER_STATE;
	sendPacket.objectID = target;
	sendPacket.playerState = (char)(reinterpret_cast<Player*>(GameObjects[target])->state);

	reinterpret_cast<Player*>(GameObjects[id])->UpdateBuf(&sendPacket, sendPacket.packetSize);
}
void Network::SendChangeHp(int id, int target) {
	sc_packet_change_hp sendPacket;
	sendPacket.packetSize = sizeof(sendPacket);
	sendPacket.packetType = SC_PACKET_CHANGE_HP;
	sendPacket.playerID = target;
	sendPacket.hp = reinterpret_cast<Player*>(GameObjects[target])->hp;

	reinterpret_cast<Player*>(GameObjects[id])->UpdateBuf(&sendPacket, sendPacket.packetSize);
}
void Network::SendChangeScene(int id, char snum) {
	sc_packet_change_scene sendPacket;
	sendPacket.packetSize = sizeof(sendPacket);
	sendPacket.packetType = SC_PACKET_CHANGE_SCENE;
	sendPacket.sceneNum = snum;

	reinterpret_cast<Player*>(GameObjects[id])->UpdateBuf(&sendPacket, sendPacket.packetSize);
}
void Network::SendRemoveObj(int id, int victm) {
	sc_packet_remove_obj sendPacket;
	sendPacket.packetSize = sizeof(sendPacket);
	sendPacket.packetType = SC_PACKET_REMOVE_OBJ;
	sendPacket.objectID = victm;
	GameObjects[victm]->collisionCount = 0;
	if (false == IsPlayer(victm))
		GameObjects[victm]->isActive = false;

	reinterpret_cast<Player*>(GameObjects[id])->UpdateBuf(&sendPacket, sendPacket.packetSize);
}
void Network::SendRemoveObj_2(int id, int victm) {
	sc_packet_remove_obj sendPacket;
	sendPacket.packetSize = sizeof(sendPacket);
	sendPacket.packetType = SC_PACKET_REMOVE_OBJ;
	sendPacket.objectID = victm;
	GameObjects[victm]->collisionCount = 0;

	reinterpret_cast<Player*>(GameObjects[id])->UpdateBuf(&sendPacket, sendPacket.packetSize);
}
void Network::SendGetItem(int id, int itemtype) {
	sc_packet_get_item sendPacket;
	sendPacket.packetSize = sizeof(sendPacket);
	sendPacket.packetType = SC_PACKET_GET_ITEM;
	sendPacket.playerID = id;
	sendPacket.itemID = itemtype;
	Player* re = reinterpret_cast<Player*>(GameObjects[id]);
	sendPacket.itemCount = re->items[itemtype];

	reinterpret_cast<Player*>(GameObjects[id])->UpdateBuf(&sendPacket, sendPacket.packetSize);
}
void Network::SendChangeWeapon(int id, int target)
{
	sc_packet_change_weapon sendPacket;
	sendPacket.packetSize = sizeof(sendPacket);
	sendPacket.packetType = SC_PACKET_CHAGE_WEAPON;
	sendPacket.playerID = target;
	sendPacket.gunID = reinterpret_cast<Player*>(GameObjects[target])->curEquip;

	reinterpret_cast<Player*>(GameObjects[id])->UpdateBuf(&sendPacket, sendPacket.packetSize);
}

void Network::Update(float elapsedTime) {
	int bufstart = 0;
	for (auto obj : GameObjects) {
		//if (!obj)continue;
		if (false == obj->isActive) continue;
		obj->Update(elapsedTime, buf, bufstart);
	}

	// 모두 READY 상태인지 검사
	if (MyScene == SCENE::lobby) {
		int ready_count = 0;
		for (int i = 0; i < MAX_USER; ++i) {
			Player* p = reinterpret_cast<Player*>(GameObjects[i]);
			if (p->isActive && p->isReady) {
				ready_count++;
			};
		}
		
		if (ready_count == MAX_USER) {
			for (int i = 0; i < MAX_USER; ++i) {
				SendChangeScene(i, (char)SCENE::stage1);
				MyScene = SCENE::stage1;
				GameObjects[WALL_ID_UP]->isMove = true;
				GameObjects[WALL_ID_DOWN]->isMove = true;
				GameObjects[WALL_ID_LEFT]->isMove = true;
				GameObjects[WALL_ID_RIGHT]->isMove = true;
				CUR_WINDOW_WIDTH = WINDOW_WIDTH * 0.75f;
				CUR_WINDOW_HEIGHT = WINDOW_HEIGHT * 0.75f;
				CUR_WINDOW_START_X = 15;
				CUR_WINDOW_START_Y = 15;
				for (int j = WALL_ID_RIGHT+1; j < MAX_OBJECT; ++j) {
					if (false == GameObjects[j]->isActive) continue;
					SendRemoveObj(i, j);
				}
			}
		}
		ready_count = 0;
	}

	else if (MyScene == SCENE::stage1) {

		if (CUR_WINDOW_START_X < 400 && std::chrono::system_clock::now() - preWallMoveTime > std::chrono::milliseconds(WallMoveTime)) {
			preWallMoveTime = std::chrono::system_clock::now();
			CUR_WINDOW_START_X += 1;
			CUR_WINDOW_START_Y += 1;
			CUR_WINDOW_WIDTH -= 2;
			CUR_WINDOW_HEIGHT -= 2;
			GameObjects[WALL_ID_UP]->pos.y += 1;
			GameObjects[WALL_ID_DOWN]->pos.y -= 1;

			GameObjects[WALL_ID_LEFT]->pos.x += 1;
			GameObjects[WALL_ID_RIGHT]->pos.x -= 1;

			for (int j = 0; j < MAX_USER; ++j) {
				if (false == GameObjects[j]->isActive) continue;
				SendMoveObj(j, WALL_ID_UP);
				SendMoveObj(j, WALL_ID_DOWN);
				SendMoveObj(j, WALL_ID_LEFT);
				SendMoveObj(j, WALL_ID_RIGHT);
			}
		}
		if (std::chrono::system_clock::now() - preItemSpawnTime > std::chrono::seconds(ItemSpawnTime)) {
			preItemSpawnTime = std::chrono::system_clock::now();
			int obj_id = GetObjID();

			if (obj_id == -1) {
				std::cout << "모든 오브젝트를 사용하였습니다." << std::endl;
				return;
			}

			GameObject* pistol = GameObjects[obj_id];
			pistol->direction = rand() % 8;
			pistol->velocity = VELOCITY;
			pistol->width = BLOCK_WIDTH;
			pistol->height = BLOCK_WIDTH;
			pistol->id = obj_id;
			pistol->sprite = (char)SPRITE::uiPistol + rand() % 5;
			pistol->type = ITEM;
			pistol->isActive = true;
			pistol->isMove = false;
			pistol->pos = Coordinate{ short(CUR_WINDOW_START_X + rand() % CUR_WINDOW_WIDTH),short(CUR_WINDOW_START_Y + rand() % CUR_WINDOW_HEIGHT) };

			for (int i = 0; i < MAX_USER; ++i) {
				Player* p = reinterpret_cast<Player*>(GameObjects[i]);
				if (false == p->isActive) continue;

				SendPutObj(i, obj_id);
			}
		}

		// 1명 남았는지 검사
		int playerCount = MAX_USER;
		for (int i = 0; i < MAX_USER; ++i) {
			Player* p = reinterpret_cast<Player*>(GameObjects[i]);
			if (p->hp <= 0) playerCount--;
		}
		if (playerCount == 1) {
			for (int i = 0; i < MAX_USER; ++i) {
				Player* p = reinterpret_cast<Player*>(GameObjects[i]);
				if (p->hp > 0)
					SendChangeScene(i, (char)SCENE::winner);
			}
			MyScene = SCENE::winner;
		}
	}

	else if (MyScene == SCENE::winner) { 
		// 캐릭터 초기화
		for (int i = 0; i < MAX_USER; ++i) {
			(GameObjects[i])->isActive = true;	
			(GameObjects[i])->isAttack = false;
			(GameObjects[i])->isReady = false;
			(GameObjects[i])->velocity = PLAYER_SPEED;
			(GameObjects[i])->pos.x = (short)1800;
			(GameObjects[i])->pos.y = (short)1900;
			(GameObjects[i])->direction = (char)DIR::N;

			reinterpret_cast<Player*>(GameObjects[i])->hp = 50;						// 체력
			reinterpret_cast<Player*>(GameObjects[i])->state = STATE::idle;
			reinterpret_cast<Player*>(GameObjects[i])->curEquip = ITEM::empty;
				
			for (int j = 0; j < 5; ++j) {	// 아이템
				reinterpret_cast<Player*>(GameObjects[i])->items[j] = 0;
				SendGetItem(i, j);
			}
			for (int j = 0; j < MAX_USER; ++j) { // 플레이어들에게 알림
				SendPutObj(j, i);
				SendChangeHp(j, i);
				SendChangeState(j, i);
				SendChangeWeapon(j, i);
				SendMoveObj(j, i);
			}
		}

		// 윈도우 크기 초기화
		CUR_WINDOW_WIDTH = WINDOW_WIDTH * 0.75f;
		CUR_WINDOW_HEIGHT = WINDOW_HEIGHT * 0.75f;
		CUR_WINDOW_START_X = 15;
		CUR_WINDOW_START_Y = 15;

		//  각 플레이어들에게 쓴 오브젝트 삭제 패킷전송
		for (int i = 0; i < MAX_USER; ++i) {
			for (int j = WALL_ID_RIGHT + 1; j < MAX_OBJECT; ++j) {
				if (false == GameObjects[j]->isActive) continue;
				SendRemoveObj_2(i, j);
			}
		}

		// 서버에서 오브젝트를 삭제
		for (int j = WALL_ID_RIGHT + 1; j < MAX_OBJECT; ++j) {
			GameObjects[j]->isActive = false;
		}

		// 벽초기화
		{
			short objlength = 100;
			// 위
			GameObjects[WALL_ID_UP]->isActive = true;
			GameObjects[WALL_ID_UP]->isMove = false;
			GameObjects[WALL_ID_UP]->velocity = 0;
			GameObjects[WALL_ID_UP]->pos = Coordinate{ WINDOW_WIDTH / 2,0 };
			GameObjects[WALL_ID_UP]->width = WINDOW_WIDTH;
			GameObjects[WALL_ID_UP]->height = objlength;
			GameObjects[WALL_ID_UP]->sprite = (int)SPRITE::wallRow;
			GameObjects[WALL_ID_UP]->type = WALL;
			GameObjects[WALL_ID_UP]->id = 3;
			// 아래
			GameObjects[WALL_ID_DOWN]->isActive = true;
			GameObjects[WALL_ID_DOWN]->isMove = false;
			GameObjects[WALL_ID_DOWN]->velocity = 0;
			GameObjects[WALL_ID_DOWN]->pos = Coordinate{ WINDOW_WIDTH / 2, WINDOW_HEIGHT };
			GameObjects[WALL_ID_DOWN]->width = WINDOW_WIDTH;
			GameObjects[WALL_ID_DOWN]->height = objlength;
			GameObjects[WALL_ID_DOWN]->sprite = (int)SPRITE::wallRow;
			GameObjects[WALL_ID_DOWN]->type = WALL;
			GameObjects[WALL_ID_DOWN]->id = 4;
			// 왼쪽
			GameObjects[WALL_ID_LEFT]->isActive = true;
			GameObjects[WALL_ID_LEFT]->isMove = false;
			GameObjects[WALL_ID_LEFT]->velocity = 0;
			GameObjects[WALL_ID_LEFT]->pos = Coordinate{ 0, WINDOW_HEIGHT / 2 };
			GameObjects[WALL_ID_LEFT]->width = objlength;
			GameObjects[WALL_ID_LEFT]->height = WINDOW_HEIGHT;
			GameObjects[WALL_ID_LEFT]->sprite = (int)SPRITE::wallCol;
			GameObjects[WALL_ID_LEFT]->type = WALL;
			GameObjects[WALL_ID_LEFT]->id = 5;
			// 오른쪽
			GameObjects[WALL_ID_RIGHT]->isActive = true;
			GameObjects[WALL_ID_RIGHT]->isMove = false;
			GameObjects[WALL_ID_RIGHT]->velocity = 0;
			GameObjects[WALL_ID_RIGHT]->pos = Coordinate{ WINDOW_WIDTH, WINDOW_HEIGHT / 2 };
			GameObjects[WALL_ID_RIGHT]->width = objlength;
			GameObjects[WALL_ID_RIGHT]->height = WINDOW_HEIGHT;
			GameObjects[WALL_ID_RIGHT]->sprite = (int)SPRITE::wallCol;
			GameObjects[WALL_ID_RIGHT]->type = WALL;
			GameObjects[WALL_ID_RIGHT]->id = 6;
		}

		MyScene = SCENE::lobby;
	}

	//플레이어의 이벤트 버퍼에 있는 내용을 전송버퍼로 옮김
	for (int i = 0; i < MAX_USER; ++i) {
		Player* p = reinterpret_cast<Player*>(GameObjects[i]);
		if (false == p->isActive) continue;
		if (0 >= p->bufSize) continue;
		p->Send(p->eventPacketBuf, p->bufSize);
		//memcpy(buf + bufstart, p->eventPacketBuf, p->bufSize);
		//bufstart += p->bufSize;
		p->bufSize = 0;
	}

	if (0 < bufstart) {
		for (int i = 0; i < MAX_USER; ++i) {
			if (false == GameObjects[i]->isActive) continue;
			reinterpret_cast<Player*>(GameObjects[i])->Send(buf, bufstart);
		}
	}
	//플레이어가 한 프레임 마다 생성된 버퍼 전송
}