#include "stdfx.h"
#include "Network.h"
#include "GameObject.h"



void GameObject::Update(float elapsedTime, char* buf, int& bufStart)
{
	if (isMove) {
		sc_packet_move_obj& pk = *reinterpret_cast<sc_packet_move_obj*>(buf + bufStart);
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
		for (auto* obj : Network::GetInstance()->GameObjects) {
			if (false == obj->isActive)continue;
			if (id == obj->id)continue;
			if (Network::GetInstance()->IsCollision(id, obj->id)) {
				//�浹ó�� ���� ��
				if (Network::GetInstance()->IsPlayer(id)) {
			

					pos.x = pk.x;
					pos.y = pk.y;
				}
				else {
					std::cout << "�浹\n";

					if (Network::GetInstance()->IsPlayer(obj->id) || collisionCount > 2)
					{
						// ü�� ���� ��Ŷ ���� ��
						reinterpret_cast<Player*>(Network::GetInstance()->GameObjects[obj->id])->ChangeHp(ATTACKHP);

						isActive = false;
						isMove = false;
						for (int i = 0; i < MAX_USER; ++i) {

							if (false == Network::GetInstance()->GameObjects[i]->isActive) continue;
							Network::GetInstance()->SendChangeHp(i, obj->id);
							Network::GetInstance()->SendRemoveObj(i, id);
						}
					}
					else {
						collisionCount++;

						switch (direction)
						{
						case (int)DIR::N:
						case (int)DIR::E:
						case (int)DIR::S:
						case (int)DIR::W:
							direction = (direction + 4) % 8;
							break;
						default:
							direction = (direction + 2) % 8;
							break;
						}
						
					}
				}
				return;
			}
		}
		pk.x = x;
		pk.y = y;
		bufStart += sizeof(pk);
	}
}


void Player::UpdateBuf(void* Packet, int packSize) {
	buf_lock.lock();
	memcpy(eventPacketBuf+bufSize, Packet, packSize);
	bufSize += packSize;
	buf_lock.unlock();
}
void Player::Send(void* Packet, int packSize)
{

	int retval = send(sock, reinterpret_cast<char*>(Packet), packSize, 0);
	if (retval == SOCKET_ERROR) {
		std::cout << "���� �߻�" << (int)id << std::endl;
	}
	std::cout << "[TCP ����]" << (int)id << " : " << retval << "����Ʈ ���½��ϴ�\n";
}

