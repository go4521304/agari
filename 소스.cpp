#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <random>
#include <time.h>
#include <atlimage.h>


HINSTANCE g_hInst;
LPCTSTR lpszClass = _T("agari!");

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
	move_player,
	spawn_monster, move_monster,
	spawn_itembox,
	pistol_delay, uzi_delay, shotgun_delay, rocket_delay
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
	bool isdelay;           //���� ������ ����

	CImage img;              //���� �̹���
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
void Aquire_itembox();      //������ �ڽ��� �Ծ�η���!
void Spawn_itembox();
void Char_Deathcheck(HWND hWnd);
void Stage_start(HWND hWnd);
void Reset_weapon_setting();
void Reset_weapon_upgrade();
void Spawn_monster();
void Spawn_boss();
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

int monster_num;          //���� ���� ����
int boss_num;             //���� �������� ����



RECT logo_rect = { 120,120,750,370 };          //�ΰ� ��ġ      (��ŸƮȭ��)
RECT play_button_rect = { 600,480,750,550 };   //play��ư ��ġ  (��ŸƮȭ��)
RECT exit_button_rect = { 600,600,750,670 };   //exit��ư ��ġ  (��ŸƮȭ��)
											   
RECT weapon_image_rect[6] = { { 220,680,270,730 },{ 300,680,350,730 },{ 380,660,430,710 },{ 460,680,510,730 },{ 540,680,590,730 },{ 620,680,670,730 }};  //���� 6�� �̹��� ��ġ

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


						/*         ���� �̹����� WEAPON ����ü �ȿ� ����!       */

CImage char_move_sprite[10][8][4]; //ĳ���� �̵� ��������Ʈ [ĳ���� ��ȣ][����][�̵� ��� 4��]
CImage char_atk_sprite[10][8][2];  //ĳ���� ���� ��������Ʈ [ĳ���� ��ȣ][����][���� ��� 2��]
int current_char_num;              //������ ĳ���� ��ȣ  char_move_sprite[current_char_num][direction][n]���� ���

CImage monster_move_sprite[8][4]; //���� �̵� ��������Ʈ [����][�̵� ��� 4��(?)]
CImage monster_atk_sprite[8][2];  //���� ���� ��������Ʈ[����][���� ��� 2��(?)]

