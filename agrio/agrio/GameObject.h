#pragma once
#include "stdfx.h"
#include "Game.h"
#include"../../agrio_Server/agrio_Server/Protocol.h"

// ��������Ʈ
CImage sprites[45];

/******************************************** GameObject ********************************************/
struct Coordinate {
	short x;
	short y;
};

class GameObject {
protected:
	Coordinate pos;
	DIR direction;
	int sprite;
	int width, height;
	bool isActive = false;

public:
	GameObject();
	// getter / setter
	Coordinate GetPos() { return pos; }
	DIR GetDir() { return direction; }
	void SetDir(DIR newDir) { direction = newDir; }
	bool GetActive() { return isActive; }

	void LoginOk(void* pk);
	void ObjMove(void* pk);
	void PutObj(void* pk);
	void RemoveObj();
	virtual void Render(HDC& hdc);
};

/******************************************** Player ********************************************/
class Player : public GameObject {
private:
	int curGun;
	STATE state;
	short hp;
public:
	int animFrame;
	int animTimer;
	short items[8];

	Player();

	STATE GetState() { return state; }
	short GetHp() { return hp; }
	int GetcurGun() { return curGun; }
	void SetWeapon(int newWeapon) { curGun = newWeapon; }

	void PlayerState(void* pk);
	void ChangeWeapon(void* pk);
	void ChangeHp(void* pk);
	void GetItem(void* pk);
	void UseItem(int index);
	virtual void Render(HDC& hdc);
};

/******************************************** GameObject ********************************************/
GameObject::GameObject()
{
	pos = { 0, 0 };
	sprite = 0;
	width = 1;
	height = 1;
	direction = DIR::N;
}

void GameObject::LoginOk(void* pk)
{
	sc_packet_login_ok* recvPacket = (sc_packet_login_ok*)pk;

	isActive = true;
	pos.x = recvPacket->x;
	pos.y = recvPacket->y;
	width = recvPacket->width;
	height = recvPacket->height;
	playerID = recvPacket->playerID;
	sprite = selPlayer;
}

void GameObject::ObjMove(void* pk)
{
	sc_packet_move_obj* recvPacket = (sc_packet_move_obj*)pk;

	direction = (DIR)recvPacket->lookDir;
	pos.x = (int)recvPacket->x;
	pos.y = (int)recvPacket->y;
}

void GameObject::PutObj(void* pk)
{
	sc_packet_put_obj* recvPacket = (sc_packet_put_obj*)pk;

	isActive = true;
	sprite = (int)recvPacket->sprite;
	pos.x = (int)recvPacket->x;
	pos.y = (int)recvPacket->y;
	width = (int)recvPacket->width;
	height = (int)recvPacket->height;
}

void GameObject::RemoveObj()
{
	isActive = false;
}

void GameObject::Render(HDC& hdc)
{
	if (isActive && sprites[sprite])
	{
		sprites[sprite].Draw(hdc, pos.x - (width / 2), pos.y - (height / 2), width, height);
	}

}

/******************************************** Player ********************************************/
Player::Player()
{
	curGun = 0;
	state = STATE::idle;
	hp = 50;
	animFrame = 2;
	animTimer = 0;
	for (int i = 0; i < 8; ++i)
		items[i] = 0;
}

void Player::PlayerState(void* pk)
{
	sc_packet_player_state* recvPacket = (sc_packet_player_state*)pk;
	state = static_cast<STATE>(recvPacket->playerState);
}

void Player::ChangeWeapon(void* pk)
{
	sc_packet_change_weapon* recvPacket = (sc_packet_change_weapon*)pk;
	curGun = (int)recvPacket->gunID;
}

void Player::ChangeHp(void* pk)
{
	sc_packet_change_hp* recvPacket = (sc_packet_change_hp*)pk;
	hp = (short)recvPacket->hp;
}

void Player::GetItem(void* pk)
{
	sc_packet_get_item* recvPacket = (sc_packet_get_item*)pk;
	items[recvPacket->itemID] = (short)recvPacket->itemCount;
}

void Player::UseItem(int index)
{
	if (items[index] > 0)
		items[index]--;
}

void Player::Render(HDC& hdc)
{
	if (isActive && sprites[sprite] && sprites[sprite + 1])
	{
		// ���ݸ��
		if (state == STATE::attack)
		{
			sprites[sprite + 1].Draw(hdc, pos.x - ((width + 6) / 2), pos.y - ((height + 2) / 2), width + 12, height + 2,
				char_atk_sprite_rect[(int)direction][animFrame].left, char_atk_sprite_rect[(int)direction][animFrame].top, 30, 32);

			if (curGun != empty && curGun != potion && curGun != box)
				sprites[(int)SPRITE::pistol - pistol + curGun].Draw(hdc, pos.x - ((width + 26) / 2), pos.y - ((height + 22) / 2), width + 32, height + 22,
					char_weapon_sprite_rect[(int)direction][animFrame].left, char_weapon_sprite_rect[(int)direction][animFrame].top, 50, 52);

		}

		// �Ϲݸ��
		else
		{
			sprites[sprite].Draw(hdc, pos.x - (width / 2), pos.y - (height / 2), width, height,
				char_move_sprite_rect[(int)direction][animFrame].left, char_move_sprite_rect[(int)direction][animFrame].top, 18, 30);
		}


		// ü�¹�
		HBRUSH hbrush, oldbrush;
		Rectangle(hdc, pos.x - 20, pos.y - 40, pos.x + 20, pos.y - 33);  //ü�¹�
		hbrush = CreateSolidBrush(RGB(255 - (int((float)hp / (float)max_hp * 255.0)), int((float)hp / (float)max_hp * 255.0), 0));   //ü�� �ۼ�Ʈ���� �� �ٸ���
		oldbrush = (HBRUSH)SelectObject(hdc, hbrush);
		Rectangle(hdc, pos.x - 19, pos.y - 39, int(pos.x - 19 + ((float)hp / (float)max_hp * 38.0)), pos.y - 34);
		SelectObject(hdc, oldbrush);
		DeleteObject(hbrush);
	}
}

std::vector<GameObject*> gameObject;

void SendStatePacket(STATE state);
