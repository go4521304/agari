#pragma once
#include "stdfx.h"
#include"../../agrio_Server/agrio_Server/Protocol.h"

/******************************************** ��ǥ********************************************/
// ��������Ʈ
const RECT char_move_sprite_rect[8][4] = //��������Ʈ ��ǥ
{ { { 0,0,18,30 },{ 32,0,50,30 },{ 64,0,82,30 },{ 96,0,114,30 } }/*N*/,{ { 133,0,151,30 },{ 165,0,173,30 },{ 197,0,215,30 },{ 229,0,247,30 } }/*NE*/,
{ { 264,0,282,30 },{ 296,0,314,30 },{ 328,0,346,30 },{ 360,0,388,30 } }/*E*/,{ { 397,0,415,30 },{ 428,0,446,30 },{ 460,0,488,30 },{ 492,0,510,30 } }/*SE*/,
{ { 0,36,18,66 },{ 32,36,50,66 },{ 64,36,82,66 },{ 96,36,114,66 } }/*S*/,{ { 133,36,151,66 },{ 165,36,173,66 },{ 197,36,215,66 },{ 229,36,247,66 } }/*SW*/,
{ { 264,36,282,66 },{ 296,36,314,66 },{ 328,36,346,66 },{ 360,36,388,66 } }/*W*/,{ { 397,36,415,66 },{ 428,36,446,66 },{ 460,36,488,66 },{ 492,36,510,66 } }/*NW*/ };
const RECT char_atk_sprite_rect[8][2] =  //���ݸ�� ��������Ʈ ��ǥ
{ {{0,0,30,32}, {30,0,60,32} }/*N*/, {{ 60,0,90,32 }, {90,0,120,32} }/*NE*/,{{120,0,150,32},{150,0,180,32}}/*E*/,{{180,0,210,32},{210,0,240,32}}/*SE*/,
{ { 0,32,30,64 },{ 30,32,60,64 } }/*S*/,{ { 60,32,90,64 },{ 90,32,120,64 } }/*SW*/,{ { 120,32,150,64 },{ 150,32,180,64 } }/*W*/,{ { 180,32,210,64 },{ 210,32,240,64 }/*NW*/ } };
const RECT char_weapon_sprite_rect[8][2] = //���� ��������Ʈ ��ǥ
{ {{0,0,50,52},{50,0,100,52}}/*N*/, {{100,0,150,52},{150,0,200,52}}/*NE*/ ,{ {200,0,250,52},{250,0,300,52}}/*E*/, {{300,0,350,52},{350,0,400,52}}/*SE*/,
{ { 0,52,50,104 },{ 50,52,100,104 } }/*S*/,{ { 100,52,150,104 },{ 150,52,200,104 } }/*SW*/ ,{ { 200,52,250,104 },{ 250,52,300,104 } }/*W*/,{ { 300,52,350,104 },{ 350,52,400,104 } }/*NW*/ };

// UI ��ǥ
const RECT logo_rect = //�ΰ� ��ġ      (��ŸƮȭ��)
{
	120,120,750,370
};
const RECT play_button_rect = //play��ư ��ġ  (��ŸƮȭ��)
{
	650,480,800,550
};
const RECT exit_button_rect = //exit��ư ��ġ  (��ŸƮȭ��)
{
	650,600,800,670
};
const RECT weapon_image_rect[5] =  //�κ��丮 5�� �̹��� ��ġ
{
	{ 285,730,335,780 },{ 355,730,405,780 },{ 425,730,475,780 },{ 495,730,545,780 }, { 565,730,615,780 }
};
const RECT GAMEOVER_rect = //���ӿ��� ��ư ��ġ  (���ӿ���ȭ��)
{
	220,50,620,150
};
const RECT rankingbox_rect = //��ŷ�ڽ� ��ġ       (���ӿ���ȭ��)
{
	100,200,745,600
};
const RECT replay_button_rect = //���÷��� ��ư ��ġ  (���ӿ���ȭ��)
{
	150,640,300,710
};
const RECT exit2_button_rect = //exit��ư ��ġ       (���ӿ���ȭ��)    //exit��ư�� �� ������°� ���ݾ����� �̹� �ʾ��� ������ exit2
{
	530,640,680,710
};

/******************************************** ���� ********************************************/
// ��������Ʈ
enum class SPRITE
{
	bgTitle, bgLobby, bgStage1, bgEnd, btnPlay, btnExit, btnReplay,
	Izuna, Izuna_Atk, GenAn, GenAn_Atk, Hinagiku, Hinagiku_Atk, Ichika, Ichika_Atk, Kagen, Kagen_Atk, Mitsumoto, Mitsumoto_Atk, Shino, Shino_Atk, Sizune, Sizune_Atk,
	pistol, uzi, shotgun, box,
	uiPistol, uiUzi, uiShotgun, uiPotion, uiBox,
	itemBox,
	bulletN, bulletNE, bulletE, bulletSE, bulletS, bulletSW, bulletW, bulletNW,
	wallCol, wallRow,
	bgWinner, bgGameover 
};

// Win API
MSG Message;
HINSTANCE g_hInst;
LPCTSTR lpszClass = _T("Agrio");

// SOCKET
SOCKET sock;

// ��������
const int character_width = 38;
const int character_height = 60;
const int win_x_size = 900;      //������ x������
const int win_y_size = 800;      //������ y������
bool isLoginOk = false;			// �α���ok�� �ö����� ���
const int max_hp = 100;			// �÷��̾��� �ִ� ü��

bool isAlreadyConnect = false;

// Ű �׼�
struct KEY_ACTION
{
	bool left = false, right = false;
	bool up = false, down = false;
	bool space = false;
	bool reqSend = false;
};
KEY_ACTION keyAction;

unsigned __int64 TIMER;

// �ִϸ��̼� ��� �ӵ�
const int ANIMATION_TIME = 100;

// �Ѿ� �߻� �ӵ�
const int ITEM_TIME[5] = { 500, 100, 500, 500, 500 };

// Ÿ��Ʋ â���� ĳ���� �����ϰ� ���ִ� ������
int selAnimation = 0;
int selTimer = ANIMATION_TIME;
int selPlayer = (int)SPRITE::Izuna;

// �÷��̾�
int playerID = 0;		// �ε��� ��
int selectedWeapon = 0;	// ������ ����
int itemTimer = 0;

// ��
SCENE scene = SCENE::title;

void GameLoop(HWND hWnd);