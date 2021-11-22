#include "stdfx.h"

#include "Network.h"


Network* Network::instance = nullptr;

Network* Network::GetInstance()
{
	return instance;
}
void Network::ProcessClient(int id)
{
	Player* client = reinterpret_cast<Player*>(GameObjects[id]);
	

	while (client->Recv()) {

	}

	return;
}
void Network::AcceptThread() {
	SOCKADDR_IN ClientAddr;
	int addrlen;
	int id = 0;
	while (true) {
		
		id = get_player_id();
		if (id == -1) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
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

void Network::send_put_obj(int id, const int target) {
	sc_packet_put_obj sendPutPacket;
	sendPutPacket.packetSize = sizeof(sendPutPacket);
	sendPutPacket.packetType = SC_PACKET_PUT_OBJ;
	sendPutPacket.x = GameObjects[target]->pos.x;
	sendPutPacket.y = GameObjects[target]->pos.y;
	sendPutPacket.width = GameObjects[target]->width;
	sendPutPacket.height = GameObjects[target]->height;
	sendPutPacket.objectID = target;
	sendPutPacket.sprite = GameObjects[target]->sprite;
	reinterpret_cast<Player*>(GameObjects[id])->Send(&sendPutPacket);
}

void Network::send_move_obj(int id, int mover) {
	sc_packet_move_obj sendMovePacket;
	sendMovePacket.packetSize = sizeof(sendMovePacket);
	sendMovePacket.packetType = SC_PACKET_MOVE_OBJ;
	sendMovePacket.objectID = mover;
	sendMovePacket.lookDir = GameObjects[mover]->direction;

	sendMovePacket.x = GameObjects[mover]->pos.x;
	sendMovePacket.y = GameObjects[mover]->pos.y;
	reinterpret_cast<Player*>(GameObjects[id])->Send(&sendMovePacket);
}

void Network::send_change_state(int id, int target) {
	sc_packet_player_state sendPacket;
	sendPacket.packetSize = sizeof(sendPacket);
	sendPacket.packetType = SC_PACKET_PLAYER_STATE;
	sendPacket.objectID = target;
	sendPacket.playerState = (char)reinterpret_cast<Player*>(GameObjects[id])->state;

	reinterpret_cast<Player*>(GameObjects[id])->Send(&sendPacket);
}

void Network::send_change_hp(int id, int target) {
	sc_packet_change_hp sendPacket;
	sendPacket.packetSize = sizeof(sendPacket);
	sendPacket.packetType = SC_PACKET_PLAYER_STATE;
	sendPacket.playerID = target;
	sendPacket.hp = 10;

	reinterpret_cast<Player*>(GameObjects[id])->Send(&sendPacket);
}

void Network::send_remove_obj(int id, int victm) {
	sc_packet_remove_obj sendPacket;
	sendPacket.packetSize = sizeof(sendPacket);
	sendPacket.packetType = SC_PACKET_REMOVE_OBJ;
	sendPacket.objectID = victm;
	
	reinterpret_cast<Player*>(GameObjects[id])->Send(&sendPacket);
}

void Network::update(float elapsedTime) {
	int bufstart = 0;
	for (auto obj : GameObjects) {
		//if (!obj)continue;
		if (false == obj->isActive) continue;
		obj->Update(elapsedTime, buf, bufstart);
		
	}

	if (0 < bufstart)
		for (int i = 0; i < MAX_USER; ++i) {
			if (false == GameObjects[i]->isActive) continue;
			reinterpret_cast<Player*>(GameObjects[i])->Send(buf);
		}
	//플레이어가 한 프레임 마다 생성된 버퍼 전송
}