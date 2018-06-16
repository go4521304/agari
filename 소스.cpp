#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <random>
#include <time.h>
#include <math.h>
#include <atlimage.h>


HINSTANCE g_hInst;
LPCTSTR lpszClass = _T("agari!");

#define character_width 38
#define character_height 60


enum direction  //���� N=0,NE=1,E=2,SE=3,S=4,SW=5,W=6,NW=7
{
	N = 0, NE = 1, E = 2, SE = 3, S = 4, SW = 5, W = 6, NW = 7
};
enum page       //������
{
	start_page, game_page, end_page
};
enum weapom     //�����ȣ pistol = 0, uzi = 1, shotgun = 2, barrel = 3, wall = 4, rocket = 5
{
	pistol = 0, uzi = 1, shotgun = 2, barrel = 3, wall = 4, rocket = 5
};
enum timer      //Ÿ�̸� �ѹ��� ���� ��� �̰� ���ݽô� 
{
	rest_time,
	move_player, atk_player,
	spawn_monster, move_monster,
	spawn_itembox,
};

typedef struct CHARACTER   //�÷��̾�, ����, �������� ����
{
	int health;            //���� ĳ���� ü��  
	int max_health;        //ĳ���� �ִ� ü��
	int x;                 //���� ĳ���� x��ǥ 
	int y;                 //���� ĳ���� y��ǥ 
	int direction;         //���� ĳ���Ͱ� �ٶ󺸴� ���� enum direction���� ���

	int sprite_num;
	bool ismoving;         //�̵������� (Ÿ�̸� ���ư����ִ���)
	bool isattacking;      //���������� (Ÿ�̸� ���ư����ִ���)

	CHARACTER* next;       //����, �������Ϳ� next��ǥ
}CHARACTER;


typedef struct WEAPON       //���� ����
{
	bool open;              //���� ���� ����
	int bullet;             //���⺰ ���� ź��
	int max_bullet;         //���⺰ �ִ� ź��, ������ �ڽ��� Ȯ�������� �ִ�ź�� Ȯ�� ����
	int damage;             //���⺰ ������        
	int range;              //���⺰ �����Ÿ�
	int delay;              //���� ��� ������ 
}WEAPON;





typedef struct BLOCK         //50*50ũ�� ���
{
	bool iswall;              //���� �ִ�?
	bool isinvinciblewall;    //�ı� �Ұ����� ���� �ִ�?
	bool isbarrel;            //������ �ִ�?
	bool isitembox;           //�����۹ڽ��� �ִ�?

	bool isempty;             //����ִ�? == ���� 4 ������Ʈ�� �ƹ��͵� ����߸� true!

	RECT rect;                //��� ��ǥ 
}BLOCK;

/************************************************�Լ� ����********************************************************/
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CALLBACK TimerProc(HWND hWnd, UINT uMSG, UINT idEvent, DWORD dwTime);
void Game_start_setting(HWND hWnd);  //���� ���� ����(���� �ʱ�ȭ ���)
bool Aquire_itembox();      //������ �ڽ��� �Ծ�η���!
bool Spawn_itembox();
void Char_Deathcheck(HWND hWnd);
void Stage_start(HWND hWnd);
void Reset_weapon_setting();
void Reset_weapon_upgrade();
void Spawn_monster();
void Spawn_boss();
bool Remaining_bullet_check();  //���� �Ѿ� Ȯ��  �������� false ������ true
bool Crash_check_character2object(int speed);
/*********************************************void()�Լ� �ְ�*****************************************************/

/*********************************************����մϴ� ��������*****************************************************/

int win_x_size = 900;      //������ x������
int win_y_size = 800;      //������ y������
int block_size = 50;       //��� ũ��

BLOCK block[32][36];       //36*32 ��� 

int mx;                    //���콺 x��ġ��	
int my;                    //���콺 y��ġ��

int current_page;          //���� ������  enum page�� ���
int stage;                 //���� ��������
int score;                 //���� ���ھ�


CHARACTER player;          //�÷��̾�
WEAPON weapon[6];          //���� 6�� enum weapon���� ���
int selected_weapon;       //���� �������� ���� enum weapon���� ���

/******************���� ���׷��̵� bool����*******************/
//�����ȣ pistol = 0, uzi = 1, shotgun = 2, barrel = 3, wall = 4, rocket = 5
bool pistol_range_up1;
bool pistol_delay_down1;
//pistol �Ѿ� ����

bool uzi_range_up1;
bool uzi_delay_down1;
bool uzi_maxammo_up1;
bool uzi_maxammo_up2;

bool shotgun_range_up1;
bool shotgun_delay_down1;
bool shotgun_maxammo_up1;
bool shotgun_maxammo_up2;

bool barrel_maxammo_up1;
bool barrel_maxammo_up2;

bool wall_maxammo_up1;
bool wall_maxammo_up2;

//rocket �����Ÿ� ����
bool rocket_delay_down1;
bool rocket_maxammo_up1;
bool rocket_maxammo_up2;
/*************************************************************/

int itembox_num;           //�ʿ� �����ϴ� ������ �ڽ� ����
int aquired_itembox_num;   //������� ������ ������ �ڽ� ����

bool isrest_time;


CHARACTER* monster_head;  //head���� �������� �ʰ� ����ϱ�
CHARACTER* boss_head;     //head���� �������� �ʰ� ����ϱ�

int monster_num;          //���� ���� ����(player.isattacking == true)
int boss_num;             //���� �������� ����


RECT char_move_sprite_rect[8][4] = //��������Ʈ ��ǥ
{ { { 0,0,18,30 },{ 32,0,50,30 },{ 64,0,82,30 },{ 96,0,114,30 } }/*N*/,{ { 133,0,151,30 },{ 165,0,173,30 },{ 197,0,215,30 },{ 229,0,247,30 } }/*NE*/,
{ { 264,0,282,30 },{ 296,0,314,30 },{ 328,0,346,30 },{ 360,0,388,30 } }/*E*/,{ { 397,0,415,30 },{ 428,0,446,30 },{ 460,0,488,30 },{ 492,0,510,30 } }/*SE*/,
{ { 0,36,18,66 },{ 32,36,50,66 },{ 64,36,82,66 },{ 96,36,114,66 } }/*S*/,{ { 133,36,151,66 },{ 165,36,173,66 },{ 197,36,215,66 },{ 229,36,247,66 } }/*SW*/,
{ { 264,36,282,66 },{ 296,36,314,66 },{ 328,36,346,66 },{ 360,36,388,66 } }/*W*/,{ { 397,36,415,66 },{ 428,36,446,66 },{ 460,36,488,66 },{ 492,36,510,66 } }/*NW*/ };

RECT char_atk_sprite_rect[8][2] =  //���ݸ�� ��������Ʈ ��ǥ
{ {{0,0,30,32}, {30,0,60,32} }/*N*/, {{ 60,0,90,32 }, {90,0,120,32} }/*NE*/,{{120,0,150,32},{150,0,180,32}}/*E*/,{{180,0,210,32},{210,0,240,32}}/*SE*/,
{ { 0,32,30,64 },{ 30,32,60,64 } }/*S*/,{ { 60,32,90,64 },{ 90,32,120,64 } }/*SW*/,{ { 120,32,150,64 },{ 150,32,180,64 } }/*W*/,{ { 180,32,210,64 },{ 210,32,240,64 }/*NW*/ } };

RECT monster_atk_sprite_rect[8][2] =  //���ݸ�� ��������Ʈ ��ǥ
{ { { 0,0,34,37 },{ 34,0,68,37 } }/*N*/,{ { 68,0,102,37 },{ 102,0,136,37 } }/*NE*/,{ { 136,0,170,37 },{ 170,0,204,37 } }/*E*/,{ { 204,0,238,37 },{ 238,0,272,37 } }/*SE*/,
 { { 0,37,34,74 },{ 34,37,68,74 } }/*S*/,{ { 68,37,102,74 },{ 102,37,136,74 } }/*SW*/,{ { 136,37,170,74 },{ 170,37,204,74 } }/*W*/,{ { 204,37,238,74 },{ 238,37,272,74 } }/*NW*/ };

