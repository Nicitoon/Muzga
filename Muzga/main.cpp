#include "Windows.h"
#include "vector"
#pragma comment(lib, "msimg32.lib")
#include "math.h"

//стурктура где храняться данные о windows окне
struct
{
	//дескрипторы, контейнеры и буфферы для windows
	RECT rc;
	HINSTANCE hIns;
	HWND hWnd;
	HDC dev_cont, contx;
	MSG msg;
	BOOL gbool = true;

	//определяет размер экрана в вашей сиситеме
	int width = GetSystemMetrics(SM_CXSCREEN) - 200, height = GetSystemMetrics(SM_CYSCREEN) - 200;
} window;

struct Block
{
	int x;
	int y;
	int width;
	int height;
};
struct Bullet
{
	int x;
	int y;
	int width;
	int height;
	bool create = false;
} bullet;
std::vector<Block> blocks;
struct Enemy
{
	int x = 500;
	int y = 700;
	int Height = 100;
	int Width = 100;
	int radius = 300;
	bool collisionis = false;
	int grav = 10;
	int center;
} enemy;

struct Player
{
	int PlayerX = 0;
	int PlayerY = 0;
	int PleyerHeight = 195;
	int PleyerWidth = 131;
	int jumpis = 1;
	int jumpPower = 0;
	int grav = 10;
	std::vector <HBITMAP> Images{
		{(HBITMAP)LoadImageA(NULL, "back.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE)},




	};

} player;

bool CheckCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
	return
		x1      < x2 + w2 &&
		x1 + w1 > x2 &&
		y1      < y2 + h2 &&
		y1 + h1 > y2;
}
bool CheckCollisionEnemy(int x1, int y1, int w1, int h1, int x2, int y2, int rad)
{
	return
		x1      < x2 + rad &&
		x1 + w1 > x2 - rad &&
		y1      < y2 + rad &&
		y1 + h1 > y2 - rad;
}

void CheckPlayerCollisions()
{
	for (int i = 0; i < blocks.size(); i++)
	{
		if (CheckCollision(
			player.PlayerX, player.PlayerY,
			player.PleyerWidth, player.PleyerHeight,
			blocks[i].x, blocks[i].y,
			blocks[i].width, blocks[i].height))
		{
			int overlapLeft = (player.PlayerX + player.PleyerWidth) - blocks[i].x;
			int overlapRight = (blocks[i].x + blocks[i].width) - player.PlayerX;
			int overlapTop = (player.PlayerY + player.PleyerHeight) - blocks[i].y;
			int overlapBottom = (blocks[i].y + blocks[i].height) - player.PlayerY;

			int minX = min(overlapLeft, overlapRight);
			int minY = min(overlapTop, overlapBottom);

			if (minX < minY)
			{
				if (overlapLeft < overlapRight)
					player.PlayerX = blocks[i].x - player.PleyerWidth;
				else
					player.PlayerX = blocks[i].x + blocks[i].width;
			}
			else
			{
				if (overlapTop < overlapBottom)
				{
					player.PlayerY = blocks[i].y - player.PleyerHeight;
					player.jumpis = 0;
					player.jumpPower = 0;
				}
				else
				{
					player.PlayerY = blocks[i].y + blocks[i].height;
					player.jumpPower = 0;
				}
			}
		}
	}
}

void EnemyAttention()
{
	enemy.center = enemy.x + enemy.Width / 2;
	if (CheckCollisionEnemy(
		player.PlayerX, player.PlayerY,
		player.PleyerWidth, player.PleyerHeight,
		enemy.center, enemy.y,
		enemy.radius))
	{
		if (player.PlayerX < enemy.x)
		{
			enemy.x -= 5;
		}
		else
		{
			enemy.x += 5;
		}

	}

}
void EnemyGravity()
{

	if (enemy.y + enemy.Height < window.height)
	{
		enemy.y += enemy.grav;

	}
	else
	{
		enemy.y = window.height - enemy.Height;
		
	}

}


void Gravity()
{

	if (player.PlayerY + player.PleyerHeight < window.height)
	{
		player.PlayerY += player.grav;

	}
	else
	{
		player.PlayerY = window.height - player.PleyerHeight;
		player.jumpis = 0;
	}
	if (player.jumpPower > 0)
	{
		player.grav = 0;
	}
	else
	{
		player.grav = 10;
	}


}
void BulletMovement()
{
	if (bullet.create)
	{
		bullet.x += 30;
	}
}

void PlayerMovement()
{
	if (GetAsyncKeyState('A'))
	{
		player.PlayerX -= 10;
	}
	if (GetAsyncKeyState('D'))
	{
		player.PlayerX += 10;
	}
	if (GetAsyncKeyState(VK_SPACE))
	{
		if (player.jumpis == 0)
		{
			player.jumpPower += 40;
			player.jumpis = 1;
		}


	}
	if (player.jumpPower > 0)
	{
		player.PlayerY -= 23;
		player.jumpPower -= 6;
	}
	if (GetAsyncKeyState('F'))
	{

		bullet.x = player.PlayerX;
		bullet.y = player.PlayerY + player.PleyerHeight / 2;
		bullet.create = true;
		
	}
}




//обработка потока сообщений
static LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	switch (msg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// Убрали весь код отсюда - рисование идёт через UpdateImage
		EndPaint(hWnd, &ps);
		return 0;
	}
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
};

