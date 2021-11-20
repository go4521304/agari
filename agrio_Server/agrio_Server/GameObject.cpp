#include "stdfx.h"
#include "Network.h"
#include "GameObject.h"



void GameObject::Update(float elapsedTime, char* buf, int& bufPos)
{
	if (isMove) {
		sc_packet_move_obj& pk = *reinterpret_cast<sc_packet_move_obj*>(buf);
		pk.packetSize = sizeof(pk);
		pk.packetType = SC_PACKET_MOVE_OBJ;
		pk.lookDir = direction;
		pk.objectID = id;

		short speed = static_cast<short>(velocity * elapsedTime);
		int x = pos.x;
		int y = pos.y;
		
		switch (direction)
		{
		case (char)DIR::N:
			//pos.x += 1;
			y -= speed;
			break;
		case (char)DIR::NE:
			x += speed;
			y -= speed;
			break;

		case (char)DIR::NW:
			x -= speed;
			y -= speed;
			break;

		case (char)DIR::S:
			//pos.x += 1;
			y += speed;
			break;

		case (char)DIR::SE:
			x += speed;
			y += speed;
			break;

		case (char)DIR::SW:
			x -= speed;
			y += speed;
			break;

		case (char)DIR::E:
			x += speed;
			break;

		case (char)DIR::W:
			x -= speed;
			break;

		default:
			break;
		}
		//�浹�� ��� ���� ��ġ�� ���������� ��� ���
		pk.x = pos.x;
		pk.y = pos.y;

		pos.x = x;
		pos.y = y;
		for (auto obj : Network::GetInstance()->GameObjects) {
			if (false == obj->isActive)continue;
			if (id == obj->id)continue;
			if (Network::GetInstance()->is_collision(id, obj->id)) {
				//�浹ó�� ���� ��
				if (Network::GetInstance()->is_player(id)) {
					//hp --
					std::cout << "�浹\n";
					pos.x = pk.x;
					pos.y = pk.y;
				}
				else {
					isActive = false;
					isMove = false;
					for (int i = 0; i < MAX_USER; ++i) {
						if (false == Network::GetInstance()->GameObjects[id]->isActive) continue;
						Network::GetInstance()->send_remove_obj(i, id);
					}
				}
				return;
			}

		}
		pk.x = x;
		pk.y = y;

		memcpy(buf + bufPos, &pk, sizeof(pk));
		bufPos += sizeof(pk);
	}
}



void Player::SendLogIn() {
	sc_packet_login_ok sendPacket;
	sendPacket.packetSize = sizeof(sendPacket);
	sendPacket.packetType = SC_PACKET_LOGIN_OK;
	sendPacket.playerID = id;
	pos.x = sendPacket.x = (short)800;
	pos.y = sendPacket.y = (short)900;
	width = sendPacket.width = PLAYER_WIDTH;
	height = sendPacket.height = PLAYER_HEIGHT;

	Send(&sendPacket);
}
void Player::Send(void* Packet) const
{
	int retval = send(sock, reinterpret_cast<char*>(Packet), reinterpret_cast<packet*>(Packet)->packetSize, 0);
	//std::cout << "[TCP ����]" << retval << "����Ʈ ���½��ϴ�\n";
}

bool Player::Recv() {
	Network* net = Network::GetInstance();
	packet pkSize;
	int retval = recv(sock, reinterpret_cast<char*>(&pkSize), sizeof(packet), MSG_WAITALL);
	if (retval == SOCKET_ERROR) {
		net->disconnect(id);
		err_display("recv()");
		return false;
	}
	std::cout << "Size : " << (int)pkSize.packetSize << std::endl;
	std::cout << "Type : " << (int)pkSize.packetType << std::endl;

	switch (pkSize.packetType)
	{
	case CS_PACKET_LOGIN:
	{
		cs_packet_login recvPecket;
		retval = recv(sock, reinterpret_cast<char*>((&recvPecket)) + 2, pkSize.packetSize - 2, MSG_WAITALL);


		std::cout << "id : " << (int)id << std::endl;
		sprite = recvPecket.playerSkin;
		isActive = true;
		direction = (char)DIR::N;
		type = PLAYER;
		velocity = PLAYER_SPEED;
		SendLogIn();

		/*
* �� Ŭ���̾�Ʈ������ ���ο� �÷��̾ ���������� �÷��̾� ������Ʈ�� �����϶����
*/
		for (int i = 0; i < MAX_USER; ++i) {
			Player* p = reinterpret_cast<Player*>(net->GameObjects[i]);
			if (false == p->isActive) continue;
			if (id == i) continue;
			net->send_put_obj(i, id);
		}

		/*
		* ���� ������ Ŭ���̾�Ʈ���� ���� �׷����� �÷��̾ �˷���
		*/
		for (const auto Client : net->GameObjects) {
			if (false == Client->isActive) continue;
			if (id == Client->GetId()) continue;
			net->send_put_obj(id, Client->GetId());
		}

	}
	break;
	case CS_PACKET_PLAYER_MOVE:
	{
		cs_packet_player_move recvPecket;
		retval = recv(sock, reinterpret_cast<char*>(&recvPecket) + 2, pkSize.packetSize - 2, MSG_WAITALL);

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
			isAttack = false;
			state = STATE::idle;
			for (int i = 0; i < MAX_USER; ++i) {
				//net->send_change_state(i, id);
			}
		}
		break;

		case (char)STATE::move:
		{
			isMove = true;
			state = STATE::move;
			for (int i = 0; i < MAX_USER; ++i) {
				//net->send_change_state(i, id);
			}
		}
		break;
		case (char)STATE::attack:
		{
			state = STATE::attack;
			for (int i = 0; i < MAX_USER; ++i) {
				//net->send_change_state(i, id);
			}
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

		int obj_id = net->get_obj_id();
		GameObject* pistol = net->GameObjects[obj_id];
		pistol->direction = recvPecket.dir;
		pistol->velocity = 50.f;
		pistol->width = BULLET_WIDTH;
		pistol->height = BULLET_HEIGHT;
		pistol->id = obj_id;
		pistol->sprite = (char)SPRITE::bulletN + direction;
		pistol->type = BULLET;
		pistol->isActive = true;
		pistol->isMove = true;
		pistol->pos = Coordinate{ recvPecket.shootX , recvPecket.shootY };

		/*
		* �� Ŭ���̾�Ʈ������ �÷��̾ ���� �߻� �ش� �÷��̾� ������Ʈ�� Render �϶����
		*/
		for (int i = 0; i < MAX_USER;++i) {
			Player* player = reinterpret_cast<Player*>(net->GameObjects[i]);
			if (false == player->isActive) continue;
			//if (id == i) continue;
			net->send_put_obj(i, obj_id);
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
	return true;
}