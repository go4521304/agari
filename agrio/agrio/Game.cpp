#include "stdfx.h"
#include "GameObject.h"
#include "GameFramework.h"

//#define SERVERIP	"121.173.248.91"
#define SERVERIP	"121.139.87.12"
#define SERVERPORT	9000
#define BUFSIZE		512


/********************************** Main **********************************/
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
	WndClass.hInstance = g_hInst;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&WndClass);

	hWnd = CreateWindow(lpszClass, lpszClass, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME,
		0, 0, win_x_size + 14, win_y_size + 36, NULL, (HMENU)NULL, g_hInst, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);


	while (true)
	{

		if (::PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
		{
			if (Message.message == WM_QUIT ||
				Message.message == WM_CLOSE ||
				Message.message == WM_DESTROY)
				break;

			::TranslateMessage(&Message);
			::DispatchMessage(&Message);
		}
		else
		{
			GameLoop(hWnd);
		}
	}

	return Message.wParam;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GameLoop(HWND hWnd)
{
	const long long FrameRate = 1000 / 30;
	while (GetTickCount64() < TIMER + FrameRate);

	if (scene == SCENE::title)
	{
		selTimer -= GetTickCount64() - TIMER;
		if (selTimer < 0)
		{
			selAnimation++;
			selAnimation %= 4;
			selTimer = ANIMATION_TIME;
		}
	}

	else if (scene == SCENE::gameover || scene == SCENE::winner)
	{

	}

	// lobby, stage1, stage2 ����...
	else
	{
		Player* p = reinterpret_cast<Player*>(gameObject[playerID]);
		if (keyAction.reqSend)
		{
			STATE oldState = p->GetState();

			if (keyAction.left && keyAction.up)
				p->SetDir(DIR::NW);

			else if (keyAction.right && keyAction.up)
				p->SetDir(DIR::NE);

			else if (keyAction.left && keyAction.down)
				p->SetDir(DIR::SW);

			else if (keyAction.right && keyAction.down)
				p->SetDir(DIR::SE);

			else if (keyAction.left)
				p->SetDir(DIR::W);

			else if (keyAction.right)
				p->SetDir(DIR::E);

			else if (keyAction.up)
				p->SetDir(DIR::N);

			else if (keyAction.down)
				p->SetDir(DIR::S);



			if (keyAction.left == false && keyAction.right == false && keyAction.up == false && keyAction.down == false)
			{
				if (p->GetState() != STATE::idle)
					SendStatePacket(STATE::idle);
			}
			else
			{
				cs_packet_player_move sendPacket;
				sendPacket.packetSize = sizeof(sendPacket);
				sendPacket.packetType = CS_PACKET_PLAYER_MOVE;
				sendPacket.dir = (char)p->GetDir();
				Send(&sendPacket);
				std::cout << "move!" << std::endl;
				if (p->GetState() != STATE::move)
					SendStatePacket(STATE::move);
			}

			if (keyAction.space)
			{
				SendStatePacket(STATE::attack);
			}
			keyAction.reqSend = false;
		}

		if (itemTimer >= 0)
			itemTimer -= GetTickCount64() - TIMER;

		if (keyAction.space)
		{
			if (itemTimer < 0)
			{
				if (selectedWeapon == pistol || selectedWeapon == uzi || selectedWeapon == shotgun)
				{
					cs_packet_shoot_bullet sendPacket;
					sendPacket.packetSize = sizeof(sendPacket);
					sendPacket.packetType = CS_PACKET_SHOOT_BULLET;
					sendPacket.playerID = playerID;
					Send(&sendPacket);
					std::cout << "Use!" << std::endl;

					p->UseItem(selectedWeapon - 1);	// �÷��̾ ������ �ִ� ���� ����
					itemTimer = ITEM_TIME[selectedWeapon - 1];	// ������ ������ �߻� �ð�����
				}

				else if (selectedWeapon == potion || selectedWeapon == box)
				{
					cs_packet_used_item sendPacket;
					sendPacket.packetSize = sizeof(sendPacket);
					sendPacket.packetType = CS_PACKET_USED_ITEM;
					sendPacket.itemNum = selectedWeapon;
					Send(&sendPacket);

					p->UseItem(selectedWeapon - 1);	// �÷��̾ ������ �ִ� ���� ����
					itemTimer = ITEM_TIME[selectedWeapon - 1];	// ������ ������ �߻� �ð�����
				}
			}
		}

		for (int i = 0; i < 3; ++i)
		{
			Player* player = reinterpret_cast<Player*>(gameObject[i]);
			if (player->GetState() == STATE::idle)
			{
				player->animFrame = 2;
				player->animTimer = 0;
			}
			else
			{
				player->animTimer -= GetTickCount64() - TIMER;
				if (player->animTimer < 0)
				{
					player->animFrame++;
					player->animTimer = ANIMATION_TIME;

					if (player->GetState() == STATE::attack)
					{
						player->animFrame %= 2;

						// std::cout << player->animFrame << std::endl;

					}
					else
						player->animFrame %= 4;

				}
			}
		}
	}

	InvalidateRect(hWnd, NULL, false);

	TIMER = GetTickCount64();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc, memdc1, memdc2;
	PAINTSTRUCT ps;

	HBRUSH hbrush, oldbrush;
	HPEN hpen, oldpen;

	CImage img;
	CImage dc, dc2;

	HFONT hFont, oldFont;

	TCHAR str[500] = {};

	int play_size_left, play_size_top;

	static bool play_button = false;  //����ȭ�� start��ư
	static bool exit_button = false;  //����ȭ�� exit��ư

	static bool replay_button = false;  //����ȭ�� replay��ư
	static bool exit2_button = false;  //����ȭ�� exit��ư

	static int MOUSE_X = 0, MOUSE_Y = 0;

	switch (uMsg)
	{
	case WM_CREATE:
	{
		{
			/*********************************************�̹��� �ε�*****************************************************/
			sprites[(int)SPRITE::btnPlay].Load(TEXT("resource/PLAY.png"));
			sprites[(int)SPRITE::btnExit].Load(TEXT("resource/EXIT.png"));
			sprites[(int)SPRITE::btnReplay].Load(TEXT("resource/REPLAY.png"));
			sprites[(int)SPRITE::bgTitle].Load(TEXT("resource/startBack(��).png"));
			sprites[(int)SPRITE::bgLobby].Load(TEXT("resource/lobby1.png"));
			sprites[(int)SPRITE::bgStage1].Load(TEXT("resource/stage1.png"));
			sprites[(int)SPRITE::bgEnd].Load(TEXT("resource/endBack.png"));

			sprites[(int)SPRITE::Izuna].Load(TEXT("resource/Izuna_move.png"));
			sprites[(int)SPRITE::GenAn].Load(TEXT("resource/Gen-An_move.png"));
			sprites[(int)SPRITE::Hinagiku].Load(TEXT("resource/Hinagiku_move.png"));
			sprites[(int)SPRITE::Ichika].Load(TEXT("resource/Ichika_move.png"));
			sprites[(int)SPRITE::Kagen].Load(TEXT("resource/Kagen_move.png"));
			sprites[(int)SPRITE::Mitsumoto].Load(TEXT("resource/Mitsumoto_move.png"));
			sprites[(int)SPRITE::Shino].Load(TEXT("resource/Shino_move.png"));
			sprites[(int)SPRITE::Sizune].Load(TEXT("resource/Sizune_move.png"));

			sprites[(int)SPRITE::Izuna_Atk].Load(TEXT("resource/Izuna_attack.png"));
			sprites[(int)SPRITE::GenAn_Atk].Load(TEXT("resource/Gen-An_attack.png"));
			sprites[(int)SPRITE::Hinagiku_Atk].Load(TEXT("resource/Hinagiku_attack.png"));
			sprites[(int)SPRITE::Ichika_Atk].Load(TEXT("resource/Ichika_attack.png"));
			sprites[(int)SPRITE::Kagen_Atk].Load(TEXT("resource/Kagen_attack.png"));
			sprites[(int)SPRITE::Mitsumoto_Atk].Load(TEXT("resource/Mitsumoto_attack.png"));
			sprites[(int)SPRITE::Shino_Atk].Load(TEXT("resource/Shino_attack.png"));
			sprites[(int)SPRITE::Sizune_Atk].Load(TEXT("resource/Sizune_attack.png"));

			sprites[(int)SPRITE::pistol].Load(TEXT("resource/attack_pistol.png"));
			sprites[(int)SPRITE::uzi].Load(TEXT("resource/attack_uzi.png"));
			sprites[(int)SPRITE::shotgun].Load(TEXT("resource/attack_shotgun.png"));
			sprites[(int)SPRITE::box].Load(TEXT("resource/box.png"));

			sprites[(int)SPRITE::uiPistol].Load(TEXT("resource/ui_pistol.png"));
			sprites[(int)SPRITE::uiUzi].Load(TEXT("resource/ui_uzi.png"));
			sprites[(int)SPRITE::uiShotgun].Load(TEXT("resource/ui_shotgun.png"));
			sprites[(int)SPRITE::uiBox].Load(TEXT("resource/ui_box.png"));
			sprites[(int)SPRITE::uiPotion].Load(TEXT("resource/ui_healpack.png"));

			sprites[(int)SPRITE::itemBox].Load(TEXT("resource/itembox.png"));

			sprites[(int)SPRITE::bulletN].Load(TEXT("resource/b0.png"));
			sprites[(int)SPRITE::bulletNE].Load(TEXT("resource/b1.png"));
			sprites[(int)SPRITE::bulletE].Load(TEXT("resource/b2.png"));
			sprites[(int)SPRITE::bulletSE].Load(TEXT("resource/b3.png"));
			sprites[(int)SPRITE::bulletS].Load(TEXT("resource/b4.png"));
			sprites[(int)SPRITE::bulletSW].Load(TEXT("resource/b5.png"));
			sprites[(int)SPRITE::bulletW].Load(TEXT("resource/b6.png"));
			sprites[(int)SPRITE::bulletNW].Load(TEXT("resource/b7.png"));

			sprites[(int)SPRITE::wallCol].Load(TEXT("resource/wall_col.png"));
			sprites[(int)SPRITE::wallRow].Load(TEXT("resource/wall_row.png"));

			sprites[(int)SPRITE::bgWinner].Load(TEXT("resource/ui_you_win.png"));
			sprites[(int)SPRITE::bgGameover].Load(TEXT("resource/ui_game_over.png"));
		}

		/*********************************************�̹��� �ε�*****************************************************/
		gameObject.reserve(MAX_OBJECT);
		for (int i = 0; i < 3; ++i)
		{
			Player* player = new Player;
			gameObject.push_back(player);
		}
		for (int i = 3; i < MAX_OBJECT; ++i)
		{
			GameObject* obj = new GameObject;
			gameObject.push_back(obj);
		}

		//gameObject[99]->test();
		/////////////////////////////////////////////////////////////////////////////////////////
	}
	break;

	case WM_GETMINMAXINFO:
	{
		((MINMAXINFO*)lParam)->ptMaxTrackSize.x = win_x_size + 14;
		((MINMAXINFO*)lParam)->ptMaxTrackSize.y = win_y_size + 36;
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = win_x_size + 14;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = win_y_size + 36;
		return false;
	}
	break;

	case WM_PAINT: // ������
	{
		hdc = BeginPaint(hWnd, &ps);

		if (scene == SCENE::title)
		{
			dc.Create(win_x_size, win_y_size, 24);	// ��� ���� DC
			memdc1 = dc.GetDC();					// ��� ���� DC
			sprites[(int)SPRITE::bgTitle].Draw(memdc1, 0, 0, win_x_size, win_y_size);
			sprites[(int)SPRITE::btnPlay].Draw(memdc1, play_button_rect);
			sprites[(int)SPRITE::btnExit].Draw(memdc1, exit_button_rect);

			// ���ÿ� ��������Ʈ
			sprites[selPlayer].Draw(memdc1, 680, 100, character_width * 2, character_height * 2,
				char_move_sprite_rect[(int)DIR::S][selAnimation].left, char_move_sprite_rect[(int)DIR::S][selAnimation].top, 18, 30);

			dc.Draw(hdc, 0, 0, win_x_size, win_y_size);	// �Ʒ��� Bitblt�� ����
			dc.ReleaseDC();		// dc ����
			dc.Destroy();		// ��� dc ����
		}

		else if (scene == SCENE::lobby)
		{
			while (!isLoginOk);
			dc.Create(win_x_size * 2, win_y_size * 2, 24);	// == CreateCompatibleBitmap
			memdc1 = dc.GetDC();							// == CreateComaptibleDC

			sprites[(int)SPRITE::bgLobby].BitBlt(memdc1, 0, 0, SRCCOPY); //���

			////////////////////////////////////////// Render ///////////////////////////////////////////////

			for (const auto& gameobj : gameObject)
			{
				gameobj->Render(memdc1);
			}

			////////////////////////////////////////// ȭ�� ���� ///////////////////////////////////////////////
			// UI �������̵鸸 memdc2�� �����

			Coordinate playerPos = gameObject[playerID]->GetPos();	// �÷��̾� ��ǥ

			dc2.Create(win_x_size, win_y_size, 24);		// ȭ�� ��¿� DC
			memdc2 = dc2.GetDC();						// ȭ�� ��¿� DC

			if (playerPos.x <= (win_x_size / 2))
			{
				play_size_left = 0;
			}
			else if (playerPos.x >= (win_x_size * 2) - (win_x_size / 2))
			{
				play_size_left = (win_x_size * 2) - win_x_size;
			}
			else
			{
				play_size_left = playerPos.x - (win_x_size / 2);
			}

			if (playerPos.y <= (win_y_size / 2))
			{
				play_size_top = 0;
			}
			else if (playerPos.y >= (win_y_size * 2) - (win_y_size / 2))
			{
				play_size_top = (win_y_size * 2) - win_y_size;
			}
			else
			{
				play_size_top = playerPos.y - (win_y_size / 2);
			}

			dc.BitBlt(memdc2, 0, 0, win_x_size, win_y_size, play_size_left, play_size_top);

			////////////////////////////////////////// UI ///////////////////////////////////////////////

			TCHAR bullet_num[5] = {};   //�Ѿ˰���
			TCHAR weapon_num[2] = {};   //�����ȣ

			hbrush = CreateSolidBrush(RGB(255, 255, 255));
			oldbrush = (HBRUSH)SelectObject(memdc2, hbrush);

			Player* p = reinterpret_cast<Player*>(gameObject[playerID]);

			for (int i = 0; i < 5; ++i)
			{
				RECT print_rect = weapon_image_rect[i];
				wsprintf(bullet_num, TEXT("%d"), p->items[i]);
				wsprintf(weapon_num, TEXT("%d"), i + 1);

				if (selectedWeapon - 1 == i)  //���õ� ����� 25�ȼ� ���� ���
				{
					print_rect.top -= 25;
					print_rect.bottom -= 25;
				}
				RoundRect(memdc2, print_rect.left, print_rect.top, print_rect.right, print_rect.bottom, 10, 10);

				sprites[(int)SPRITE::uiPistol + i].Draw(memdc2, print_rect);

				SetBkMode(memdc2, TRANSPARENT);  //������

				print_rect.top -= 20;
				print_rect.bottom -= 20;
				DrawText(memdc2, bullet_num, _tcslen(bullet_num), &print_rect, DT_CENTER | DT_VCENTER);

				print_rect.top += 70;
				print_rect.bottom += 70;
				DrawText(memdc2, weapon_num, _tcslen(weapon_num), &print_rect, DT_CENTER | DT_VCENTER);
			}
			SelectObject(memdc2, oldbrush);
			DeleteObject(hbrush);

			///////////////////////////////////////////////////////////////////////////////////////////

			dc2.Draw(hdc, 0, 0);

			dc.ReleaseDC();		// dc ����
			dc.Destroy();		// ��� dc ����
			dc2.ReleaseDC();
			dc2.Destroy();
		}

		else if (scene == SCENE::stage1)
		{
			dc.Create(win_x_size * 2, win_y_size * 2, 24);	// == CreateCompatibleBitmap
			memdc1 = dc.GetDC();							// == CreateComaptibleDC

			sprites[(int)SPRITE::bgStage1].BitBlt(memdc1, 0, 0, SRCCOPY); //���

			////////////////////////////////////////// Render ///////////////////////////////////////////////

			for (const auto& gameobj : gameObject)
			{
				gameobj->Render(memdc1);
			}

			////////////////////////////////////////// ȭ�� ���� ///////////////////////////////////////////////
			// UI �������̵鸸 memdc2�� �����

			Coordinate playerPos = gameObject[playerID]->GetPos();	// �÷��̾� ��ǥ

			dc2.Create(win_x_size, win_y_size, 24);		// ȭ�� ��¿� DC
			memdc2 = dc2.GetDC();						// ȭ�� ��¿� DC

			if (playerPos.x <= (win_x_size / 2))
			{
				play_size_left = 0;
			}
			else if (playerPos.x >= (win_x_size * 2) - (win_x_size / 2))
			{
				play_size_left = (win_x_size * 2) - win_x_size;
			}
			else
			{
				play_size_left = playerPos.x - (win_x_size / 2);
			}

			if (playerPos.y <= (win_y_size / 2))
			{
				play_size_top = 0;
			}
			else if (playerPos.y >= (win_y_size * 2) - (win_y_size / 2))
			{
				play_size_top = (win_y_size * 2) - win_y_size;
			}
			else
			{
				play_size_top = playerPos.y - (win_y_size / 2);
			}

			dc.BitBlt(memdc2, 0, 0, win_x_size, win_y_size, play_size_left, play_size_top);

			////////////////////////////////////////// UI ///////////////////////////////////////////////

			TCHAR bullet_num[5] = {};   //�Ѿ˰���
			TCHAR weapon_num[2] = {};   //�����ȣ

			hbrush = CreateSolidBrush(RGB(255, 255, 255));
			oldbrush = (HBRUSH)SelectObject(memdc2, hbrush);

			Player* p = reinterpret_cast<Player*>(gameObject[playerID]);

			for (int i = 0; i < 5; ++i)
			{
				RECT print_rect = weapon_image_rect[i];
				wsprintf(bullet_num, TEXT("%d"), p->items[i]);
				wsprintf(weapon_num, TEXT("%d"), i + 1);

				if (selectedWeapon - 1 == i)  //���õ� ����� 25�ȼ� ���� ���
				{
					print_rect.top -= 25;
					print_rect.bottom -= 25;
				}
				RoundRect(memdc2, print_rect.left, print_rect.top, print_rect.right, print_rect.bottom, 10, 10);

				sprites[(int)SPRITE::uiPistol + i].Draw(memdc2, print_rect);

				SetBkMode(memdc2, TRANSPARENT);  //������

				print_rect.top -= 20;
				print_rect.bottom -= 20;
				DrawText(memdc2, bullet_num, _tcslen(bullet_num), &print_rect, DT_CENTER | DT_VCENTER);

				print_rect.top += 70;
				print_rect.bottom += 70;
				DrawText(memdc2, weapon_num, _tcslen(weapon_num), &print_rect, DT_CENTER | DT_VCENTER);
			}
			SelectObject(memdc2, oldbrush);
			DeleteObject(hbrush);

			///////////////////////////////////////////////////////////////////////////////////////////

			dc2.Draw(hdc, 0, 0);

			dc.ReleaseDC();		// dc ����
			dc.Destroy();		// ��� dc ����
			dc2.ReleaseDC();
			dc2.Destroy();

		}

		else if (scene == SCENE::gameover)
		{

			dc.Create(win_x_size, win_y_size, 24);
			memdc1 = dc.GetDC();
			sprites[(int)SPRITE::bgEnd].Draw(memdc1, 0, 0, win_x_size, win_y_size);
			sprites[(int)SPRITE::bgGameover].Draw(memdc1, GAMEOVER_rect);
			sprites[(int)SPRITE::btnReplay].Draw(memdc1, replay_button_rect);	//replay��ư
			sprites[(int)SPRITE::btnExit].Draw(memdc1, exit2_button_rect);		//exit��ư

			dc.Draw(hdc, 0, 0, win_x_size, win_y_size);

			dc.ReleaseDC();		// dc ����
			dc.Destroy();		// ��� dc ����
		}

		else if (scene == SCENE::winner)
		{
			dc.Create(win_x_size, win_y_size, 24);
			memdc1 = dc.GetDC();
			sprites[(int)SPRITE::bgEnd].Draw(memdc1, 0, 0, win_x_size, win_y_size);
			sprites[(int)SPRITE::bgWinner].Draw(memdc1, GAMEOVER_rect);
			sprites[(int)SPRITE::btnReplay].Draw(memdc1, replay_button_rect);	//replay��ư
			sprites[(int)SPRITE::btnExit].Draw(memdc1, exit2_button_rect);		//exit��ư

			dc.Draw(hdc, 0, 0, win_x_size, win_y_size);

			dc.ReleaseDC();		// dc ����
			dc.Destroy();		// ��� dc ����
		}


		EndPaint(hWnd, &ps);
	}
	break;

	case WM_LBUTTONDOWN:
	{
		MOUSE_X = LOWORD(lParam);
		MOUSE_Y = HIWORD(lParam);

		if (scene == SCENE::title)
		{
			if ((play_button_rect.left <= MOUSE_X) && (play_button_rect.top <= MOUSE_Y) && (MOUSE_X <= play_button_rect.right) && (MOUSE_Y <= play_button_rect.bottom))  //play��ư ����, ������ ���� �۵�
			{
				play_button = true;
			}
			else if ((exit_button_rect.left <= MOUSE_X) && (exit_button_rect.top <= MOUSE_Y) && (MOUSE_X <= exit_button_rect.right) && (MOUSE_Y <= exit_button_rect.bottom)) //exit��ư ����, ������ ���� �۵�
			{
				exit_button = true;
			}
		}
		else if (scene == SCENE::gameover || scene == SCENE::winner)
		{
			if ((replay_button_rect.left <= MOUSE_X) && (replay_button_rect.top <= MOUSE_Y) && (MOUSE_X <= replay_button_rect.right) && (MOUSE_Y <= replay_button_rect.bottom))  //replay��ư ����, ������ ���� �۵�
			{
				replay_button = true;
			}
			else if ((exit2_button_rect.left <= MOUSE_X) && (exit2_button_rect.top <= MOUSE_Y) && (MOUSE_X <= exit2_button_rect.right) && (MOUSE_Y <= exit2_button_rect.bottom)) //exit2��ư ����, ������ ���� �۵�
			{
				exit2_button = true;
			}
		}
	}
	break;

	case WM_LBUTTONUP:
	{
		if (scene == SCENE::title)
		{
			if (play_button == true)
			{
				if (isAlreadyConnect == false)
					ConnectServer();
				else { // �ʱ�ȭ
					Player* p = reinterpret_cast<Player*>(gameObject[playerID]);
					p->animFrame = 2;
					p->animTimer = 0;
					for (int i = 0; i < 8; ++i)
						p->items[i] = 0;

					keyAction.space = false;
					keyAction.up = false;
					keyAction.down = false;
					keyAction.left = false;
					keyAction.right = false;
					keyAction.reqSend = false;
				}
				cs_packet_login packet;
				packet.packetSize = sizeof(cs_packet_login);
				packet.packetType = CS_PACKET_LOGIN;
				packet.playerSkin = (char)selPlayer;
				Send(&packet);

				play_button = false;

				std::cout << "Push Play BTN" << std::endl;
			}
			else if (exit_button == true)
			{
				exit_button = false;
				PostQuitMessage(0);          //���α׷� ����
				break;
			}
		}
		else if (scene == SCENE::gameover || scene == SCENE::winner)
		{
			if (replay_button == true)
			{
				scene = SCENE::title;
				isLoginOk = false;
			}
			else if (exit2_button == true)
			{
				exit_button = false;
				PostQuitMessage(0);          //���α׷� ����
				break;
			}
		}
	}
	break;

	case WM_MOUSEMOVE:
	{
	}
	break;

	case WM_KEYDOWN:
	{
		if (scene == SCENE::title)
		{
			switch (wParam)
			{
			case '1':
				selPlayer = (int)SPRITE::Izuna;
				break;

			case '2':
				selPlayer = (int)SPRITE::GenAn;
				break;

			case '3':
				selPlayer = (int)SPRITE::Hinagiku;
				break;

			case '4':
				selPlayer = (int)SPRITE::Ichika;
				break;

			case '5':
				selPlayer = (int)SPRITE::Kagen;
				break;

			case '6':
				selPlayer = (int)SPRITE::Mitsumoto;
				break;

			case '7':
				selPlayer = (int)SPRITE::Shino;
				break;

			case '8':
				selPlayer = (int)SPRITE::Sizune;
				break;
			}
		}

		else if (scene == SCENE::gameover || scene == SCENE::winner)
		{

		}

		// lobby / stages input
		else
		{
			Player* p = reinterpret_cast<Player*>(gameObject[playerID]);

			switch (wParam)
			{
			case VK_LEFT:
				if (keyAction.left == false)
				{
					keyAction.left = true;
					keyAction.reqSend = true;
				}
				break;

			case VK_RIGHT:
				if (keyAction.right == false)
				{
					keyAction.right = true;
					keyAction.reqSend = true;
				}
				break;

			case VK_UP:
				if (keyAction.up == false)
				{
					keyAction.up = true;
					keyAction.reqSend = true;
				}
				break;

			case VK_DOWN:
				if (keyAction.down == false)
				{
					keyAction.down = true;
					keyAction.reqSend = true;
				}
				break;

			case VK_SPACE:
				keyAction.space = true;
				keyAction.reqSend = true;
				break;
			}
		}
	}
	break;

	case WM_KEYUP:
	{
		if (scene == SCENE::title)
		{

		}

		else if (scene == SCENE::gameover || scene == SCENE::winner)
		{

		}

		else
		{
			Player* p = reinterpret_cast<Player*>(gameObject[playerID]);
			switch (wParam)
			{
			case VK_ESCAPE:
				std::cout << "���� ��ǥ: (" << p->GetPos().x << ", " << p->GetPos().y << ")" << std::endl;
				//PostQuitMessage(0);
				break;

			case VK_LEFT:
				keyAction.left = false;
				keyAction.reqSend = true;
				break;

			case VK_RIGHT:
				keyAction.right = false;
				keyAction.reqSend = true;
				break;

			case VK_UP:
				keyAction.up = false;
				keyAction.reqSend = true;
				break;

			case VK_DOWN:
				keyAction.down = false;
				keyAction.reqSend = true;
				break;

			case VK_SPACE:
				keyAction.space = false;
				keyAction.reqSend = true;
				break;

			case '1':
			case '2':
			case '3':
				cs_packet_used_item sendPacket;
				sendPacket.packetSize = sizeof(sendPacket);
				sendPacket.packetType = CS_PACKET_USED_ITEM;
				sendPacket.itemNum = static_cast<int>((wParam - '0'));
				Send(&sendPacket);
			case '4':
			case '5':
				int key = static_cast<int>((wParam - '0'));
				selectedWeapon = key;
				p->SetWeapon(key);
				break;
			}
		}
	}
	break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void ConnectServer()
{
	int retval;

	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return;

	//socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket");

	//connect
	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(ServerAddr));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_addr.s_addr = inet_addr(SERVERIP);
	ServerAddr.sin_port = htons(SERVERPORT);

	// ���� �����ϸ� �翬���ϵ��� �ٲ���
	retval = connect(sock, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	HANDLE hThread;
	hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)sock, 0, NULL);

	if (hThread == NULL)closesocket(sock);

	isAlreadyConnect = true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Send(void* Packet)
{
	int retval = send(sock, reinterpret_cast<char*>(Packet), reinterpret_cast<packet*>(Packet)->packetSize, 0);
	std::cout << "[TCP ����] " << (int)reinterpret_cast<packet*>(Packet)->packetType << "\n";
}

void Recv(SOCKET sock) {

	packet pkSize;
	int retval = recv(sock, reinterpret_cast<char*>(&pkSize), sizeof(packet), MSG_WAITALL);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		return;
	}

	//	SC_PACKET_LOGIN_OK = 1;
	//	SC_PACKET_CHANGE_SCENE = 2;
	//	SC_PACKET_MOVE_OBJ = 3;
	//	SC_PACKET_PLAYER_STATE = 4;
	//	SC_PACKET_PUT_OBJ = 5;
	//	SC_PACKET_REMOVE_OBJ = 6;
	//	SC_PACKET_CHANGE_HP = 7;
	//	SC_PACKET_GET_ITEM = 8;
	//	SC_PACKET_ITEM_COUNT = 9;
	//	SC_PACKET_CHAGE_WEAPON = 10;

	switch (pkSize.packetType)
	{
	case SC_PACKET_LOGIN_OK:
	{
		sc_packet_login_ok recvPacket;
		retval += recv(sock, reinterpret_cast<char*>(&recvPacket) + 2, pkSize.packetSize - 2, MSG_WAITALL);
		playerID = (int)recvPacket.playerID;
		gameObject[playerID]->LoginOk(&recvPacket);
		isLoginOk = true;
		scene = SCENE::lobby;

		std::cout << "Recv LoginOK" << std::endl;
	}
	break;
	case SC_PACKET_CHANGE_SCENE:
	{
		sc_packet_change_scene recvPacket;
		retval += recv(sock, reinterpret_cast<char*>(&recvPacket) + 2, pkSize.packetSize - 2, MSG_WAITALL);
		scene = (SCENE)recvPacket.sceneNum;
	}
	break;
	case SC_PACKET_MOVE_OBJ:
	{
		sc_packet_move_obj recvPacket;
		retval += recv(sock, reinterpret_cast<char*>(&recvPacket) + 2, pkSize.packetSize - 2, MSG_WAITALL);

		gameObject[(int)recvPacket.objectID]->ObjMove(&recvPacket);
	}
	break;
	case SC_PACKET_PLAYER_STATE:
	{
		sc_packet_player_state recvPacket;
		retval += recv(sock, reinterpret_cast<char*>(&recvPacket) + 2, pkSize.packetSize - 2, MSG_WAITALL);
		Player* player = reinterpret_cast<Player*>(gameObject[(int)recvPacket.objectID]);

		player->PlayerState(&recvPacket);
	}
	break;
	case SC_PACKET_PUT_OBJ:
	{
		sc_packet_put_obj recvPacket;
		retval += recv(sock, reinterpret_cast<char*>(&recvPacket) + 2, pkSize.packetSize - 2, MSG_WAITALL);

		gameObject[(int)recvPacket.objectID]->PutObj(&recvPacket);
	}
	break;

	case SC_PACKET_REMOVE_OBJ:
	{
		sc_packet_remove_obj recvPacket;
		retval += recv(sock, reinterpret_cast<char*>(&recvPacket) + 2, pkSize.packetSize - 2, MSG_WAITALL);

		gameObject[(int)recvPacket.objectID]->RemoveObj();
	}
	break;
	case SC_PACKET_CHANGE_HP:
	{
		sc_packet_change_hp recvPacket;
		retval += recv(sock, reinterpret_cast<char*>(&recvPacket) + 2, pkSize.packetSize - 2, MSG_WAITALL);
		Player* player = reinterpret_cast<Player*>(gameObject[(int)recvPacket.playerID]);

		player->ChangeHp(&recvPacket);
	}
	break;
	case SC_PACKET_GET_ITEM:
	{
		sc_packet_get_item recvPacket;
		retval += recv(sock, reinterpret_cast<char*>(&recvPacket) + 2, pkSize.packetSize - 2, MSG_WAITALL);
		Player* player = reinterpret_cast<Player*>(gameObject[(int)recvPacket.playerID]);

		player->GetItem(&recvPacket);
	}
	break;
	//case SC_PACKET_ITEM_COUNT:
	//{
	//	sc_packet_get_item recvPacket;
	//	retval += recv(sock, reinterpret_cast<char*>(&recvPacket) + 2, pkSize.packetSize - 2, MSG_WAITALL);
	//	Player* player = reinterpret_cast<Player*>(gameObject[(int)recvPacket.playerID]);

	//	player->ItemCount();
	//}
	break;
	case SC_PACKET_CHAGE_WEAPON:
	{
		sc_packet_get_item recvPacket;
		retval += recv(sock, reinterpret_cast<char*>(&recvPacket) + 2, pkSize.packetSize - 2, MSG_WAITALL);
		Player* player = reinterpret_cast<Player*>(gameObject[(int)recvPacket.playerID]);

		player->ChangeWeapon(&recvPacket);

		if ((int)recvPacket.playerID == playerID)
		{
			selectedWeapon = player->GetcurGun();
		}
	}
	break;
	default:
		char tmp[20];
		retval += recv(sock, tmp, pkSize.packetSize - 2, MSG_WAITALL);
		break;
	}
}

DWORD WINAPI ProcessClient(LPVOID arg)
{
	//SOCKET sock = reinterpret_cast<SOCKET>(arg);
	//������ ������ ���
	int len;

	while (1) {

		Recv(sock);
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void SendStatePacket(STATE state)
{
	Player* p = reinterpret_cast<Player*>(gameObject[playerID]);
	cs_packet_player_state sendPacket;
	sendPacket.packetSize = sizeof(sendPacket);
	sendPacket.packetType = CS_PACKET_PLAYER_STATE;
	sendPacket.playerState = (char)state;
	Send(&sendPacket);
}