//создания windows окна
void InitWindow()
{
	//имя класса окна
	const char* NameClass = "Window";

	//размер окна
	window.rc = { 0,0,window.width,window.height
	};

	//учет размера
	AdjustWindowRect(&window.rc, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE);

	//дескриптор класса окна
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.lpszClassName = NameClass;
	wc.hInstance = window.hIns;
	wc.lpfnWndProc = &WindowProc;

	//регистрация класса окна
	auto NameClassId = RegisterClassEx(&wc);

	//деструктор окна
	window.hWnd = CreateWindowEx(
		NULL,
		MAKEINTATOM(NameClassId),
		"practicum5",
		WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		window.rc.right - window.rc.left,
		window.rc.bottom - window.rc.top,
		NULL,
		NULL,
		window.hIns,
		NULL
	);

	//показ окна
	ShowWindow(window.hWnd, SW_SHOW);
}

//отрисовка изображений .bmp
void ShowBitmap(HDC hDC, int x, int y, int x1, int y1, HBITMAP hBitmapBall)
{
	HDC hMemDC = CreateCompatibleDC(hDC);
	HBITMAP hOldbm = (HBITMAP)SelectObject(hMemDC, hBitmapBall);

	BITMAP bm;
	GetObject(hBitmapBall, sizeof(BITMAP), &bm); // ← вот это и было пропущено

	TransparentBlt(
		window.contx,
		x, y, x1, y1,
		hMemDC,
		0, 0, bm.bmWidth, bm.bmHeight,
		RGB(255, 0, 222)
	);

	SelectObject(hMemDC, hOldbm);
	DeleteDC(hMemDC);
}

//загрузка модулей приложения
void InitApp()
{
	//создание и иниализация контекста устройсва и девайс устройства
	window.dev_cont = GetDC(window.hWnd);
	window.contx = CreateCompatibleDC(window.dev_cont);
	SelectObject(window.contx, CreateCompatibleBitmap(window.dev_cont, window.width, window.height));

	blocks.push_back({ 1100, 550, 100, 100 });
	blocks.push_back({ 700, 750, 100, 100 });
	blocks.push_back({ 500, 850, 150, 50 });
}

//обновление приложения
void UpdateApp()
{
	PlayerMovement();
	Gravity();
	CheckPlayerCollisions();
	EnemyAttention();
	EnemyGravity();
	BulletMovement();







}

//обработка команд устройств ввода
void UpdateKeyCode()
{
	//выход из приложения на ESC
	if (GetAsyncKeyState(VK_ESCAPE))
	{
		window.msg.message = WM_QUIT;
	}
}

//обновление изображений
void UpdateImage()
{
	BitBlt(window.dev_cont, 0, 0, window.width, window.height, window.contx, 0, 0, SRCCOPY);
	//отрисовка заднего фона
	ShowBitmap(window.contx, 0, 0, window.width, window.height, (HBITMAP)LoadImageA(NULL, "back.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
	for (int i = 0; i < blocks.size(); i++)
	{
		ShowBitmap(window.contx,
			blocks[i].x, blocks[i].y,
			blocks[i].width, blocks[i].height,
			(HBITMAP)LoadImageA(NULL, "block.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
	}
	ShowBitmap(window.contx, player.PlayerX, player.PlayerY, player.PleyerWidth, player.PleyerHeight, (HBITMAP)LoadImageA(NULL, "Player.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
	ShowBitmap(window.contx, enemy.x, enemy.y, enemy.Width, enemy.Height, (HBITMAP)LoadImageA(NULL, "enemy.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
	if (bullet.create)
	{
		ShowBitmap(window.contx, bullet.x, bullet.y, 100, 100, (HBITMAP)LoadImageA(NULL, "bullet.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));

	}
}

//вход в программу
int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd)
{
	InitWindow();
	InitApp();

	//основной цикл обновления приложения
	while (window.gbool)
	{
		//обработка соощений для окна
		while (PeekMessage(&window.msg, NULL, 0, 0, PM_REMOVE))
		{
			UpdateKeyCode();

			//отбработка сообщений
			if (window.msg.message == WM_QUIT)
			{
				window.gbool = false;
				break;
			}
			TranslateMessage(&window.msg);
			DispatchMessage(&window.msg);
		}

		UpdateImage();
		UpdateApp();

		//задержка обновления
		Sleep(16);
	}
	return 0;
}