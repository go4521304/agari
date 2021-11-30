#pragma once

const short PLAYER_WIDTH = 18;
const short PLAYER_HEIGHT = 30;
const int MAX_HP = 100;
const int HEALING = 20;
const int ATTACKHP = -10;

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
	int collisionCount = 0;
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
	int hp = 50; // ���� Ȯ���� ���� ������ 50���� ����
	short items[8];
	SOCKET sock;

	int nMagazine = 0;//�ִ� �Ѿ� ����
public:
	Player() {};

	~Player() {};

	void SetSockId(SOCKET socket, int clientId) {
		id = clientId;
		sock = socket;
	};
	void ChangeHp(int value) {
		hp += value;
		hp = std::clamp(hp, 0, MAX_HP);
		sc_packet_change_hp pk;
		pk.packetSize = sizeof(pk);
		pk.packetType = SC_PACKET_CHANGE_HP;
		pk.playerID = id;
		pk.hp = hp;

		Send(&pk, pk.packetSize);
	}
	void Send(void* Packet, int packetSize) const;
	bool Recv();

	void SendLogIn();
};