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
			x += speed*0.7;
			y -= speed*0.7;
			break;

		case (char)DIR::NW:
			x -= speed*0.7;
			y -= speed*0.7;
			break;

		case (char)DIR::S:
			//pos.x += 1;
			y += speed;
			break;

		case (char)DIR::SE:
			x += speed * 0.7;
			y += speed * 0.7;
			break;

		case (char)DIR::SW:
			x -= speed * 0.7;
			y += speed * 0.7;
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
		Network* net = Network::GetInstance();

		if (type == PLAYER && net->MyScene == SCENE::lobby) {
			if ((830 < pk.x && 970 > pk.x) && (650 < pk.y && 780 > pk.y))
				reinterpret_cast<Player*>(this)->isReady = true;
			else
				reinterpret_cast<Player*>(this)->isReady = false;
		}

		for (auto* obj : net->GameObjects) {
			if (false == obj->isActive)continue;
			if (id == obj->id)continue;
			if (net->IsCollision(id, obj->id)) {
				//�浹ó�� ���� ��
				if (type == PLAYER) {

					switch (obj->type)
					{
					case BULLET:
					{
						reinterpret_cast<Player*>(this)->ChangeHp(ATTACKHP);

						GameObject* Object = (net->GameObjects[obj->id]);
						Object->isActive = false;
						Object->isMove = false;
						for (int i = 0; i < MAX_USER; ++i) {
							if (false == Network::GetInstance()->GameObjects[i]->isActive) continue;
							net->SendChangeHp(i, id);
							net->SendRemoveObj(i, obj->id);
							// ü���� 0�� �Ǹ�
							if (reinterpret_cast<Player*>(this)->hp <= 0) {
								net->SendRemoveObj(i, id);
								net->SendChangeScene(id, (char)SCENE::gameover);
							}

						}
					}
					break;
					case ITEM:
					{
						int item = (net->GameObjects[obj->id])->sprite - (int)SPRITE::uiPistol;
						if (item <= shotgun)
							reinterpret_cast<Player*>(net->GameObjects[id])->items[item] += 5;
						else
							reinterpret_cast<Player*>(net->GameObjects[id])->items[item] += 1;
						net->GameObjects[obj->id]->isActive = false;
						for (int i = 0; i < MAX_USER; ++i) {
							if (false == net->GameObjects[i]->isActive) continue;
							net->SendRemoveObj(i, obj->id);
						}
						net->SendGetItem(id, item);
					}
					break;
					case WALL:
					case BOX:
						pos.x = pk.x;
						pos.y = pk.y;
						break;
					default:
						break;
					}

				}
				else {// �÷��̾ �ƴ�(�Ѿ�, ������, ��)������Ʈ�� �浹���� �� �浹Ÿ����(obj->type)�̶��
					switch (type) {
					case BOX:
						break;
					case WALL:
						switch (id)
						{
						case WALL_ID_UP:
							obj->pos.y += 1;
							break;
						case WALL_ID_DOWN:
							obj->pos.y -= 1;
							break;
						case WALL_ID_LEFT:
							obj->pos.x += 1;
							break;
						case WALL_ID_RIGHT:
							obj->pos.x -= 1;
							break;
						default:
							break;
						}
						for (int i = 0; i < MAX_USER; ++i) {
							if (false == net->GameObjects[i]->isActive) continue;
							net->SendMoveObj(i, obj->id);
						}
						break;
					case BULLET:
						if (obj->type == PLAYER && reinterpret_cast<Player*>(net->GameObjects[obj->id])->hp > 0) {
							reinterpret_cast<Player*>(net->GameObjects[obj->id])->ChangeHp(ATTACKHP);

							GameObject* Object = (net->GameObjects[id]);
							Object->isActive = false;
							Object->isMove = false;
							
							for (int i = 0; i < MAX_USER; ++i) {
								if (false == net->GameObjects[i]->isActive) continue;
								net->SendChangeHp(i, obj->id);
								net->SendRemoveObj(i, id);
								// ü���� 0�� �Ǹ�
								if (reinterpret_cast<Player*>(net->GameObjects[obj->id])->hp <= 0) {
									net->SendChangeScene(obj->id, (char)SCENE::gameover);
									net->SendRemoveObj(i, obj->id);
								}
							}
						}
						else if (obj->type == WALL) {

							if (collisionCount > 2) {
								for (int i = 0; i < MAX_USER; ++i) {
									if (false == net->GameObjects[i]->isActive) continue;
									net->SendRemoveObj(i, id);
								}
							}
							else
							{
								collisionCount++;
								switch (direction)
								{
								case (int)DIR::N:
								case (int)DIR::E:
								case (int)DIR::S:
								case (int)DIR::W:
									direction = (direction + 4) % 8;
									break;
								case (int)DIR::NE:
									if(obj->direction == (int)DIR::W)
										direction = (int)DIR::NW;
									else
										direction = (int)DIR::SE;
									break;
								case (int)DIR::NW:
									if (obj->direction == (int)DIR::E)
										direction = (int)DIR::NE;
									else
										direction = (int)DIR::SW;
									break;
								case (int)DIR::SE:
									if (obj->direction == (int)DIR::W)
										direction = (int)DIR::SW;
									else
										direction = (int)DIR::NE;
									break;
								case (int)DIR::SW:
									if (obj->direction == (int)DIR::E)
										direction = (int)DIR::SE;
									else
										direction = (int)DIR::NW;

									break;
								}
							}


						}
						else if (obj->type == BOX) {
							if (collisionCount > 1) {
								for (int i = 0; i < MAX_USER; ++i) {
									if (false == net->GameObjects[i]->isActive) continue;
									net->SendRemoveObj(i, id);
								}
							}
							else
							{
								collisionCount++;
								switch (direction)
								{
								case (int)DIR::N:
								case (int)DIR::E:
								case (int)DIR::S:
								case (int)DIR::W:
									direction = (direction + 4) % 8;
									break;
								case (int)DIR::NE:
									if (obj->direction == (int)DIR::W)
										direction = (int)DIR::NW;
									else
										direction = (int)DIR::SE;
									break;
								case (int)DIR::NW:
									if (obj->direction == (int)DIR::E)
										direction = (int)DIR::NE;
									else
										direction = (int)DIR::SW;
									break;
								case (int)DIR::SE:
									if (obj->direction == (int)DIR::W)
										direction = (int)DIR::SW;
									else
										direction = (int)DIR::NE;
									break;
								case (int)DIR::SW:
									if (obj->direction == (int)DIR::E)
										direction = (int)DIR::SE;
									else
										direction = (int)DIR::NW;

									break;
								}
							}
							if (obj->collisionCount > 1) {
								for (int i = 0; i < MAX_USER; ++i) {
									if (false == net->GameObjects[i]->isActive) continue;
									net->SendRemoveObj(i, obj->id);
								}
							}
							else
							{
 								obj->collisionCount++;

							}
						}
						break;
					}
					
				}
			}
		}
		pk.x = pos.x;
		pk.y = pos.y;
		bufStart += sizeof(pk);
	}
}


