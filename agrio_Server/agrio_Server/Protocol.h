#pragma once


//	�ִ� ���� ���� Ŭ���̾�Ʈ
const char MAX_USER = 3;
// �ִ� ������Ʈ ����
const int MAX_OBJECT = 200;

// ĳ������ ����
enum class STATE { idle, move, attack };

//	�Ѿ� �߻� ����
enum class DIR { N, NE, E, SE, S, SW, W, NW };

// ������ ��
enum class SCENE { title, lobby, stage1, gameover, winner };

//	������ ID
enum ITEM { empty, pistol, uzi, shotgun, potion, box };

//	������Ʈ Ÿ��
enum OBJ_TYPE { PLAYER, BOX, BULLET, ITEM, WALL };

//	��Ŷ Ÿ��(Client->Server)
const char CS_PACKET_LOGIN = 1;
const char CS_PACKET_PLAYER_MOVE = 2;
const char CS_PACKET_PLAYER_STATE = 3;
const char CS_PACKET_SHOOT_BULLET = 4;
const char CS_PACKET_USED_ITEM = 5;

//	��Ŷ Ÿ��(Server->Client)
const char SC_PACKET_LOGIN_OK = 1;
const char SC_PACKET_CHANGE_SCENE = 2;
const char SC_PACKET_MOVE_OBJ = 3;
const char SC_PACKET_PLAYER_STATE = 4;
const char SC_PACKET_PUT_OBJ = 5;
const char SC_PACKET_REMOVE_OBJ = 6;
const char SC_PACKET_CHANGE_HP = 7;
const char SC_PACKET_GET_ITEM = 8;
const char SC_PACKET_CHAGE_WEAPON = 10;

//	��Ŷ ����(Client->Server)
#pragma pack(push,1)
struct packet {
	unsigned char packetSize;
	char packetType;
};
struct cs_packet_login : packet {
	char playerSkin;
};
struct cs_packet_player_move : packet {
	char dir;
};
struct cs_packet_player_state : packet {
	char playerState;
};
struct cs_packet_shoot_bullet : packet {
	char playerID;
};
struct cs_packet_used_item : packet {
	char itemNum;
};

//	��Ŷ ����(Server->Client)
struct sc_packet_login_ok : packet {
	char playerID;
	short x, y;
	short width, height;
};
struct sc_packet_change_scene : packet {
	char sceneNum;
};
struct sc_packet_move_obj : packet {
	char objectID;
	char lookDir;
	short x, y;
};
struct sc_packet_player_state : packet {
	char objectID;
	char playerState;
};
struct sc_packet_put_obj : packet {
	char objectID;
	char sprite;
	short x, y;
	short width, height;
};
struct sc_packet_remove_obj : packet {
	char objectID;
};
struct sc_packet_change_hp : packet {
	char playerID;
	unsigned char hp;
};
struct sc_packet_get_item : packet {
	char playerID;
	char itemID;
	char itemCount;
};
struct sc_packet_change_weapon : packet {
	char playerID;
	char gunID; // == itemID
};
#pragma pack(pop)