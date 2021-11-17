#include "stdfx.h"
#include "Network.h"
#include "Client.h"

void Client::Send(void* Packet) const
{

	int retval = send(sock, reinterpret_cast<char*>(Packet), reinterpret_cast<packet*>(Packet)->packetSize, 0);
	std::cout << "[TCP ����]" << retval << "����Ʈ ���½��ϴ�\n";

}

void Client::Recv() {
	Network* net = Network::GetInstance();
	packet pkSize;
	int retval = recv(sock, reinterpret_cast<char*>(&pkSize), sizeof(packet), MSG_WAITALL);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		return;
	}
	std::cout << "Size : " << (int)pkSize.packetSize << std::endl;
	std::cout << "Type : " << (int)pkSize.packetType << std::endl;

	switch (pkSize.packetType)
	{
	case CS_PACKET_LOGIN:
	{
		cs_packet_login recvPecket;
		retval = recv(sock, reinterpret_cast<char*>((&recvPecket))+2, pkSize.packetSize - 2, MSG_WAITALL);

		std::cout << "ID: " << id << " , Skin : " << (int)recvPecket.playerSkin << std::endl;

		net->send_login_ok(id);
		/*
		* ���� ������ Ŭ���̾�Ʈ���� ���� �׷����� �÷��̾ �˷���
		*/
		
		//Ŭ���̾�Ʈ�� ���ӿ�����Ʈ�� ���ľߵ� �� ����
		for (const auto& Client : net->Clients) {
			if (Client.id != id) {
				net->send_put_obj(id,Client.id);
			}
		}
		/*
		* �� Ŭ���̾�Ʈ������ ���ο� �÷��̾ ���������� �÷��̾� ������Ʈ�� �����϶����
		*/
		for (const auto& Client : net->Clients) {
			if (Client.id != id) {
				net->send_put_obj(Client.id,id);
			}
		}
	}
	break;
	case CS_PACKET_PLAYER_MOVE:
	{
		cs_packet_player_move recvPecket;
		retval = recv(sock, reinterpret_cast<char*>(&recvPecket)+2, pkSize.packetSize - 2, MSG_WAITALL);

		std::cout << "ID: " << id << " , cs_packet_player_move : " << (int)recvPecket.dir << std::endl;
		/*
		* �� Ŭ���̾�Ʈ������ �÷��̾ �̵������� �ش� �÷��̾� ������Ʈ�� �̵� �϶����
		*/
		for (const auto& Client : net->Clients) {
			if (Client.id != id) {
				net->send_move_obj(Client.id,id);
			}
		}
	}
	break;
	case CS_PACKET_PLAYER_STATE:
	{

	}
	break;
	case CS_PACKET_SHOOT_BULLET:
	{
		cs_packet_shoot_bullet recvPecket;
		retval = recv(sock, reinterpret_cast<char*>(&recvPecket)+2, pkSize.packetSize - 2, MSG_WAITALL);

		std::cout << "ID: " << id << " , cs_packet_player_move : " << (int)recvPecket.shootX << " " << (int)recvPecket.shootY << " " << (int)recvPecket.dir << std::endl;
		/*
		* �� Ŭ���̾�Ʈ������ �÷��̾ ���� �߻� �ش� �÷��̾� ������Ʈ�� Render �϶����
		*/
		for (const auto& Client : net->Clients) {
			if (Client.id != id) {//
				//net->send_move_obj(Client.id);
			}
		}

	}
	break;
	case CS_PACKET_USED_ITEM:
	{

	}
	break;
	default:
		std::cout << "�߸��� ��Ŷ ����";
		break;
	}
}