void Player::UpdateBuf(void* Packet, int packSize) {
	buf_lock.lock();
	memcpy(eventPacketBuf + bufSize, Packet, packSize);
	bufSize += packSize;
	buf_lock.unlock();
}
void Player::Send(void* buf, int bufSize)
{
	int retval = send(sock, reinterpret_cast<char*>(buf), bufSize, 0);
	if (retval == SOCKET_ERROR) {
		std::cout << "���� �߻�" << (int)id << std::endl;
	}
	//std::cout << "[TCP Server]" << (int)id << " : " << retval << "Byte Send\n";
}

void SetBulletPos(DIR direction, Coordinate& pos, short dist)
{
	switch (direction)
	{
	case DIR::N:
		pos.y -= dist;
		break;
	case DIR::NE:
		pos.x += dist;
		pos.y -= dist;
		break;

	case DIR::NW:
		pos.x -= dist;
		pos.y -= dist;
		break;

	case DIR::S:
		pos.y += dist;
		break;

	case DIR::SE:
		pos.x += dist;
		pos.y += dist;
		break;

	case DIR::SW:
		pos.x -= dist;
		pos.y += dist;
		break;

	case DIR::E:
		pos.x += dist;
		break;

	case DIR::W:
		pos.x -= dist;
		break;

	default:
		break;
	}
}