RECT logo_rect = { 120,120,750,370 };          //�ΰ� ��ġ      (��ŸƮȭ��)
RECT play_button_rect = { 650,480,800,550 };   //play��ư ��ġ  (��ŸƮȭ��)
RECT exit_button_rect = { 650,600,800,670 };   //exit��ư ��ġ  (��ŸƮȭ��)

RECT weapon_image_rect[6] = { { 220,680,270,730 },{ 300,680,350,730 },{ 380,680,430,730 },{ 460,680,510,730 },{ 540,680,590,730 },{ 620,680,670,730 } };  //���� 6�� �̹��� ��ġ

RECT GAMEOVER_rect = { 220,50,620,150 };       //���ӿ��� ��ư ��ġ  (���ӿ���ȭ��)
RECT rankingbox_rect = { 100,200,745,600 };    //��ŷ�ڽ� ��ġ       (���ӿ���ȭ��)
RECT replay_button_rect = { 150,640,300,710 }; //���÷��� ��ư ��ġ  (���ӿ���ȭ��)
RECT exit2_button_rect = { 530,640,680,710 };  //exit��ư ��ġ       (���ӿ���ȭ��)    //exit��ư�� �� ������°� ���ݾ����� �̹� �ʾ��� ������ exit2

/*********************************************����մϴ� ���尡��������������*****************************************/


/*********************************************��Ʈ�� �̹���*****************************************************/


CImage start_page_bk_img;   //����ȭ�� ���
CImage game_page_bk_img;    //����ȭ�� ���
CImage end_page_bk_img;     //����ȭ�� ���

CImage logo_img;            //�ΰ�
CImage play_button_img;     //play��ư
CImage exit_button_img;     //exit��ư

CImage itembox_img;
CImage wall_img;
CImage barrel_img;
CImage rocket_bullet_img;

int current_char_num;       //ĳ���� ���ÿ�... ����� ������

//////
CImage charac_sprite;
CImage charac_atk_sprite;

CImage monster_sprite;
CImage monster_atk_sprite;

CImage boss_sprite;
CImage boss_atk_sprite;
//////

CImage monster_move_sprite[8][4]; //���� �̵� ��������Ʈ [����][�̵� ��� 4��(?)]


								  /*********************************************��Ʈ�� �̹���*****************************************************/