/*********************************************��Ʈ�� �̹���*****************************************************/

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	HWND hWnd;
	MSG Message;
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

	hWnd = CreateWindow(lpszClass, _T("agari!"), WS_OVERLAPPEDWINDOW, 0, 0, win_x_size + 25, win_y_size + 25, NULL, (HMENU)NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	srand(GetTickCount());  //��ƽī��Ʈ! �����õ�~

	while (GetMessage(&Message, 0, 0, 0)) {
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

	CImage img;
	CImage dc;

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

		/*********************************************�̹��� �ε�*****************************************************/

		current_page = start_page;  //���� �������� ������������

		for (int i = 0; i < 32; i++)
		{
			for (int j = 0; j < 36; j++)
			{
				block[i][j].rect = { 50 * j,50 * i,50 * j + 50,50 * i + 50 };  //36*32 ��� ��ǥ �Է�
			}
		}


		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		if (current_page == start_page)
		{
			dc.Create(win_x_size, win_y_size, 24);	// == CreateCompatibleBitmap
			memdc1 = dc.GetDC();					// == CreateComaptibleDC

			start_page_bk_img.Draw(memdc1, 0,0,win_x_size,win_y_size);
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
			dc.Create(win_x_size, win_y_size, 24);	// == CreateCompatibleBitmap
			memdc1 = dc.GetDC();					// == CreateComaptibleDC
			//��� ���

			//�����۹ڽ� ���

			//ĳ����, ���� ���

			//ü�¹� ���

			//������Ʈ ���(��, ����)

			//�Ѿ� ���

			//ui ���

			dc.Draw(hdc, 0, 0, win_x_size, win_y_size);	// �Ʒ��� Bitblt�� ����
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
			{
				if ((player.direction == N) || (player.direction == S))  //�̵� ������ ������
					player.direction = N;
				else if (player.direction == W)  //�̵� ������ �ϼ���
					player.direction = NW;
				else if (player.direction == E)  //�̵� ������ �ϵ�����
					player.direction = NE;

				if (player.ismoving == false)    //�����־��ٸ�
					player.ismoving = true;

				InvalidateRect(hWnd, NULL, false);
				break;
			}
			case 'a':
			{
				if ((player.direction == E) || (player.direction == W))  //�̵� ������ ����
					player.direction = W;
				else if (player.direction == N)  //�̵� ������ �ϼ���
					player.direction = NW;
				else if (player.direction == S)  //�̵� ������ ������
					player.direction = SW;

				if (player.ismoving == false)    //�����־��ٸ�
					player.ismoving = true;

				InvalidateRect(hWnd, NULL, false);
				break;
			}
			case 's':
			{
				if ((player.direction == N) || (player.direction == S))  //�̵� ������ ������
					player.direction = S;
				else if (player.direction == W)  //�̵� ������ ������
					player.direction = SW;
				else if (player.direction == E)  //�̵� ������ ��������
					player.direction = SE;

				if (player.ismoving == false)    //�����־��ٸ�
					player.ismoving = true;

				InvalidateRect(hWnd, NULL, false);
				break;
			}
			case 'd':
			{
				if ((player.direction == E) || (player.direction == W))  //�̵� ������ ������
					player.direction = E;
				else if (player.direction == N)  //�̵� ������ �ϵ�����
					player.direction = NE;
				else if (player.direction == S)  //�̵� ������ ��������
					player.direction = SE;

				if (player.ismoving == false)    //�����־��ٸ�
					player.ismoving = true;

				InvalidateRect(hWnd, NULL, false);
				break;
			}

			case 'q':  //���� ���� ����
			{
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
			{
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
				if (weapon[pistol].open == true)
				{
					selected_weapon = pistol;
					InvalidateRect(hWnd, NULL, false);
				}

				break;
			}
			case '2':  //uzi ����
			{
				if (weapon[uzi].open == true)
				{
					selected_weapon = uzi;
					InvalidateRect(hWnd, NULL, false);
				}
				break;
			}
			case '3':  //shotgun ����
			{
				if (weapon[shotgun].open == true)
				{
					selected_weapon = shotgun;
					InvalidateRect(hWnd, NULL, false);
				}
				break;
			}
			case '4':  //barrel ����
			{
				if (weapon[barrel].open == true)
				{
					selected_weapon = barrel;
					InvalidateRect(hWnd, NULL, false);
				}
				break;
			}
			case '5':  //wall ����
			{
				if (weapon[wall].open == true)
				{
					selected_weapon = wall;
					InvalidateRect(hWnd, NULL, false);
				}
				break;
			}
			case '6':  //rocket ����
			{
				if (weapon[rocket].open == true)
				{
					selected_weapon = rocket;
					InvalidateRect(hWnd, NULL, false);
				}
				break;
			}
			case VK_SPACE:  //���� ���
			{
				if (weapon[selected_weapon].bullet == 0)      //�Ѿ� ���� ���� ���� �ƴ���
				{
					//�޼����� ���ٴ���... �Ѿ��� ������ �˸��� ���� 
					//�����ٸ� �׳� ���� �� ���� break�� Ż�����ڱ�?

				}
				else
				{
					if (weapon[selected_weapon].isdelay == true)  //���� ��� �����̽� �߻� �Ұ�!
						break;


					/*   ����! pistol�� �Ѿ��� �����̹Ƿ� pistol �߻�� �Ѿ� �پ��� �ȴ�!            */
					/*   ����! �Ѹ��� �߻� �����̰� ����! �߻��� Ÿ�̸ӷ� ������ �����ϴ°� ��������!  */  // wall�� barrel�� �����̰� ����! 

					if (selected_weapon == pistol)
					{
						//���� �߻�
						//������ ����
					}
					else if (selected_weapon == uzi)
					{
						//���� �߻�
						//������ ����
					}
					else if (selected_weapon == shotgun)
					{
						//���� �߻�
						//������ ����
					}
					/*�Ϸ�*/else if (selected_weapon == barrel)
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
						else
						{
							block[temp_y][temp_x].isbarrel = true;
							block[temp_y][temp_x].isempty = false;
						}
					
						InvalidateRect(hWnd, NULL, false);
					}
					/*�Ϸ�*/else if (selected_weapon == wall)
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
						else
						{
							block[temp_y][temp_x].iswall = true;
							block[temp_y][temp_x].isempty = false;
						}

						InvalidateRect(hWnd, NULL, false);
					}
					else if (selected_weapon == rocket)
					{
						//���� �߻�
						//������ ����
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
			{
				if (player.direction == NE)       //�̵� ������ ������
					player.direction = E;
				else if (player.direction == NW)  //�̵� ������ ����
					player.direction = W;
				else if (player.direction == N)   //�̵��� ����
				{
					player.ismoving = false;
					//�������� �ƴҰ�� ��������Ʈ�� �⺻���� 
				}

				InvalidateRect(hWnd, NULL, false);
				break;
			}
			case 'a':
			{
				if (player.direction == NW)       //�̵� ������ ������
					player.direction = N;
				else if (player.direction == SW)  //�̵� ������ ������
					player.direction = S;
				else if (player.direction == W)   //�̵��� ����
				{
					player.ismoving = false;
					//�������� �ƴҰ�� ��������Ʈ�� �⺻���� 
				}

				InvalidateRect(hWnd, NULL, false);
				break;
			}
			case 's':
			{
				if (player.direction == SE)       //�̵� ������ ������
					player.direction = E;
				else if (player.direction == SW)  //�̵� ������ ����
					player.direction = W;
				else if (player.direction == S)   //�̵��� ����
				{
					player.ismoving = false;
					//�������� �ƴҰ�� ��������Ʈ�� �⺻���� 
				}

				InvalidateRect(hWnd, NULL, false);
				break;
			}
			case 'd':
			{
				if (player.direction == NE)       //�̵� ������ ������
					player.direction = N;
				else if (player.direction == SE)  //�̵� ������ ������
					player.direction = S;
				else if (player.direction == E)   //�̵��� ����
				{
					player.ismoving = false;
					//�������� �ƴҰ�� ��������Ʈ�� �⺻���� 
				}

				InvalidateRect(hWnd, NULL, false);
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
	player.x = 900;  //��ġ �߾�
	player.y = 800;
	player.direction = S;  //�Ʒ��� �ٶ�
	player.sprite_num = 0; //�⺻ �̹���
	player.ismoving = false;
	player.isattacking = false;

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
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Aquire_itembox()             //������ �ڽ��� �Ծ�η���!
{
	int opened_weapon_num;        //�߰� ������ ���� ���� (pistol ����)

	for (int i = 5; i > -1; i--)
	{
		if (weapon[i].open == true)
		{
			opened_weapon_num = i;
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

void Spawn_itembox()     //18*16 ��Ͽ��� ����ִ� ��Ͽ��� �������� ������ �ڽ� ����	
{
	if (itembox_num < 6) //�����۹ڽ��� 5��������! (�뷱�� �ʿ�)
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
		KillTimer(hWnd,spawn_itembox);
	
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

	//�� �ʿ��� �������� �ϴ� Ÿ�̸� ���� - �� ������ҿ��� �ѵθ����� ������, �Ա��� ������ ������ ���Ұ�� ������ �ʵ��� �ؾ���
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Spawn_monster()
{
	/* ���� �� ���� �뷱�� �ʿ� */
	CHARACTER* p = monster_head;
	int i = 0;

	while (i < 50/*���� �뷱�� ����*/)
	{
		//head���� �������� �ʰ� ����ϱ�
		CHARACTER* temp_character = (CHARACTER*)malloc(sizeof(CHARACTER));

		temp_character->health = 100;
		temp_character->max_health = 100;
		//temp_character->x=
		//temp_character->y=
		//temp_character->direction=
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
		//temp_character->x=
		//temp_character->y=
		//temp_character->direction=
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

  /*            ���⺰ ����, �뷱�� �ʿ�                              */

	for (int i = 1; i < 6; i++)
	{
		weapon[i].open = false;
		weapon[i].bullet = 0;
		weapon[i].isdelay = false;
	}

	weapon[pistol].open = true;   //ó���� ���ѹۿ� ����.. ��ĳ��..
	weapon[pistol].bullet = weapon[pistol].max_bullet;
	weapon[pistol].isdelay = false;

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

