#include "stdfx.h"

#include "Network.h"


Network* Network::instance = nullptr;

Network* Network::GetInstance()
{
	return instance;
}
void Network::ProcessClient(void* arg)
{
	Client* client = (Client*)arg;


	while (true)
		client->Recv();

	return ;
}
void Network::AcceptThread(void* arg) {
	SOCKET listensock = (SOCKET)arg;
	SOCKADDR_IN ClientAddr;
	int addrlen;
	int id = 0;
	while (id < 4) {
		addrlen = sizeof(ClientAddr);
		SOCKET client_sock = accept(listensock, (SOCKADDR*)&ClientAddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			return;
		}
		std::cout << "\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=" << inet_ntoa(ClientAddr.sin_addr)
			<< "��Ʈ ��ȣ=" << ntohs(ClientAddr.sin_port) << std::endl;


		Clients.emplace_back(client_sock, id++);//�̺κ� �ذ��ؾߵ�

		threads.emplace_back(&Network::ProcessClient, Network::GetInstance(), (void*)client_sock);
		
	}
}
void err_quit(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL
	);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}

void err_display(const char* msg)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL
	);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, (LPCWSTR)msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
}

void Network::send_login_ok(int id) {
	sc_packet_login_ok sendPacket;

	sendPacket.packetSize = sizeof(sendPacket);
	sendPacket.packetType = SC_PACKET_LOGIN_OK;
	sendPacket.playerID = id;
	sendPacket.x = 0;
	sendPacket.y = 0;
	Clients[id].Send(&sendPacket);
}

void Network::send_put_obj(int id) {
	sc_packet_put_obj sendPutPacket;
	sendPutPacket.packetSize = sizeof(sendPutPacket);
	sendPutPacket.packetType = SC_PACKET_PUT_OBJ;
	sendPutPacket.x = 1;
	sendPutPacket.objectID = id;
	Clients[id].Send(&sendPutPacket);
}
void Network::send_move_obj(int id) {
	sc_packet_obj_move sendPutPacket;
	sendPutPacket.packetSize = sizeof(sendPutPacket);
	sendPutPacket.packetType = SC_PACKET_PUT_OBJ;
	// objectID, lookDir, x, y;

	Clients[id].Send(&sendPutPacket);
}

void Network::update() {
	int bufstart = 0;
	for (auto& obj : GameObjects) {
		obj.Update(buf, bufstart);
	}
	for (auto& Client : Clients) {
		Client.Send(buf);
	}
}