MSG Message;
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;

	WNDCLASSEX WndClass;
	g_hInst = hInstance;

	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&WndClass);

	hWnd = CreateWindow(lpszClass, _T("agari!"), WS_OVERLAPPEDWINDOW, 0, 0, win_x_size + 14, win_y_size + 36, NULL, (HMENU)NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	srand(GetTickCount());  //��ƽī��Ʈ! �����õ�~

	while (GetMessage(&Message, 0, 0, 0))
	{
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}

	return Message.wParam;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

	HDC hdc, memdc1, memdc2;
	PAINTSTRUCT ps;

	HBRUSH hbrush, oldbrush;

	CImage img;
	CImage dc, dc2;

	TCHAR str[500] = {};


	static bool play_button = false;  //����ȭ�� start��ư
	static bool exit_button = false;  //����ȭ�� exit��ư

	static bool replay_button = false;  //����ȭ�� replay��ư
	static bool exit2_button = false;  //����ȭ�� exit��ư

	switch (uMsg) {
	case WM_CREATE:
		/*********************************************�̹��� �ε�*****************************************************/

		play_button_img.Load(TEXT("..\\agari\\resource\\PLAY.png"));
		exit_button_img.Load(TEXT("..\\agari\\resource\\EXIT.png"));
		start_page_bk_img.Load(TEXT("..\\agari\\resource\\startBack.png"));

		charac_sprite.Load(TEXT("..\\agari\\resource\\Izuna_move.png"));
		charac_atk_sprite.Load(TEXT("..\\agari\\resource\\Izuna_attack.png"));

		monster_sprite.Load(TEXT("..\\agari\\resource\\Monster_move.png"));
		monster_atk_sprite.Load(TEXT("..\\agari\\resource\\Monster_attack.png"));

		itembox_img.Load(TEXT("..\\agari\\resource\\cube.png"));
		wall_img.Load(TEXT("..\\agari\\resource\\wall.png"));
		barrel_img.Load(TEXT("..\\agari\\resource\\barrel.png"));
		rocket_bullet_img.Load(TEXT("..\\agari\\resource\\Rocket.png"));

		/*********************************************�̹��� �ε�*****************************************************/

		current_page = start_page;  //���� �������� ������������

		for (int i = 0; i < 32; i++)
		{
			for (int j = 0; j < 36; j++)
			{
				block[i][j].rect = { 50 * j,50 * i,50 * j + 50,50 * i + 50 };  //36*32 ��� ��ǥ �Է�
			}
		}

		monster_head = (CHARACTER*)malloc(sizeof(CHARACTER));
		monster_head->next = NULL;
		boss_head = (CHARACTER*)malloc(sizeof(CHARACTER));
		boss_head->next = NULL;

		break;
	case WM_PAINT:

		hdc = BeginPaint(hWnd, &ps);

		if (current_page == start_page)
		{
			dc.Create(win_x_size, win_y_size, 24);	// == CreateCompatibleBitmap
			memdc1 = dc.GetDC();					// == CreateComaptibleDC

			start_page_bk_img.Draw(memdc1, 0, 0, win_x_size, win_y_size);
			play_button_img.Draw(memdc1, play_button_rect);
			exit_button_img.Draw(memdc1, exit_button_rect);

			dc.Draw(hdc, 0, 0, win_x_size, win_y_size);	// �Ʒ��� Bitblt�� ����
			dc.ReleaseDC();		// dc ����
			dc.Destroy();		// ��� dc ����

								// �Ʒ��� ���� �۾��� (���صǸ� ������)
								//BitBlt(hdc, 0, 0, win_x_size, win_y_size, memdc1, 0, 0, SRCCOPY);
								//DeleteObject(membitmap1);
								//DeleteObject(memdc1);
		}
		else if (current_page == game_page)
		{
			dc.Create(win_x_size * 2, win_y_size * 2, 24);	// == CreateCompatibleBitmap
			memdc1 = dc.GetDC();					// == CreateComaptibleDC

			//////////////////////////////////////////��� ���///////////////////////////////////////////////
			hbrush = CreateSolidBrush(RGB(200, 255, 255));
			oldbrush = (HBRUSH)SelectObject(memdc1, hbrush);
			Rectangle(memdc1, 0, 0, win_x_size * 2, win_y_size * 2);
			SelectObject(memdc1, oldbrush);
			DeleteObject(hbrush);

			for (int i = 0; i < 33; i++)  //��� ���м� ���
			{
				MoveToEx(memdc1, 0, i * 50, NULL);
				LineTo(memdc1, win_x_size * 2, i * 50);
			}
			for (int i = 0; i < 37; i++)
			{
				MoveToEx(memdc1, i * 50, 0, NULL);
				LineTo(memdc1, i * 50, win_y_size * 2);
			}

			//////////////////////////////////////////���� ������Ʈ ���///////////////////////////////////////////////
			for (int i = 0; i < 32; i++)
			{
				for (int j = 0; j < 36; j++)
				{
					if (block[i][j].isitembox)
					{
						itembox_img.Draw(memdc1, block[i][j].rect.left + 10, block[i][j].rect.top + 4, 30, 41, 0, 0, 30, 41);
					}
					else if ((block[i][j].iswall) || (block[i][j].isinvinciblewall))
					{
						wall_img.Draw(memdc1, block[i][j].rect.left, block[i][j].rect.top, 50, 50, 0, 0, 22, 30);
					}
					else if (block[i][j].isbarrel)
					{
						barrel_img.Draw(memdc1, block[i][j].rect.left, block[i][j].rect.top, 50, 50, 0, 0, 544, 720);
					}
				}

			}

			//////////////////////////////////////////ĳ���� ���///////////////////////////////////////////////
			if (player.isattacking == false)  //������ �ƴҶ�
			{
				charac_sprite.Draw(memdc1, player.x - character_width / 2, player.y - character_height / 2, character_width, character_height,
					char_move_sprite_rect[player.direction][player.sprite_num].left, char_move_sprite_rect[player.direction][player.sprite_num].top, 18, 30);
			}
			else                              //�������϶�
			{
				charac_atk_sprite.Draw(memdc1, player.x - 24, player.y - 32, 64, 64,
					char_atk_sprite_rect[player.direction][player.sprite_num].left, char_atk_sprite_rect[player.direction][player.sprite_num].top, 30, 32);

			}

			//////////////////////////////////////////ü�¹� ���///////////////////////////////////////////////
			Rectangle(memdc1, player.x - 20, player.y - 40, player.x + 20, player.y - 33);  //ü�¹�
			hbrush = CreateSolidBrush(RGB(255 - (int((float)player.health / (float)player.max_health*255.0)), int((float)player.health / (float)player.max_health*255.0), 0));   //ü�� �ۼ�Ʈ���� �� �ٸ���
			oldbrush = (HBRUSH)SelectObject(memdc1, hbrush);
			Rectangle(memdc1, player.x - 19, player.y - 39, int(player.x - 19 + ((float)player.health / (float)player.max_health*38.0)), player.y - 34);
			SelectObject(memdc1, oldbrush);
			DeleteObject(hbrush);

			//////////////////////////////////////////���� ���///////////////////////////////////////////////
			CHARACTER* p = monster_head->next;

			while (p != NULL)
			{
				if (p->isattacking == false) //������ �ƴҶ�
				{
					monster_sprite.Draw(memdc1, p->x - character_width / 2, p->y - character_height / 2, character_width, character_height,
						char_move_sprite_rect[p->direction][p->sprite_num].left, char_move_sprite_rect[p->direction][p->sprite_num].top, 18, 30);
				}
				else                         //�������϶�
				{
					monster_atk_sprite.Draw(memdc1, p->x - 24, p->y - 32, 64, 64,
						monster_atk_sprite_rect[p->direction][p->sprite_num].left, monster_atk_sprite_rect[p->direction][p->sprite_num].top, 34, 37);
				}
				p = p->next;
			}

			//////////////////////////////////////////�������� ���///////////////////////////////////////////////


			int temp_atk;
			if (player.isattacking == true)
				temp_atk = 1;
			else
				temp_atk = 0;

			int temp_mv;
			if (player.ismoving == true)
				temp_mv = 1;
			else
				temp_mv = 0;
			wsprintf(str, TEXT("stage : %d  score : %d  direction : %d  x : %d  y: %d  weapon : %d  bullet : %d  itembox_num : %d  ak? %d mv? %d"), stage, score, player.direction, player.x, player.y, selected_weapon, weapon[selected_weapon].bullet, itembox_num, temp_atk, temp_mv);
			TextOut(memdc1, player.x - (win_x_size / 2), player.y - (win_y_size / 2), str, _tcslen(str));





			//������Ʈ ���(��, ����)

			//�Ѿ� ���


			//////////////////////////////////////�ϴ� �� ���� ���//////////////////////////////////////////
			hbrush = CreateSolidBrush(RGB(0, 0, 0));
			oldbrush = (HBRUSH)SelectObject(memdc1, hbrush);

			if ((win_x_size / 2 <= player.x) && (player.x <= win_x_size / 2 * 3) && (win_y_size / 2 <= player.y) && (player.y <= win_y_size / 2 * 3))
			{
				for (int i = 0; i < 6; i++)
					RoundRect(memdc1, player.x - (win_x_size / 2) + weapon_image_rect[i].left, player.y - (win_y_size / 2) + weapon_image_rect[i].top,
						player.x - (win_x_size / 2) + weapon_image_rect[i].right, player.y - (win_y_size / 2) + weapon_image_rect[i].bottom, 10, 10);
				dc.Draw(hdc, 0, 0, win_x_size, win_y_size, player.x - (win_x_size / 2), player.y - (win_y_size / 2), win_x_size, win_y_size);
			}

			else if ((player.x < win_x_size / 2) && (player.y < win_y_size / 2))
			{
				for (int i = 0; i < 6; i++)
					RoundRect(memdc1, weapon_image_rect[i].left, weapon_image_rect[i].top, weapon_image_rect[i].right, weapon_image_rect[i].bottom, 10, 10);
				dc.Draw(hdc, 0, 0, win_x_size, win_y_size, 0, 0, win_x_size, win_y_size);
			}
			else if ((win_x_size / 2 * 3 < player.x) && (player.y < win_y_size / 2))
			{
				for (int i = 0; i < 6; i++)
					RoundRect(memdc1, weapon_image_rect[i].left + win_x_size, weapon_image_rect[i].top, weapon_image_rect[i].right + win_x_size, weapon_image_rect[i].bottom, 10, 10);
				dc.Draw(hdc, 0, 0, win_x_size, win_y_size, win_x_size, 0, win_x_size, win_y_size);
			}
			else if ((win_x_size / 2 * 3 < player.x) && (win_y_size / 2 * 3 < player.y))
			{
				for (int i = 0; i < 6; i++)
					RoundRect(memdc1, weapon_image_rect[i].left + win_x_size, weapon_image_rect[i].top + win_y_size, weapon_image_rect[i].right + win_x_size,
						weapon_image_rect[i].bottom + win_y_size, 10, 10);
				dc.Draw(hdc, 0, 0, win_x_size, win_y_size, win_x_size, win_y_size, win_x_size, win_y_size);
			}
			else if ((player.x < win_x_size / 2) && (win_y_size / 2 * 3 < player.y))
			{
				for (int i = 0; i < 6; i++)
					RoundRect(memdc1, weapon_image_rect[i].left, weapon_image_rect[i].top + win_y_size, weapon_image_rect[i].right, weapon_image_rect[i].bottom + win_y_size, 10, 10);
				dc.Draw(hdc, 0, 0, win_x_size, win_y_size, 0, win_y_size, win_x_size, win_y_size);
			}
			else if (player.x < win_x_size / 2)
			{
				for (int i = 0; i < 6; i++)
					RoundRect(memdc1, weapon_image_rect[i].left, weapon_image_rect[i].top + player.y - (win_y_size / 2),
						weapon_image_rect[i].right, weapon_image_rect[i].bottom + player.y - (win_y_size / 2), 10, 10);
				dc.Draw(hdc, 0, 0, win_x_size, win_y_size, 0, player.y - (win_y_size / 2), win_x_size, win_y_size);
			}
			else if (player.y < win_y_size / 2)
			{
				for (int i = 0; i < 6; i++)
					RoundRect(memdc1, weapon_image_rect[i].left + player.x - (win_x_size / 2), weapon_image_rect[i].top,
						weapon_image_rect[i].right + player.x - (win_x_size / 2), weapon_image_rect[i].bottom, 10, 10);
				dc.Draw(hdc, 0, 0, win_x_size, win_y_size, player.x - (win_x_size / 2), 0, win_x_size, win_y_size);
			}
			else if (win_x_size / 2 * 3 < player.x)
			{
				for (int i = 0; i < 6; i++)
					RoundRect(memdc1, weapon_image_rect[i].left + win_x_size, weapon_image_rect[i].top + player.y - (win_y_size / 2),
						weapon_image_rect[i].right + win_x_size, weapon_image_rect[i].bottom + player.y - (win_y_size / 2), 10, 10);
				dc.Draw(hdc, 0, 0, win_x_size, win_y_size, win_x_size, player.y - (win_y_size / 2), win_x_size, win_y_size);
			}
			else if (win_y_size / 2 * 3 < player.y)
			{
				for (int i = 0; i < 6; i++)
					RoundRect(memdc1, weapon_image_rect[i].left + player.x - (win_x_size / 2), weapon_image_rect[i].top + win_y_size,
						weapon_image_rect[i].right + player.x - (win_x_size / 2), weapon_image_rect[i].bottom + win_y_size, 10, 10);
				dc.Draw(hdc, 0, 0, win_x_size, win_y_size, player.x - (win_x_size / 2), win_y_size, win_x_size, win_y_size);
			}

			SelectObject(memdc1, oldbrush);
			DeleteObject(hbrush);

			dc.ReleaseDC();		// dc ����
			dc.Destroy();		// ��� dc ����

		}
		else if (current_page == end_page)
		{
			//���ھ�

			//��������

			//replay��ư

			//exit��ư
		}

		EndPaint(hWnd, &ps);
		break;

	case WM_LBUTTONDOWN:
	{
		mx = LOWORD(lParam);
		my = HIWORD(lParam);

		if (current_page == start_page)
		{
			if ((play_button_rect.left <= mx) && (play_button_rect.top <= my) && (mx <= play_button_rect.right) && (my <= play_button_rect.bottom))  //play��ư ����, ������ ���� �۵�
			{
				play_button = true;
				{
					/***********************************

					��ư Ŭ���� ����Ʈ ������ ������ �� ������ �ϴ�..��

					************************************/
				}
			}
			else if ((exit_button_rect.left <= mx) && (exit_button_rect.top <= my) && (mx <= exit_button_rect.right) && (my <= exit_button_rect.bottom)) //exit��ư ����, ������ ���� �۵�
			{
				exit_button = true;
				{
					/***********************************

					��ư Ŭ���� ����Ʈ ������ ������ �� ������ �ϴ�..��

					************************************/
				}
			}
		}
		else if (current_page == game_page)
		{



		}
		else if (current_page == end_page)
		{
			if ((replay_button_rect.left <= mx) && (replay_button_rect.top <= my) && (mx <= replay_button_rect.right) && (my <= replay_button_rect.bottom))  //replay��ư ����, ������ ���� �۵�
			{
				replay_button = true;
				{
					/***********************************

					��ư Ŭ���� ����Ʈ ������ ������ �� ������ �ϴ�..��

					************************************/
				}
			}
			else if ((exit2_button_rect.left <= mx) && (exit2_button_rect.top <= my) && (mx <= exit2_button_rect.right) && (my <= exit2_button_rect.bottom)) //exit2��ư ����, ������ ���� �۵�
			{
				exit2_button = true;
				{
					/***********************************

					��ư Ŭ���� ����Ʈ ������ ������ �� ������ �ϴ�..��

					************************************/
				}
			}
		}
		break;
	}
	case WM_LBUTTONUP:
	{
		if (current_page == start_page)
		{
			if (play_button == true)
			{
				current_page = game_page;   //����ȭ������ �̵�
				play_button = false;

				SetTimer(hWnd, spawn_itembox, 15000, TimerProc);//15�ʸ��� �����۹ڽ� ����
				Game_start_setting(hWnd);                       //���� ���� ����(���� �ʱ�ȭ ���)

				InvalidateRect(hWnd, NULL, false);
			}
			else if (exit_button == true)
			{
				exit_button = false;
				PostQuitMessage(0);          //���α׷� ����
				break;
			}
		}
		else if (current_page == game_page)
		{



		}
		else if (current_page == end_page)
		{
			if (replay_button == true)
			{
				current_page = game_page;   //����ȭ������ �̵�
				play_button = false;

				SetTimer(hWnd, spawn_itembox, 15000, TimerProc);//15�ʸ��� �����۹ڽ� ����
				Game_start_setting(hWnd);                       //���� ���� ����(���� �ʱ�ȭ ���)


				InvalidateRect(hWnd, NULL, false);
			}
			else if (exit2_button == true)
			{
				exit_button = false;
				PostQuitMessage(0);          //���α׷� ����
				break;
			}
		}
		break;
	}
	case WM_MOUSEMOVE:

		mx = LOWORD(lParam);
		my = HIWORD(lParam);

		break;
	case WM_KEYDOWN:
	{

		if (current_page == game_page)
		{

			switch (wParam)
			{
			case 'w':
			case 'W':
			{
				if ((player.direction == N) || (player.direction == S) || (player.direction == SE) || (player.direction == SW))  //�̵� ������ ������
					player.direction = N;
				else if (player.direction == W)  //�̵� ������ �ϼ���
				{
					if (player.ismoving == true)
						player.direction = NW;
					else
						player.direction = N;
				}
				else if (player.direction == E)  //�̵� ������ �ϵ�����
				{
					if (player.ismoving == true)
						player.direction = NE;
					else
						player.direction = N;
				}

				if (player.ismoving == false)    //�����־��ٸ�
					player.ismoving = true;

				InvalidateRect(hWnd, NULL, false);
				break;
			}
			case 'a':
			case 'A':
			{
				if ((player.direction == E) || (player.direction == W) || (player.direction == NE) || (player.direction == SE))  //�̵� ������ ����
					player.direction = W;
				else if (player.direction == N)  //�̵� ������ �ϼ���
				{
					if (player.ismoving == true)
						player.direction = NW;
					else
						player.direction = W;
				}
				else if (player.direction == S)  //�̵� ������ ������
				{
					if (player.ismoving == true)
						player.direction = SW;
					else
						player.direction = W;
				}

				if (player.ismoving == false)    //�����־��ٸ�
					player.ismoving = true;

				InvalidateRect(hWnd, NULL, false);
				break;
			}
			case 's':
			case 'S':
			{
				if ((player.direction == N) || (player.direction == S) || (player.direction == NE) || (player.direction == NW))  //�̵� ������ ������
					player.direction = S;
				else if (player.direction == W)  //�̵� ������ ������
				{
					if (player.ismoving == true)
						player.direction = SW;
					else
						player.direction = S;
				}
				else if (player.direction == E)  //�̵� ������ ��������
				{
					if (player.ismoving == true)
						player.direction = SE;
					else
						player.direction = S;
				}

				if (player.ismoving == false)    //�����־��ٸ�
					player.ismoving = true;

				InvalidateRect(hWnd, NULL, false);
				break;
			}
			case 'd':
			case 'D':
			{
				if ((player.direction == E) || (player.direction == W) || (player.direction == NW) || (player.direction == SW))  //�̵� ������ ������
					player.direction = E;
				else if (player.direction == N)  //�̵� ������ �ϵ�����
				{
					if (player.ismoving == true)
						player.direction = NE;
					else
						player.direction = E;
				}
				else if (player.direction == S)  //�̵� ������ ��������
				{
					if (player.ismoving == true)
						player.direction = SE;
					else
						player.direction = E;
				}

				if (player.ismoving == false)    //�����־��ٸ�
					player.ismoving = true;

				InvalidateRect(hWnd, NULL, false);
				break;
			}

			case 'q':  //���� ���� ����
			case 'Q':
			{
				if (player.isattacking == true)  //���� ����߿��� ���� ���� �Ұ�
					break;

				if (selected_weapon == pistol)                    //pistol�� ���� ó�� ����
					break;
				else if (weapon[selected_weapon - 1].open == true)     //���� ���⸦ �������ִٸ�
				{
					selected_weapon--;
					InvalidateRect(hWnd, NULL, false);
				}

				break;
			}
			case 'e':  //���� ���� ����
			case 'E':
			{
				if (player.isattacking == true)  //���� ����߿��� ���� ���� �Ұ�
					break;

				if (selected_weapon == rocket)                     //rocket�� ���� ������ ����
					break;
				else if (weapon[selected_weapon + 1].open == true) //���� ���⸦ �������ִٸ�
				{
					selected_weapon++;
					InvalidateRect(hWnd, NULL, false);
				}

				break;
			}
			case '1':  //pistol ����
			{

				if (player.isattacking == true)  //���� ����߿��� ���� ���� �Ұ�
					break;

				if (weapon[pistol].open == true)
				{
					selected_weapon = pistol;
					InvalidateRect(hWnd, NULL, false);
				}

				break;
			}
			case '2':  //uzi ����
			{
				if (player.isattacking == true)  //���� ����߿��� ���� ���� �Ұ�
					break;

				if (weapon[uzi].open == true)
				{
					selected_weapon = uzi;
					InvalidateRect(hWnd, NULL, false);
				}
				break;
			}
			case '3':  //shotgun ����
			{
				if (player.isattacking == true)  //���� ����߿��� ���� ���� �Ұ�
					break;

				if (weapon[shotgun].open == true)
				{
					selected_weapon = shotgun;
					InvalidateRect(hWnd, NULL, false);
				}
				break;
			}
			case '4':  //barrel ����
			{
				if (player.isattacking == true)  //���� ����߿��� ���� ���� �Ұ�
					break;

				if (weapon[barrel].open == true)
				{
					selected_weapon = barrel;
					InvalidateRect(hWnd, NULL, false);
				}
				break;
			}
			case '5':  //wall ����
			{
				if (player.isattacking == true)  //���� ����߿��� ���� ���� �Ұ�
					break;

				if (weapon[wall].open == true)
				{
					selected_weapon = wall;
					InvalidateRect(hWnd, NULL, false);
				}
				break;
			}
			case '6':  //rocket ����
			{
				if (player.isattacking == true)  //���� ����߿��� ���� ���� �Ұ�
					break;

				if (weapon[rocket].open == true)
				{
					selected_weapon = rocket;
					InvalidateRect(hWnd, NULL, false);
				}
				break;
			}
			case VK_SPACE:  //���� ���
			{
				if (Remaining_bullet_check())      //�Ѿ� ���� ���� ���� �ƴ���
				{
					//�޼����� ���ٴ���... �Ѿ��� ������ �˸��� ���� 
					//�����ٸ� �׳� ���� �� ���� break�� Ż�����ڱ�?

				}
				else
				{

					/*   ����! pistol�� �Ѿ��� �����̹Ƿ� pistol �߻�� �Ѿ� �پ��� �ȴ�!            */
					/*   ����! �Ѹ��� �߻� �����̰� ����! �߻��� Ÿ�̸ӷ� ������ �����ϴ°� ��������!  */  // wall�� barrel�� �����̰� ����! 

					if (selected_weapon == pistol)
					{
						if (player.isattacking == false)
						{
							SetTimer(hWnd, atk_player, weapon[selected_weapon].delay, TimerProc);
							player.isattacking = true;
						}
					}
					else if (selected_weapon == uzi)
					{
						if (player.isattacking == false)
						{
							SetTimer(hWnd, atk_player, weapon[selected_weapon].delay, TimerProc);
							player.isattacking = true;
						}
					}
					else if (selected_weapon == shotgun)
					{
						if (player.isattacking == false)
						{
							SetTimer(hWnd, atk_player, weapon[selected_weapon].delay, TimerProc);
							player.isattacking = true;
						}
					}
					else if (selected_weapon == barrel)
					{
						/*               ��� �浹üũ                */
						int temp_x;
						int temp_y;

						for (int i = 0; i < 31; i++)
						{
							if ((block[i][0].rect.top <= player.y) && (player.y <= block[i + 1][0].rect.bottom))//y���� ã��
							{
								temp_y = i;
								break;
							}
						}
						for (int i = 0; i < 36; i++)
						{
							if ((block[0][i].rect.left <= player.x) && (player.x <= block[0][i + 1].rect.right))//x���� ã��
							{
								temp_x = i;
								break;
							}
						}
						if (block[temp_y][temp_x].isempty == false)  //������� �ʴٸ� break;
							break;
						else  //���� ���
						{
							block[temp_y][temp_x].isbarrel = true;
							block[temp_y][temp_x].isempty = false;
							weapon[selected_weapon].bullet--;
						}

						InvalidateRect(hWnd, NULL, false);
					}
					else if (selected_weapon == wall)
					{
						/*               ��� �浹üũ                */
						int temp_x;
						int temp_y;

						for (int i = 0; i < 31; i++)
						{
							if ((block[i][0].rect.top <= player.y) && (player.y <= block[i + 1][0].rect.bottom))//y���� ã��
							{
								temp_y = i;
								break;
							}
						}
						for (int i = 0; i < 36; i++)
						{
							if ((block[0][i].rect.left <= player.x) && (player.x <= block[0][i + 1].rect.right))//x���� ã��
							{
								temp_x = i;
								break;
							}
						}
						if (block[temp_y][temp_x].isempty == false)  //������� �ʴٸ� break;
							break;
						else  //wall ���
						{
							block[temp_y][temp_x].iswall = true;
							block[temp_y][temp_x].isempty = false;
							weapon[selected_weapon].bullet--;
						}

						InvalidateRect(hWnd, NULL, false);
					}
					else if (selected_weapon == rocket)
					{

						if (player.isattacking == false)
						{
							SetTimer(hWnd, atk_player, weapon[selected_weapon].delay, TimerProc);
							player.isattacking = true;
						}
					}

				}
				break;
			}

			break;
			}
		}
		else if (current_page == end_page)
		{

		}
		break;
	}
	case WM_KEYUP:
	{
		if (current_page == game_page)
		{
			switch (wParam)
			{
			case 'w':
			case 'W':
			{
				if (player.direction == NE)       //�̵� ������ ������
					player.direction = E;
				else if (player.direction == NW)  //�̵� ������ ����
					player.direction = W;
				else if (player.direction == N)   //�̵��� ����
				{
					player.ismoving = false;
					player.sprite_num = 0;
				}

				InvalidateRect(hWnd, NULL, false);
				break;
			}
			case 'a':
			case 'A':
			{
				if (player.direction == NW)       //�̵� ������ ������
					player.direction = N;
				else if (player.direction == SW)  //�̵� ������ ������
					player.direction = S;
				else if (player.direction == W)   //�̵��� ����
				{
					player.ismoving = false;
					player.sprite_num = 0;
				}

				InvalidateRect(hWnd, NULL, false);
				break;
			}
			case 's':
			case 'S':
			{
				if (player.direction == SE)       //�̵� ������ ������
					player.direction = E;
				else if (player.direction == SW)  //�̵� ������ ����
					player.direction = W;
				else if (player.direction == S)   //�̵��� ����
				{
					player.ismoving = false;
					player.sprite_num = 0;
				}

				InvalidateRect(hWnd, NULL, false);
				break;
			}
			case 'd':
			case 'D':
			{
				if (player.direction == NE)       //�̵� ������ ������
					player.direction = N;
				else if (player.direction == SE)  //�̵� ������ ������
					player.direction = S;
				else if (player.direction == E)   //�̵��� ����
				{
					player.ismoving = false;
					player.sprite_num = 0;
				}

				InvalidateRect(hWnd, NULL, false);
				break;
			}
			case VK_SPACE:  //���� ����
			{
				if (player.isattacking == true)
				{
					player.sprite_num = 0;
					player.isattacking = false;
					KillTimer(hWnd, atk_player);
					InvalidateRect(hWnd, NULL, false);
				}

			}
			break;
			}

		}
		else if (current_page == end_page)
		{


		}

		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CALLBACK TimerProc(HWND hWnd, UINT uMSG, UINT idEvent, DWORD dwTime)
{
	switch (idEvent)
	{
	case rest_time:  //�� �������� ���� �� ���� ������������ ��
	{
		if (isrest_time == false)  //�������� �������� ��
		{

			isrest_time = true;
		}
		else    //�߰� �޽Ľð� ������ ��
		{
			//�������� ����
			Stage_start(hWnd);

			isrest_time = false;
			KillTimer(hWnd, rest_time);
		}
		break;
	}
	case spawn_itembox:
	{
		Spawn_itembox();
		InvalidateRect(hWnd, NULL, false);
		break;
	}
	case atk_player: //���� ���
	{
		if (selected_weapon == pistol)
		{
			//���� �߻�
			//ĳ���� ��������Ʈ ����
		}
		else if (selected_weapon == uzi)
		{

			if (Remaining_bullet_check())   //���� �Ѿ� Ȯ�� -> ������ killtimer, isfiring=false
			{
				player.isattacking = false;
				KillTimer(hWnd, atk_player);
				//ĳ���� ��������Ʈ ����

			}
			else
			{

				//���� �߻�
				//ĳ���� ��������Ʈ ����
				weapon[selected_weapon].bullet--;//�Ѿ� ����
			}
		}
		else if (selected_weapon == shotgun)
		{
			if (Remaining_bullet_check())//���� �Ѿ� Ȯ�� -> ������ killtimer, isfiring=false
			{
				player.isattacking = false;
				KillTimer(hWnd, atk_player);
				//ĳ���� ��������Ʈ ����

			}
			else
			{
				//���� �߻�
				//ĳ���� ��������Ʈ ����
				weapon[selected_weapon].bullet--;//�Ѿ� ����
			}
		}
		else if (selected_weapon == rocket)
		{
			if (Remaining_bullet_check())//���� �Ѿ� Ȯ�� -> ������ killtimer, isfiring=false
			{
				player.isattacking = false;
				KillTimer(hWnd, atk_player);
				//ĳ���� ��������Ʈ ����
			}
			else
			{
				//���� �߻�
				//ĳ���� ��������Ʈ ����
				weapon[selected_weapon].bullet--;//�Ѿ� ����
			}
		}

		if (player.isattacking == true)
		{
			if (player.sprite_num == 0)
				player.sprite_num = 1;
			else
				player.sprite_num = 0;
		}

		InvalidateRect(hWnd, NULL, false);

		break;
	}
	case move_player:
	{
		if (player.ismoving == false)
			break;

		int x_crash_check;
		int y_crash_check;

		int speed = 5;

		if (player.isattacking == true)  //�������϶��� �ƴҶ� ����� �ٸ��Ƿ� �浹üũ�� �ٸ���
		{
			x_crash_check = 32;
			y_crash_check = 32;
		}
		else
		{
			x_crash_check = 19;
			y_crash_check = 30;
		}

		if (Crash_check_character2object(speed) == false)
		{
			if (player.direction == N)
			{
				if (!(player.y - y_crash_check - speed < 0))  //�� ��� �浹üũ
					player.y -= speed;
			}
			else if (player.direction == NE)
			{
				if (!(win_x_size * 2 < player.x + x_crash_check + speed))  //�� ��� �浹üũ
					player.x += speed;
				if (!(player.y - y_crash_check - speed < 0))  //�� ��� �浹üũ
					player.y -= speed;
			}
			else if (player.direction == E)
			{
				if (!(win_x_size * 2 < player.x + x_crash_check + speed))  //�� ��� �浹üũ
					player.x += speed;
			}
			else if (player.direction == SE)
			{
				if (!(win_x_size * 2 < player.x + x_crash_check + speed))  //�� ��� �浹üũ
					player.x += speed;
				if (!(win_y_size * 2 < player.y + y_crash_check + speed))  //�� ��� �浹üũ
					player.y += speed;
			}
			else if (player.direction == S)
			{
				if (!(win_y_size * 2 < player.y + y_crash_check + speed))  //�� ��� �浹üũ
					player.y += speed;
			}
			else if (player.direction == SW)
			{
				if (!(player.x - x_crash_check - speed < 0))  //�� ��� �浹üũ
					player.x -= speed;
				if (!(win_y_size * 2 < player.y + y_crash_check + speed))  //�� ��� �浹üũ
					player.y += speed;
			}
			else if (player.direction == W)
			{
				if (!(player.x - x_crash_check - speed < 0))  //�� ��� �浹üũ
					player.x -= speed;
			}
			else if (player.direction == NW)
			{
				if (!(player.x - x_crash_check - speed < 0))  //�� ��� �浹üũ
					player.x -= speed;
				if (!(player.y - y_crash_check - speed < 0))  //�� ��� �浹üũ
					player.y -= speed;
			}
		}

		if (player.isattacking == false)
		{
			if (player.sprite_num == 3)
				player.sprite_num = 0;
			else
				player.sprite_num++;
		}





		InvalidateRect(hWnd, NULL, false);
		break;
	}
	case move_monster:
	{
		int monster_speed = 5;          //���� ���ǵ� - �뷱�� ���� �ʿ�
		int monster_attack_range = 14;  //���� �����Ÿ� - �뷱�� ���� �ʿ�

		int x_crash_check;
		int y_crash_check;
		RECT monster_attack_range_rect = {};  //���� �����Ÿ� rect - intersectrect�� ���
		RECT temp_rect = {};             //intersectrect ����� �޾ƿ��� rect. ���ڸ� ����θ� �Լ��� �۵��� ���ؼ� �־����. �����ص���

		RECT player_rect;               //�÷��̾� rect - �÷��̾ �������� ���� �ƴҶ� ũ�Ⱑ �ٸ��� ������ ���
		if (player.isattacking == true) //�÷��̾� rect ����
			player_rect = { player.x - 32,player.y - 32,player.x + 32,player.y + 32 };
		else
			player_rect = { player.x - 32,player.y - 32,player.x + 32,player.y + 32 };

		CHARACTER* p = monster_head->next;
		while (p != NULL)
		{

			if (p->isattacking == true)  //�������϶��� �ƴҶ� ����� �ٸ��Ƿ� �浹üũ�� �ٸ���
			{
				x_crash_check = 32;
				y_crash_check = 32;
			}
			else
			{
				x_crash_check = 19;
				y_crash_check = 30;
			}

			if (p->isattacking == false)
			{
				monster_attack_range_rect = { p->x - x_crash_check- monster_attack_range/**/,p->y - y_crash_check- monster_attack_range/**/,
					p->x + x_crash_check+ monster_attack_range /**/,p->y + y_crash_check + monster_attack_range }; //���� �����Ÿ� rect - intersectrect�� ���

				if (IntersectRect(&temp_rect,&player_rect,&monster_attack_range_rect))          //�̵��� �÷��̾ �����Ÿ� �ȿ� ������ 
				{
					//-> ���ݸ�� ����
					p->sprite_num = 0;
					p->isattacking = true;
					p->ismoving = false;

				}
				else if (abs(player.x - p->x) < character_width)   
				{
					if (player.y > p->y)
					{
						p->direction = S;
					}
					else
					{
						p->direction = N;
					}
				}
				else if (abs(player.y - p->y) < character_height)
				{
					if (player.x > p->x)
					{
						p->direction = E;
					}
					else
					{
						p->direction = W;
					}
				}
				else if ((player.y > p->y) && (p->x < player.x))
				{
					p->direction = SE;
				}
				else if ((player.y > p->y) && (p->x > player.x))
				{
					p->direction = SW;
				}
				else if ((player.y < p->y) && (p->x > player.x))
				{
					p->direction = NW;
				}
				else if ((player.y < p->y) && (p->x < player.x))
				{
					p->direction = NE;
				}

				if (p->isattacking == false)  //�����߿� �̵��� ����
				{
					if (p->direction == N)
					{
						if (!(p->y - y_crash_check - monster_speed < 0))  //�� ��� �浹üũ
							p->y -= monster_speed;
					}
					else if (p->direction == NE)
					{
						if (!(win_x_size * 2 < p->x + x_crash_check + monster_speed))  //�� ��� �浹üũ
							p->x += monster_speed;
						if (!(p->y - y_crash_check - monster_speed < 0))  //�� ��� �浹üũ
							p->y -= monster_speed;
					}
					else if (p->direction == E)
					{
						if (!(win_x_size * 2 < p->x + x_crash_check + monster_speed))  //�� ��� �浹üũ
							p->x += monster_speed;
					}
					else if (p->direction == SE)
					{
						if (!(win_x_size * 2 < p->x + x_crash_check + monster_speed))  //�� ��� �浹üũ
							p->x += monster_speed;
						if (!(win_y_size * 2 < p->y + y_crash_check + monster_speed))  //�� ��� �浹üũ
							p->y += monster_speed;
					}
					else if (p->direction == S)
					{
						if (!(win_y_size * 2 < p->y + y_crash_check + monster_speed))  //�� ��� �浹üũ
							p->y += monster_speed;
					}
					else if (p->direction == SW)
					{
						if (!(p->x - x_crash_check - monster_speed < 0))  //�� ��� �浹üũ
							p->x -= monster_speed;
						if (!(win_y_size * 2 < p->y + y_crash_check + monster_speed))  //�� ��� �浹üũ
							p->y += monster_speed;
					}
					else if (p->direction == W)
					{
						if (!(p->x - x_crash_check - monster_speed < 0))  //�� ��� �浹üũ
							p->x -= monster_speed;
					}
					else if (p->direction == NW)
					{
						if (!(p->x - x_crash_check - monster_speed < 0))  //�� ��� �浹üũ
							p->x -= monster_speed;
						if (!(p->y - y_crash_check - monster_speed < 0))  //�� ��� �浹üũ
							p->y -= monster_speed;
					}

					if (p->sprite_num == 3)
						p->sprite_num = 0;
					else
						p->sprite_num++;
				}
			}
			else  //���� 
			{
				if (p->sprite_num == 0)
				{
					//�÷��̾� �ǰ�����
					p->sprite_num++;
				}
				else if (p->sprite_num == 1)   //���� ����
				{
					p->sprite_num = 0;
					p->isattacking = false;
					p->ismoving = true;
				}
			}

			p = p->next;
		}

		InvalidateRect(hWnd, NULL, false);	
		break;
	}

	break;
	}



}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Game_start_setting(HWND hWnd)         //���� ���� ����(���� �ʱ�ȭ ���)
{
	stage = 0;                    //�������� �ʱ�ȭ (�������� ���� �Լ����� 1�� ������Ű�Ƿ� 0���� �ʱ�ȭ)
	score = 0;                    //���ھ� �ʱ�ȭ

	player.max_health = 1000;     //ĳ���� �ִ�ü�� 1000���� ����
	player.health = player.max_health;
	player.x = 900;               //��ġ �߾�
	player.y = 800;
	player.direction = S;         //�Ʒ��� �ٶ�
	player.sprite_num = 0;        //�⺻ �̹���
	player.ismoving = false;
	player.isattacking = false;
	SetTimer(hWnd, move_player, 50, TimerProc);   //�÷��̾� �̵�

	monster_num = 0;
	boss_num = 0;

	for (int i = 0; i < 32; i++)  //������ �ʱ�ȭ
	{
		for (int j = 0; j < 36; j++)
		{
			block[i][j].iswall = false;
			block[i][j].isinvinciblewall = false;       //�ı� �Ұ����� �� (����) �����Ϸ��� ���⼭! ���������� ��������?
			block[i][j].isbarrel = false;
			block[i][j].isitembox = false;
			block[i][j].isempty = true;
		}
	}
	itembox_num = 0;
	aquired_itembox_num = 0;      //������ ������ �ڽ� ���� �ʱ�ȭ
	for (int i = 0; i < 5; i++)   //�����۹ڽ� 5�� ���� 
		Spawn_itembox();

	Reset_weapon_setting();

	isrest_time = true;

	block[17][15].iswall = true;
	block[17][15].isempty = false;

	block[16][14].isbarrel = true;
	block[16][14].isempty = false;

	SetTimer(hWnd, rest_time, 5000, TimerProc);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Aquire_itembox()             //������ �ڽ��� �Ծ�η���!
{
	/*  ������ �ڽ� ����, itembox_num--���� ó�� �ʿ�  */
	itembox_num--;
	aquired_itembox_num++;

	int opened_weapon_num;        //�߰� ������ ���� ���� (pistol ����)
	for (int i = 5; i > -1; i--)
	{
		if (weapon[i].open == true)
		{
			opened_weapon_num = i;
			break;
		}
	}

	if (2 <= stage)  //2������������ ���� �ϳ��� Ǯ��
	{
		for (int i = 1; i < stage; i++)
		{
			if (weapon[i].open == false)
			{
				weapon[i].open = true;
				return 0;
			}
			if (i == 5)
				break;
		}
	}

	/*             Ư�� ����, �뷱��? ���� ��ȹ �ʿ�              */
	/*                ���� ���׷��̵� �ر� ������                 */
	//if ((aquired_itembox_num == ) || ())
	//{

	//}
	//else if ((aquired_itembox_num == ) || ())
	//{

	//}

	/*             ���׷��̵� �Ұ��ɽ�               */

	int rand_num = rand() % (opened_weapon_num * 2 + 1);   //���̵� ������ �� Ȯ�� �߰��ϸ� ��

	if (rand_num == 0)  //   1/((ȹ���� ���� ����*2)+1)�� Ȯ���� ü��ȸ��
	{
		player.health = player.max_health;
	}
	else if ((rand_num == 1) || (rand_num == 2))
	{
		/*---------------------------------�Ѿ� ȹ��� �ִ���� ���� ���� �ƴ� ���� ��ġ�� ���� (�뷱�� �ʿ�)--------------------------*/

		/*if ((weapon[uzi].bullet +       ) > weapon[uzi].max_bullet)  //�ִ�ź�� ���� �̻����� ȹ�� �Ұ�
		weapon[uzi].bullet = weapon[uzi].max_bullet;
		weapon[uzi].bullet +=       ;*/
	}
	else if ((rand_num == 3) || (rand_num == 4))
	{
		/*---------------------------------�Ѿ� ȹ��� �ִ���� ���� ���� �ƴ� ���� ��ġ�� ���� (�뷱�� �ʿ�)--------------------------*/

		/*if ((weapon[shotgun].bullet +       ) > weapon[shotgun].max_bullet)  //�ִ�ź�� ���� �̻����� ȹ�� �Ұ�
		weapon[shotgun].bullet = weapon[shotgun].max_bullet;
		weapon[shotgun].bullet +=       ;*/
	}
	else if ((rand_num == 5) || (rand_num == 6))
	{
		/*---------------------------------�Ѿ� ȹ��� �ִ���� ���� ���� �ƴ� ���� ��ġ�� ���� (�뷱�� �ʿ�)--------------------------*/

		/*if ((weapon[barrel].bullet +       ) > weapon[barrel].max_bullet)  //�ִ�ź�� ���� �̻����� ȹ�� �Ұ�
		weapon[barrel].bullet = weapon[barrel].max_bullet;
		weapon[barrel].bullet +=       ;*/
	}
	else if ((rand_num == 7) || (rand_num == 8))
	{
		/*---------------------------------�Ѿ� ȹ��� �ִ���� ���� ���� �ƴ� ���� ��ġ�� ���� (�뷱�� �ʿ�)--------------------------*/

		/*if ((weapon[wall].bullet +       ) > weapon[wall].max_bullet)  //�ִ�ź�� ���� �̻����� ȹ�� �Ұ�
		weapon[wall].bullet = weapon[wall].max_bullet;
		weapon[wall].bullet +=       ;*/
	}
	else if ((rand_num == 9) || (rand_num == 10))
	{
		/*---------------------------------�Ѿ� ȹ��� �ִ���� ���� ���� �ƴ� ���� ��ġ�� ���� (�뷱�� �ʿ�)--------------------------*/

		/*if ((weapon[rocket].bullet +       ) > weapon[rocket].max_bullet)  //�ִ�ź�� ���� �̻����� ȹ�� �Ұ�
		weapon[rocket].bullet = weapon[rocket].max_bullet;
		weapon[rocket].bullet +=       ;*/
	}

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Spawn_itembox()     //18*16 ��Ͽ��� ����ִ� ��Ͽ��� �������� ������ �ڽ� ����	
{
	if (itembox_num > 4) //�����۹ڽ��� 5��������! (�뷱�� �ʿ�)
		return 1;
	else
	{
		int temp_x;
		int temp_y;

		while (true)     //����ִ� ĭ�� ã�ƺ��ô�!
		{
			temp_x = rand() % 36;    //0 ~ ���� ��� ����(36)-1 ���� ����
			temp_y = rand() % 32;    //0 ~ ���� ��� ����(32)-1 ���� ����

			if (block[temp_y][temp_x].isempty == true)
				break;
		}

		block[temp_y][temp_x].isitembox = true;
		block[temp_y][temp_x].isempty = false;

		itembox_num++;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Char_Deathcheck(HWND hWnd)    //ĳ���� ü�� 0�ƴ��� Ȯ�� �Լ� ����߻�, ���� ���ݽÿ� �� �ٸ� �����ָ� �ȴٱ�?? ���� ��� üũ �Լ��� ���� ����Ŷ�, Ī��!
{
	if (player.health == 0)
	{
		current_page = end_page;

		//Ÿ�̸� �༮��.. ��...�� �׿������ھ�!!! ���ӾƾƾƾƤ����ƾƤ�����!!!
		//KillTimer(hWnd,);
		//KillTimer(hWnd.);
		//KillTimer(hWnd.);
		//KillTimer(hWnd,);
		KillTimer(hWnd, spawn_itembox);

		InvalidateRect(hWnd, NULL, false);
	}

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Stage_start(HWND hWnd)   //�������� ���� ����(���� ����) �� �Լ� �ۼ� ���������� �������´�� ���°ž� �������
{
	stage++;
	SetTimer(hWnd, rest_time, 5000, TimerProc);

	Spawn_monster();
	Spawn_boss();
	SetTimer(hWnd, move_monster, 100, TimerProc);
	//�� �ʿ��� �������� �ϴ� Ÿ�̸� ���� - �� ������ҿ��� �ѵθ����� ������, �Ա��� ������ ������ ���Ұ�� ������ �ʵ��� �ؾ���
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Spawn_monster()
{
	/* ���� �� ���� �뷱�� �ʿ� */
	CHARACTER* p = monster_head;
	int i = 0;

	while (i < 1/*���� �뷱�� ����*/)
	{
		//head���� �������� �ʰ� ����ϱ�
		CHARACTER* temp_character = (CHARACTER*)malloc(sizeof(CHARACTER));

		temp_character->health = 100;
		temp_character->max_health = 100;
		temp_character->x = 1000;
		temp_character->y = 1000;
		temp_character->direction = E;
		temp_character->sprite_num = 0;
		temp_character->ismoving = false;
		temp_character->isattacking = false;
		temp_character->next = NULL;

		p->next = temp_character;

		monster_num++;
		i++;
	}

	//�� ���� - ���Ḯ��Ʈ
	//head���� �������� �ʰ� ����ϱ�
	//�� ��ǥ ���� - ���̵� : HELL
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Spawn_boss()
{
	/* ���� �� ���� �뷱�� �ʿ� */
	CHARACTER* p = boss_head;
	int i = 0;

	while (i < 50/*���� �뷱�� ����*/)
	{
		//head���� �������� �ʰ� ����ϱ�
		CHARACTER* temp_character = (CHARACTER*)malloc(sizeof(CHARACTER));

		temp_character->health = 500;
		temp_character->max_health = 500;
		//	temp_character->x = 
		//	temp_character->y = 1
		//	temp_character->direction = 
		temp_character->sprite_num = 0;
		temp_character->ismoving = true;
		temp_character->isattacking = false;
		temp_character->next = NULL;

		p->next = temp_character;

		monster_num++;
		i++;
	}

	//�� ���� - ���Ḯ��Ʈ
	//head���� �������� �ʰ� ����ϱ�
	//�� ��ǥ ���� - ���̵� : HELL
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Reset_weapon_setting()
{
	/*              ���⺰ ����, �뷱�� �ʿ�                            */
	Reset_weapon_upgrade();
	/*pistol = 0, uzi = 1, shotgun = 2, barrel = 3, wall = 4, rocket = 5*/

	/*             ���⺰ �ִ� ź�� �� ����, �뷱�� �ʿ�                */
	weapon[pistol].max_bullet = 9999;   //pistol�� �Ѿ� ����
	weapon[1].max_bullet = 40;
	weapon[2].max_bullet = 20;
	weapon[3].max_bullet = 10;
	weapon[4].max_bullet = 10;
	weapon[5].max_bullet = 5;

	/*             ���⺰ ������ ����, �뷱�� �ʿ�                      */
	weapon[pistol].damage = 20;
	weapon[1].damage = 20;
	weapon[2].damage = 50;
	weapon[3].damage = 300;
	weapon[4].damage = 0;  //wall ������ ����
	weapon[5].damage = 300;

	/*            ���⺰ �����Ÿ� ����, �뷱�� �ʿ�                     */
	weapon[pistol].range = 200;
	weapon[1].range = 400;
	weapon[2].range = 100;
	weapon[3].range = 0;
	weapon[4].range = 0;
	weapon[5].range = 2000;  //�����Ÿ� ����

	/*            ���⺰ ������ ����, �뷱�� �ʿ�                     */
	weapon[pistol].delay = 200;
	weapon[1].delay = 400;
	weapon[2].delay = 100;
	weapon[3].delay = 0;
	weapon[4].delay = 0;
	weapon[5].delay = 2000;

	/*            ���⺰ ����, �뷱�� �ʿ�                              */

	for (int i = 1; i < 6; i++)
	{
		weapon[i].open = false;
		weapon[i].bullet = 0;
	}

	weapon[pistol].open = true;   //ó���� ���ѹۿ� ����.. ��ĳ��..
	weapon[pistol].bullet = weapon[pistol].max_bullet;

	selected_weapon = pistol;     //���� ����
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Reset_weapon_upgrade()   //���� ���׷��̵� �ʱ�ȭ
{
	pistol_range_up1 = false;
	pistol_delay_down1 = false;
	//pistol �Ѿ� ����

	uzi_range_up1 = false;
	uzi_delay_down1 = false;
	uzi_maxammo_up1 = false;
	uzi_maxammo_up2 = false;

	shotgun_range_up1 = false;
	shotgun_delay_down1 = false;
	shotgun_maxammo_up1 = false;
	shotgun_maxammo_up2 = false;

	barrel_maxammo_up1 = false;
	barrel_maxammo_up2 = false;

	wall_maxammo_up1 = false;
	wall_maxammo_up2 = false;

	//rocket �����Ÿ� ����
	rocket_delay_down1 = false;
	rocket_maxammo_up1 = false;
	rocket_maxammo_up2 = false;
}

bool Remaining_bullet_check()  //���� �Ѿ� Ȯ��  �������� false ������ true
{
	if (weapon[selected_weapon].bullet != 0)
	{
		return false;
	}
	else
		return true;
}

bool Crash_check_character2object(int speed)    //����������Ʈ���� �浹�˻� wall, barrel, itembox 
{
	int character_block_x = player.x / 50;  //�Ҽ��� ����
	int character_block_y = player.y / 50;  //�Ҽ��� ����

	RECT character_rect;
	RECT temp_rect = {};  //intersectrect ����� �޾ƿ��� rect. ���ڸ� ����θ� �Լ��� �۵��� ���ؼ� �־����. �����ص���

	if (player.isattacking)
	{
		character_rect = { player.x - 32,player.y - 32,player.x + 32,player.y + 32 };

	}
	else
	{
		character_rect = { player.x - 19,player.y - 30,player.x + 19,player.y + 30 };
	}


	if (player.direction == N)  //���� ��ǥ ����x �̵� �Ŀ� ����� ��ġ����
	{
		character_rect.top -= speed;
	}
	else if (player.direction == NE)
	{
		character_rect.top -= speed;
		character_rect.right += speed;

	}
	else if (player.direction == E)
	{
		character_rect.right += speed;
	}
	else if (player.direction == SE)
	{
		character_rect.bottom += speed;
		character_rect.right += speed;
	}
	else if (player.direction == S)
	{
		character_rect.bottom += speed;
	}
	else if (player.direction == SW)
	{
		character_rect.bottom += speed;
		character_rect.left -= speed;
	}
	else if (player.direction == W)
	{
		character_rect.left -= speed;
	}
	else if (player.direction == NW)
	{
		character_rect.top -= speed;
		character_rect.left -= speed;
	}

	for (int i = (player.x / 50 - 1); i <= (player.x / 50 + 1); i++)
	{
		if (i < 0)
			continue;

		for (int j = (player.y / 50 - 1); j <= (player.y / 50 + 1); j++)
		{
			if (j < 0)
				continue;

			if (IntersectRect(&temp_rect, &block[j][i].rect, &character_rect))
			{
				if ((block[j][i].isinvinciblewall) || (block[j][i].iswall) || (block[j][i].isbarrel))   //�������� ������
				{

					return true;
				}
				else if (block[j][i].isitembox)
				{
					block[j][i].isitembox = false;
					block[j][i].isempty = true;
					Aquire_itembox();  //�����۹ڽ� ����

					return false;
				}
			}


		}
	}
	return false;
}