bool Player::Recv() {
	Network* net = Network::GetInstance();
	packet pkSize;
	int retval = recv(sock, reinterpret_cast<char*>(&pkSize), sizeof(packet), MSG_WAITALL);
	if (retval == SOCKET_ERROR) {
		net->Disconnect(id);
		err_display("recv()");
		return false;
	}
	std::cout << "Size : " << (int)pkSize.packetSize << std::endl;
	std::cout << "Type : " << (int)pkSize.packetType << std::endl;

	switch (pkSize.packetType)
	{
	case CS_PACKET_LOGIN:
	{
		cs_packet_login recvPacket;
		retval = recv(sock, reinterpret_cast<char*>((&recvPacket)) + 2, pkSize.packetSize - 2, MSG_WAITALL);


		std::cout << "id : " << (int)id << std::endl;
		sprite = recvPacket.playerSkin;
		isActive = true;
		direction = (char)DIR::N;
		type = PLAYER;
		velocity = PLAYER_SPEED;
		pos.x = (short)800;
		pos.y =(short)900;
		width =  PLAYER_WIDTH;
		height = PLAYER_HEIGHT;

		net->SendLoginOk(id);
		/*
		* �� Ŭ���̾�Ʈ������ ���ο� �÷��̾ ���������� �÷��̾� ������Ʈ�� �����϶����
		*/
		for (int i = 0; i < MAX_USER; ++i) {
			Player* p = reinterpret_cast<Player*>(net->GameObjects[i]);
			if (false == p->isActive) continue;
			if (id == i) continue;
			net->SendPutObj(i, id);
		}

		/*
		* ���� ������ Ŭ���̾�Ʈ���� ���� �׷����� �÷��̾ �˷���
		*/ 
		for (const auto Client : net->GameObjects) {
			if (false == Client->isActive) continue;
			if (id == Client->GetId()) continue;
			net->SendPutObj(id, Client->GetId());
		}

	}
	break;
	case CS_PACKET_PLAYER_MOVE:
	{
		cs_packet_player_move recvPacket;
		retval = recv(sock, reinterpret_cast<char*>(&recvPacket) + 2, pkSize.packetSize - 2, MSG_WAITALL);

		/*
		* �� Ŭ���̾�Ʈ������ �÷��̾ �̵������� �ش� �÷��̾� ������Ʈ�� �̵� �϶����
		*/
		direction = (char)recvPacket.dir;

		isMove = true;

	}
	break;
	case CS_PACKET_PLAYER_STATE:
	{
		cs_packet_player_state recvPacket;
		retval = recv(sock, reinterpret_cast<char*>(&recvPacket) + 2, pkSize.packetSize - 2, MSG_WAITALL);
		switch (recvPacket.playerState)
		{
		case (char)STATE::idle:
		{
			isMove = false;
			isAttack = false;
			state = STATE::idle;
		}
		break;

		case (char)STATE::move:
		{
			isMove = true;
			state = STATE::move;
		}
		break;
		case (char)STATE::attack:
		{
			state = STATE::attack;
			nMagazine++;
		}
		break;
		default:
			break;
		}
		for (int i = 0; i < MAX_USER; ++i)
		{
			Player* player = reinterpret_cast<Player*>(net->GameObjects[i]);
			if (false == player->isActive) continue;
			net->SendChangeState(i, id);
		}
	}
	break;
	case CS_PACKET_SHOOT_BULLET:
	{
		cs_packet_shoot_bullet recvPacket;
		retval = recv(sock, reinterpret_cast<char*>(&recvPacket) + 2, pkSize.packetSize - 2, MSG_WAITALL);
		//if (nMagazine > 5) break;
		//nMagazine++;
		int obj_id = net->GetObjID();
		GameObject* pistol = net->GameObjects[obj_id];
		pistol->direction = recvPacket.dir;
		pistol->velocity = VELOCITY;
		pistol->width = BULLET_WIDTH;
		pistol->height = BULLET_HEIGHT;
		pistol->id = obj_id;
		pistol->sprite = (char)SPRITE::bulletN + direction;
		pistol->type = BULLET;
		pistol->isActive = true;
		pistol->isMove = true;
		pistol->pos = Coordinate{ recvPacket.shootX , recvPacket.shootY };


		/*
		* �� Ŭ���̾�Ʈ������ �÷��̾ ���� �߻� �ش� �÷��̾� ������Ʈ�� Render �϶����
		*/
		for (int i = 0; i < MAX_USER; ++i) {
			Player* player = reinterpret_cast<Player*>(net->GameObjects[i]);
			if (false == player->isActive) continue;
			//if (id == i) continue;
			net->SendPutObj(i, obj_id);
		}

	}
	break;
	case CS_PACKET_USED_ITEM:
	{
		cs_packet_used_item recvPacket;
		retval = recv(sock, reinterpret_cast<char*>(&recvPacket) + 2, pkSize.packetSize - 2, MSG_WAITALL);
		switch (recvPacket.itemNum)
		{
		case (char)ITEM::potion:
		{
			ChangeHp(HEALING);
			/*
			* �� Ŭ���̾�Ʈ������ �÷��̾ ������ ����Ͽ����Ƿ� �ش� �÷��̾��� ü���� Chage �϶����
			*/
			for (int i = 0; i < MAX_USER; ++i) {
				if (false == net->GameObjects[i]->isActive) continue;
				net->SendChangeHp(i, id);
			}

		}
		break;
		case (char)ITEM::box:
		{

		}
		break;
		default:
			break;
		}
	}
	break;
	default:
		std::cout << "�߸��� ��Ŷ ����";
		break;
	}
	return true;
}