void SetPistol(int obj_id, char direction, Coordinate pos) {
	if (direction == -1) direction = 7;
	GameObject* pistol = Network::GetInstance()->GameObjects[obj_id];
	pistol->direction = direction;
	pistol->velocity = VELOCITY;
	pistol->width = BULLET_WIDTH;
	pistol->height = BULLET_HEIGHT;
	pistol->id = obj_id;
	pistol->sprite = (char)SPRITE::bulletN + direction;
	pistol->type = BULLET;
	pistol->isActive = true;
	pistol->isMove = true;
	pistol->pos = pos;
	SetBulletPos((DIR)pistol->direction, pistol->pos, 50);
}
void SetBox(int obj_id, char direction, Coordinate pos) {
	if (direction == -1) direction = 7;
	GameObject* pistol = Network::GetInstance()->GameObjects[obj_id];
	pistol->direction = direction;
	pistol->velocity = 0;
	pistol->width = BLOCK_WIDTH;
	pistol->height = BLOCK_HEIGHT;
	pistol->id = obj_id;
	pistol->sprite = (char)SPRITE::box;
	pistol->type = BOX;
	pistol->isActive = true;
	pistol->isMove = false;
	pistol->pos = pos;
	SetBulletPos((DIR)pistol->direction, pistol->pos, 50);
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
	
	switch (pkSize.packetType)
	{
	case CS_PACKET_LOGIN:
	{
			cs_packet_login recvPacket;
			retval = recv(sock, reinterpret_cast<char*>((&recvPacket)) + 2, pkSize.packetSize - 2, MSG_WAITALL);
		if (net->MyScene == SCENE::lobby) {


			std::cout << "id : " << (int)id << std::endl;
			sprite = recvPacket.playerSkin;
			isActive = true;
			direction = (char)DIR::N;
			type = PLAYER;
			velocity = PLAYER_SPEED;
			pos.x = (short)800;
			pos.y = (short)900;
			width = PLAYER_WIDTH;
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
			* ���� ������ Ŭ���̾�Ʈ���� ���� �׷����� ������Ʈ�� �˷���
			*/
			for (const auto obj : net->GameObjects) {
				if (false == obj->isActive) continue;
				if (id == obj->GetId()) continue;
				net->SendPutObj(id, obj->GetId());
			}
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
			//nMagazine++;
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
		if (items[curEquip-1] <= 0) break;
		//nMagazine++;
		items[curEquip - 1]--;

		if (curEquip == shotgun) {
			for (int i = -1; i < 2; ++i) {
				int obj_id = net->GetObjID();
				if (obj_id == -1) {
					
					std::cout << "��� ������Ʈ�� ����Ͽ����ϴ�." << std::endl;
					break;
				}
				SetPistol(obj_id, (net->GameObjects[recvPacket.playerID]->direction+i)%8, net->GameObjects[recvPacket.playerID]->pos);
				for (int i = 0; i < MAX_USER; ++i) {
					Player* player = reinterpret_cast<Player*>(net->GameObjects[i]);
					if (false == player->isActive) continue;
					//if (id == i) continue;
					net->SendPutObj(i, obj_id);
				}
			}
		}
		else {
			int obj_id = net->GetObjID();
			if (obj_id == -1) {
				std::cout << "��� ������Ʈ�� ����Ͽ����ϴ�." << std::endl;
				break;
			}
			SetPistol(obj_id, net->GameObjects[recvPacket.playerID]->direction, net->GameObjects[recvPacket.playerID]->pos);
			for (int i = 0; i < MAX_USER; ++i) {
				Player* player = reinterpret_cast<Player*>(net->GameObjects[i]);
				if (false == player->isActive) continue;
				//if (id == i) continue;
				net->SendPutObj(i, obj_id);
			}
		}
		
		/*
		* �� Ŭ���̾�Ʈ������ �÷��̾ ���� �߻� �ش� �÷��̾� ������Ʈ�� Render �϶����
		*/
		

	}
	break;
	case CS_PACKET_USED_ITEM:
	{
		cs_packet_used_item recvPacket;
		retval = recv(sock, reinterpret_cast<char*>(&recvPacket) + 2, pkSize.packetSize - 2, MSG_WAITALL);
		switch (recvPacket.itemNum)
		{
		case (char)ITEM::pistol:
		case (char)ITEM::uzi:
		case (char)ITEM::shotgun:
			curEquip = (char)recvPacket.itemNum;
			for (int i = 0; i < MAX_USER; ++i) {
				if (false == net->GameObjects[i]->isActive) continue;
				net->SendChangeWeapon(i, id);
			}
			break;

		case (char)ITEM::potion:
		{
			if (items[potion-1] <= 0) break;
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
			if (items[box -1 ] <= 0) break;
			int obj_id = net->GetObjID();
			if (obj_id == -1) {
				std::cout << "��� ������Ʈ�� ����Ͽ����ϴ�." << std::endl;
				break;
			}
			SetBox(obj_id, (net->GameObjects[id]->direction), net->GameObjects[id]->pos);
			for (int i = 0; i < MAX_USER; ++i) {
				Player* player = reinterpret_cast<Player*>(net->GameObjects[i]);
				if (false == player->isActive) continue;
				//if (id == i) continue;
				net->SendPutObj(i, obj_id);
			}
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