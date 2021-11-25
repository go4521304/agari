#pragma once

const short PLAYER_WIDTH = 18;
const short PLAYER_HEIGHT = 30;
const short MAX_HP = 100;
const int HEALING = 20;

const float PLAYER_SPEED = 100.f;
enum class SPRITE
{
	bgTitle, bgStage1, bgEnd, btnPlay, btnExit, btnReplay,
	Izuna, Izuna_Atk, GenAn, GenAn_Atk, Hinagiku, Hinagiku_Atk, Ichika, Ichika_Atk, Kagen, Kagen_Atk, Mitsumoto, Mitsumoto_Atk, Shino, Shino_Atk, Sizune, Sizune_Atk,
	pistol, uzi, shotgun, box,
	uiPistol, uiUzi, uiShotgun, uiPotion, uiBox,
	itemBox,
	bulletN, bulletNE, bulletE, bulletSE, bulletS, bulletSW, bulletW, bulletNW
};

struct Coordinate
{
	short x, y;
};
class GameObject
{
public:
	Coordinate pos;
	char direction;
	float velocity;
	unsigned short width, height;
	unsigned char id;
	char sprite;
	char type;
	bool isMove = false;
	bool isAttack = false;

	bool isActive = false;

	unsigned char GetId() const {
		return id;
	}

	void Update(float elapsedTime,char* buf, int& bufPos);
};


class Player : public GameObject
{
public:

	char curEquip;
	STATE state;
	short hp = 50; // ���� Ȯ���� ���� ������ 50���� ����
	short items[8];
	SOCKET sock;

public:
	Player() {};

	~Player() {};

	void SetSockId(SOCKET socket, int clientId) {
		id = clientId;
		sock = socket;
	};
	void ChangeHp(int value) {
		std::cout << "���� ������ �Ҹ��ٴ�" << std::endl;
		if (hp >= 0 && hp < MAX_HP) {
			if (MAX_HP - hp < value) // ������ ����
				hp += MAX_HP - hp;
			else
				hp += value;
		}
	}
	void Send(void* Packet, int packetSize) const;
	bool Recv();

	void SendLogIn();
};