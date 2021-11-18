#include "stdfx.h"
#include "Network.h"
#include "GameObject.h"


void GameObject::Update(float elapsedTime, char* buf, int& bufStart)
{
	if (isMove) {
		sc_packet_move_obj pk;
		pk.packetSize = sizeof(pk);
		pk.packetType = SC_PACKET_MOVE_OBJ;
		pk.lookDir = direction;
		pk.objectID = id;

		short speed = static_cast<short>( 100 * elapsedTime);
		switch (direction)
		{
		case (char)DIR::N:
			//pos.x += 1;
			pos.y -= speed;
			break;
		case (char)DIR::NE:
			pos.x += speed;
			pos.y -= speed;
			break;

		case (char)DIR::NW:
			pos.x -= speed;
			pos.y -= speed;
			break;

		case (char)DIR::S:
			//pos.x += 1;
			pos.y += speed;
			break;

		case (char)DIR::SE:
			pos.x += speed;
			pos.y -= speed;
			break;

		case (char)DIR::SW:
			pos.x -= speed;
			pos.y -= speed;
			break;

		case (char)DIR::E:
			pos.x -= speed;
			break;

		case (char)DIR::W:
			pos.x += speed;
			break;

		default:
			break;
		}
		pk.x = pos.x;
		pk.y = pos.y;
		memcpy(buf + bufStart, &pk, sizeof(pk));
		bufStart += sizeof(pk);
	}


}


void Player::Send(void* Packet) const
{
	int retval = send(sock, reinterpret_cast<char*>(Packet), reinterpret_cast<packet*>(Packet)->packetSize, 0);
	std::cout << "[TCP ����]" << retval << "����Ʈ ���½��ϴ�\n";
}

void Player::Recv() {
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
		retval = recv(sock, reinterpret_cast<char*>((&recvPecket)) + 2, pkSize.packetSize - 2, MSG_WAITALL);

		std::cout << "ID: " << id << " , Skin : " << (int)recvPecket.playerSkin << std::endl;
		isActive = true;
		direction = (char)DIR::N;

		SendLogIn();
		/*
		* ���� ������ Ŭ���̾�Ʈ���� ���� �׷����� �÷��̾ �˷���
		*/
		for (const auto Client : net->GameObjects) {
			if (false == Client->isActive) continue;
			if (id == Client->GetId()) continue;
			net->send_put_obj(id, Client->GetId());
		}
		/*
		* �� Ŭ���̾�Ʈ������ ���ο� �÷��̾ ���������� �÷��̾� ������Ʈ�� �����϶����
		*/
		for (const auto& Client : net->GameObjects) {
			if (false == net->is_player(Client->GetId())) continue;
			if (id == Client->GetId()) continue;
			net->send_put_obj(Client->GetId(), id);
		}
	}
	break;
	case CS_PACKET_PLAYER_MOVE:
	{
		cs_packet_player_move recvPecket;
		retval = recv(sock, reinterpret_cast<char*>(&recvPecket) + 2, pkSize.packetSize - 2, MSG_WAITALL);

		std::cout << "ID: " << id << " , cs_packet_player_move : " << (int)recvPecket.dir << std::endl;
		/*
		* �� Ŭ���̾�Ʈ������ �÷��̾ �̵������� �ش� �÷��̾� ������Ʈ�� �̵� �϶����
		*/
		direction = (char)recvPecket.dir;

		isMove = true;

	}
	break;
	case CS_PACKET_PLAYER_STATE:
	{
		cs_packet_player_state recvPecket;
		retval = recv(sock, reinterpret_cast<char*>(&recvPecket) + 2, pkSize.packetSize - 2, MSG_WAITALL);
		switch (recvPecket.playerState)
		{
		case (char)STATE::idle:
		{
			isMove = false;
		}
		break;

		case (char)STATE::move:
		{
			isMove = true;
		}
		break;
		case (char)STATE::attack:
		{
			
		}
		break;
		default:
			break;
		}
	}
	break;
	case CS_PACKET_SHOOT_BULLET:
	{
		cs_packet_shoot_bullet recvPecket;
		retval = recv(sock, reinterpret_cast<char*>(&recvPecket) + 2, pkSize.packetSize - 2, MSG_WAITALL);

		std::cout << "ID: " << id << " , cs_packet_player_move : " << (int)recvPecket.shootX << " " << (int)recvPecket.shootY << " " << (int)recvPecket.dir << std::endl;
		/*
		* �� Ŭ���̾�Ʈ������ �÷��̾ ���� �߻� �ش� �÷��̾� ������Ʈ�� Render �϶����
		*/
		//for (const auto& Client : net->GameObjects) {
		//	if (Client.id != id) {//
		//		//net->send_move_obj(Client.id);
		//	}
		//}

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