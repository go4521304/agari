#include "Game.h"

#define SERVERIP	"127.0.0.1"
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
	srand((unsigned int)time(NULL));

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
		/*********************************************�̹��� �ε�*****************************************************/
		sprites[(int)SPRITE::btnPlay].Load(TEXT("resource/PLAY.png"));
		sprites[(int)SPRITE::btnExit].Load(TEXT("resource/EXIT.png"));
		sprites[(int)SPRITE::btnReplay].Load(TEXT("resource/REPLAY.png"));
		sprites[(int)SPRITE::bgTitle].Load(TEXT("resource/startBack.png"));
		sprites[(int)SPRITE::bgStage1].Load(TEXT("resource/stage.png"));
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
		sprites[(int)SPRITE::uiPotion].Load(TEXT("resource/ui_box.png"));	// uiposition �̹��� ��������

		sprites[(int)SPRITE::itemBox].Load(TEXT("resource/itembox.png"));

		/*********************************************�̹��� �ε�*****************************************************/

		for (int i = 0; i < 4; ++i)
		{
			Player* player = new Player;
			gameObject.push_back((GameObject*)player);
		}
		for (int i = 0; i < 100; ++i)
		{
			GameObject* obj = new GameObject;
			gameObject.push_back(obj);
		}

		
		gameObject[5]->test();	////////////////////////// �ӽ�

		////////////////////////////////////////// winsock ///////////////////////////////////////////////

		int retval;

		WSADATA wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
			return 1;

		//socket
		SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
		if (sock == INVALID_SOCKET) err_quit("socket");

		//connect

		SOCKADDR_IN ServerAddr;
		ZeroMemory(&ServerAddr, sizeof(ServerAddr));
		ServerAddr.sin_family = AF_INET;
		ServerAddr.sin_addr.s_addr = inet_addr(SERVERIP);
		ServerAddr.sin_port = htons(SERVERPORT);
		retval = connect(sock, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr));
		if (retval == SOCKET_ERROR) err_quit("connect()");

		HANDLE hThread;


		hThread = CreateThread(NULL, 0, ProcessClient, (LPVOID)sock, 0, NULL);

		if (hThread == NULL)closesocket(sock);

		/////////////////////////////////////////////////////////////////////////////////////////

		TIMER = GetTickCount64();
		SetTimer(hWnd, 0, 10, (TIMERPROC)TimerProc);	// updateLoop
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
			sprites[playerSel].Draw(memdc1, 680, 100, character_width * 2, character_height * 2,
				char_move_sprite_rect[(int)DIR::S][selAnimation].left, char_move_sprite_rect[(int)DIR::S][selAnimation].top, 18, 30);

			dc.Draw(hdc, 0, 0, win_x_size, win_y_size);	// �Ʒ��� Bitblt�� ����
			dc.ReleaseDC();		// dc ����
			dc.Destroy();		// ��� dc ����
		}

		else if (scene == SCENE::lobby)
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

			int tempX = 900, tempY = 800;	////////////////////////// �ӽ�

			dc2.Create(win_x_size, win_y_size, 24);		// ȭ�� ��¿� DC
			memdc2 = dc2.GetDC();						// ȭ�� ��¿� DC

			if (tempX <= (win_x_size / 2))
			{
				play_size_left = 0;
			}
			else if (tempX >= (win_x_size * 2) - (win_x_size / 2))
			{
				play_size_left = (win_x_size * 2) - win_x_size;
			}
			else
			{
				play_size_left = tempX - (win_x_size / 2);
			}

			if (tempY <= (win_y_size / 2))
			{
				play_size_top = 0;
			}
			else if (tempY >= (win_y_size * 2) - (win_y_size / 2))
			{
				play_size_top = (win_y_size * 2) - win_y_size;
			}
			else
			{
				play_size_top = tempY - (win_y_size / 2);
			}

			dc.BitBlt(memdc2, 0, 0, win_x_size, win_y_size, play_size_left, play_size_top);

			////////////////////////////////////////// UI ///////////////////////////////////////////////

			TCHAR bullet_num[5] = {};   //�Ѿ˰���
			TCHAR weapon_num[2] = {};   //�����ȣ

			hbrush = CreateSolidBrush(RGB(255, 255, 255));
			oldbrush = (HBRUSH)SelectObject(memdc2, hbrush);
			for (int i = 0; i < 5; ++i)
			{
				RECT print_rect = weapon_image_rect[i];		// ���� �޾ƿü� �ֵ��� �����ؾ���
				wsprintf(bullet_num, TEXT("%d"), 0);
				wsprintf(weapon_num, TEXT("%d"), i + 1);

				//if (selected_weapon == i)  //���õ� ����� 25�ȼ� ���� ���
				//{
				//	print_rect.top -= 25;
				//	print_rect.bottom -= 25;
				//}
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

		}

		else if (scene == SCENE::gameover)
		{

			dc.Create(win_x_size, win_y_size, 24);
			memdc1 = dc.GetDC();
			sprites[(int)SPRITE::bgEnd].Draw(memdc1, 0, 0, win_x_size, win_y_size);

			
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
		else if (scene == SCENE::gameover)
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
				scene = SCENE::lobby;
				play_button = false;
				InvalidateRect(hWnd, NULL, false);
			}
			else if (exit_button == true)
			{
				exit_button = false;
				PostQuitMessage(0);          //���α׷� ����
				break;
			}
		}
		else if (scene == SCENE::gameover)
		{
			if (replay_button == true)
			{
				scene = SCENE::title;
				replay_button = false;

				InvalidateRect(hWnd, NULL, false);
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
				playerSel = (int)SPRITE::Izuna;
				break;

			case '2':
				playerSel = (int)SPRITE::GenAn;
				break;

			case '3':
				playerSel = (int)SPRITE::Hinagiku;
				break;

			case '4':
				playerSel = (int)SPRITE::Ichika;
				break;

			case '5':
				playerSel = (int)SPRITE::Kagen;
				break;

			case '6':
				playerSel = (int)SPRITE::Mitsumoto;
				break;

			case '7':
				playerSel = (int)SPRITE::Shino;
				break;

			case '8':
				playerSel = (int)SPRITE::Sizune;
				break;
			}
		}
	}
	break;

	case WM_KEYUP:
	{
	}
	break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CALLBACK TimerProc(HWND hWnd, UINT uMSG, UINT idEvent, DWORD dwTime)
{
	// idEvent�� �޼����� ������� �� �ٵ� �Ⱦ� ����
	if (scene == SCENE::title)
	{
		selTimer -= GetTickCount64() - TIMER;
		if (selTimer < 0)
		{
			selAnimation++;
			selAnimation %= 4;
			selTimer = 100;
		}
	}

	TIMER = GetTickCount64();
	InvalidateRect(hWnd, NULL, false);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GameObject::LoginOk(void* pk)
{
	sc_packet_login_ok* recvPacket = (sc_packet_login_ok*)pk;

	isActive = true;
	pos.x = recvPacket->x;
	pos.y = recvPacket->y;
}

void GameObject::ObjMove(void* pk)
{
	sc_packet_obj_move* recvPacket = (sc_packet_obj_move*)pk;

	direction = (DIR)recvPacket->lookDir;
	pos.x = (int)recvPacket->x;
	pos.y = (int)recvPacket->y;
}

void GameObject::PutObj(void* pk)
{
	sc_packet_put_obj* recvPacket = (sc_packet_put_obj*)pk;

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
	if (isActive)
		sprites[sprite].Draw(hdc, pos.x - (width / 2), pos.y + (height / 2), width, height);

	// ����, �Ѿ��� ���� ����� �� �ΰ�?
	// 1�� �Ѿ��� ���⺰�� �����ؼ� ������ �״�� ��� => �̰ŷ� �Ҳ�
	// 2�� �Ѿ� �̹����� �ѹ��� ��� �����ؼ� ���� ���⿡ ���缭 ó�� => ���
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Player::PlayerState(void* pk)
{

}

void Player::ChangeWeapon(void* pk)
{

}

void Player::ChangeHp(void* pk)
{
	
}

void Player::GetItem(void* pk)
{
	
}

void Player::ItemCount(void* pk)
{
	
}

void Player::UseItem(int index)
{
	
}

void Player::Render(HDC& hdc)
{

}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Recv(SOCKET sock) {

	packet pkSize;
	int retval = recv(sock, reinterpret_cast<char*>(&pkSize), sizeof(packet), MSG_WAITALL);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		return;
	}

	switch (pkSize.packetType)
	{
	case SC_PACKET_LOGIN_OK:
	{
		sc_packet_login_ok recvPacket;
		retval += recv(sock, reinterpret_cast<char*>((&recvPacket + 2)), pkSize.packetSize - 2, MSG_WAITALL);

		playerID = (int)recvPacket.playerID;
		gameObject[playerID]->LoginOk(&recvPacket);
	}
	break;
	case SC_PACKET_PUT_OBJ:
	{
		sc_packet_put_obj recvPacket;
		retval += recv(sock, reinterpret_cast<char*>((&recvPacket + 2)), pkSize.packetSize - 2, MSG_WAITALL);

		gameObject[(int)recvPacket.objectID]->PutObj(&recvPacket);
	}
	break;
	case SC_PACKET_OBJ_MOVE:
	{
		sc_packet_obj_move recvPacket;
		retval += recv(sock, reinterpret_cast<char*>((&recvPacket + 2)), pkSize.packetSize - 2, MSG_WAITALL);

		gameObject[(int)recvPacket.objectID]->PutObj(&recvPacket);
	}
	break;
	case SC_PACKET_REMOVE_OBJ:
	{
		sc_packet_remove_obj recvPacket;
		retval += recv(sock, reinterpret_cast<char*>((&recvPacket + 2)), pkSize.packetSize - 2, MSG_WAITALL);

		gameObject[(int)recvPacket.objectID]->RemoveObj();
	}
	break;
	default:
		break;
	}
}

DWORD WINAPI ProcessClient(LPVOID arg)
{
	SOCKET sock = reinterpret_cast<SOCKET>(arg);
	//������ ������ ���
	int len;

	while (1) {

		Recv(sock);
	}
	return 0;
}
 
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GameObject::test()
{
	isActive = true;
	width = 30;
	height = 40;
	sprite = (int)SPRITE::box;
	pos.x = 800;
	pos.y = 900;
}

void Player::test()
{

}