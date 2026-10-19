//linker::system::subsystem  - Windows(/ SUBSYSTEM:WINDOWS)
//configuration::advanced::character set - not set
//linker::input::additional dependensies Msimg32.lib; Winmm.lib
#include "windows.h"
#include "vector"
#include "string"
#pragma comment(lib, "Msimg32.lib") 

// секция данных игры
struct sprite {
    float x, y, width, height, rad, dx, dy, speed;
    bool iscreate = false;
    HBITMAP hBitmap;//хэндл к спрайту шарика
};

sprite racket;//ракетка игрока
sprite enemy;//ракетка противника
sprite ball;//шарик
#define raw 2
#define colomn 10
sprite block[colomn][raw];

struct {
    int score, balls;//количество набранных очков и оставшихся "жизней"
    bool action = false;//состояние - ожидание (игрок должен нажать пробел) или игра
} game;

struct {
    HWND hWnd;//хэндл окна
    HDC device_context, context;// два контекста устройства (для буферизации)
    int width, height;//сюда сохраним размеры окна которое создаст программа
} window;

HBITMAP hBack;// хэндл для фонового изображения

//cекция кода
bool CheckCollision(
    float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2)
{
    return
        x1      < x2 + w2 &&
        x1 + w1 > x2 &&
        y1      < y2 + h2 &&
        y1 + h1 > y2;
}

std::vector<POINT> TraceLine(int x0, int y0, int x1, int y1)
{
    std::vector<POINT> points;

    int dx = (x1 >= x0) ? (x1 - x0) : (x0 - x1);
    int dy = -((y1 >= y0) ? (y1 - y0) : (y0 - y1));

    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx + dy;

    int x = x0;
    int y = y0;

    while (true)
    {
        SetPixel(window.context, x, y, RGB(255,255,255));
        points.push_back({ x, y });

        if (x == x1 && y == y1)
        {
            break;
        }

        int e2 = 2 * err;

        if (e2 >= dy)
        {
            err += dy;
            x += sx;
        }

        if (e2 <= dx)
        {
            err += dx;
            y += sy;
        }
    }

    return points;
}

void InitGame()
{
    //в этой секции загружаем спрайты с помощью функций gdi
    //пути относительные - файлы должны лежать рядом с .exe
    //результат работы LoadImageA сохраняет в хэндлах битмапов, рисование спрайтов будет произовдиться с помощью этих хэндлов
    ball.hBitmap = (HBITMAP)LoadImageA(NULL, "ball.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    racket.hBitmap = (HBITMAP)LoadImageA(NULL, "racket.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hBack = (HBITMAP)LoadImageA(NULL, "back.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    for (int i = 0; i < colomn; i++)
    {
        for (int b = 0; b < raw; b++)
        {
            block[i][b].iscreate = true;
            block[i][b].width = window.width / colomn;
            block[i][b].height = 100;

            block[i][b].x = 0 + (i * block[i][b].width);
            block[i][b].y = 300 + (b * block[i][b].height);
        }
    }
    //------------------------------------------------------

    racket.width = 300;
    racket.height = 50;
    racket.speed = 30;//скорость перемещения ракетки
    racket.x = window.width / 2.;//ракетка посередине окна
    racket.y = window.height - racket.height;//чуть выше низа экрана - на высоту ракетки

    ball.dy = (rand() % 65 + 35) / 100.;//формируем вектор полета шарика
    ball.dx = -(1 - ball.dy);//формируем вектор полета шарика
    ball.speed = 11;
    ball.rad = 20;
    ball.x = racket.x;//x координата шарика - на середие ракетки
    ball.y = racket.y - ball.rad;//шарик лежит сверху ракетки

    game.score = 0;
    game.balls = 9;
}
void Paint_line(
    float start_x, float start_y,
    float iterator,
    float X_pixel2, float Y_pixel2,
    int R, int G, int B)
{
    float dx = abs(start_x - X_pixel2);
    float dy = abs(start_y - Y_pixel2);
    float C;
    C = sqrt(pow(dx, 2) + pow(dy, 2));

    SetPixel(window.context, iterator * dx / C + start_x, iterator * dy / C + start_y, RGB(R, G, B));
}

bool CheckBlockCollisionAt(float bx, float by)
{
    float ballLeft = bx - ball.rad;
    float ballRight = bx + ball.rad;
    float ballTop = by - ball.rad;
    float ballBottom = by + ball.rad;

    for (int i = 0; i < colomn; i++)
    {
        for (int b = 0; b < raw; b++)
        {
            if (!block[i][b].iscreate)
            {
                continue;
            }
            if (CheckCollision(
                ballLeft,
                ballTop,
                2.0f * ball.rad,
                2.0f * ball.rad,
                block[i][b].x,
                block[i][b].y,
                block[i][b].width,
                block[i][b].height))
            {
                float blockLeft = block[i][b].x;
                float blockRight =
                    block[i][b].x + block[i][b].width;

                float blockTop = block[i][b].y;
                float blockBottom =
                    block[i][b].y + block[i][b].height;

                float overlapLeft = ballRight - blockLeft;
                float overlapRight = blockRight - ballLeft;
                float overlapTop = ballBottom - blockTop;
                float overlapBottom = blockBottom - ballTop;

                float minX = min(overlapLeft, overlapRight);
                float minY = min(overlapTop, overlapBottom);

                block[i][b].iscreate = false;

                if (minX < minY)
                {
                    ball.dx *= -1;
                }
                else
                {
                    ball.dy *= -1;
                }

                return true;
            }
        }
    }

    return false;
}

void ProcessSound(const char* name)//проигрывание аудиофайла в формате .wav, файл должен лежать в той же папке где и программа
{
    //PlaySound(TEXT(name), NULL, SND_FILENAME | SND_ASYNC);//переменная name содежрит имя файла. флаг ASYNC позволяет проигрывать звук паралельно с исполнением программы
}

void ShowScore()
{
    //поиграем шрифтами и цветами
    SetTextColor(window.context, RGB(160, 160, 160));
    SetBkColor(window.context, RGB(0, 0, 0));
    SetBkMode(window.context, TRANSPARENT);
    auto hFont = CreateFont(70, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI");
    auto hTmp = (HFONT)SelectObject(window.context, hFont);

    char txt[32];//буфер для текста
    _itoa_s(game.score, txt, 10);//преобразование числовой переменной в текст. текст окажется в переменной txt
    TextOutA(window.context, 10, 10, "Score", 5);
    TextOutA(window.context, 200, 10, (LPCSTR)txt, strlen(txt));

    _itoa_s(game.balls, txt, 10);
    TextOutA(window.context, 10, 100, "Balls", 5);
    TextOutA(window.context, 200, 100, (LPCSTR)txt, strlen(txt));
}

void ProcessInput()
{
    if (GetAsyncKeyState(VK_LEFT)) racket.x -= racket.speed;
    if (GetAsyncKeyState(VK_RIGHT)) racket.x += racket.speed;

    if (!game.action && GetAsyncKeyState(VK_SPACE))
    {
        game.action = true;
        ProcessSound("bounce.wav");
    }
}

void ShowBitmap(HDC hDC, int x, int y, int x1, int y1, HBITMAP hBitmapBall, bool alpha = false)
{
    HBITMAP hbm, hOldbm;
    HDC hMemDC;
    BITMAP bm;

    hMemDC = CreateCompatibleDC(hDC); // Создаем контекст памяти, совместимый с контекстом отображения
    hOldbm = (HBITMAP)SelectObject(hMemDC, hBitmapBall);// Выбираем изображение bitmap в контекст памяти

    if (hOldbm) // Если не было ошибок, продолжаем работу
    {
        GetObject(hBitmapBall, sizeof(BITMAP), (LPSTR)&bm); // Определяем размеры изображения

        if (alpha)
        {
            TransparentBlt(window.context, x, y, x1, y1, hMemDC, 0, 0, x1, y1, RGB(0, 0, 0));//все пиксели черного цвета будут интепретированы как прозрачные
        }
        else
        {
            StretchBlt(hDC, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY); // Рисуем изображение bitmap
        }

        SelectObject(hMemDC, hOldbm);// Восстанавливаем контекст памяти
    }

    DeleteDC(hMemDC); // Удаляем контекст памяти
}


void WordWrite(std::string* word)
{
    *word += " !";
}


void ShowRacketAndBall()
{
    ShowBitmap(window.context, 0, 0, window.width, window.height, hBack);//задний фон
    ShowBitmap(window.context, racket.x - racket.width / 2., racket.y, racket.width, racket.height, racket.hBitmap);// ракетка игрока
    for (int i = 0; i < colomn; i++)
    {
        for (size_t b = 0; b < raw; b++)
        {
            if (block[i][b].iscreate)
            {
                ShowBitmap(window.context, block[i][b].x, block[i][b].y, 200, 100, (HBITMAP)LoadImageA(NULL, "block.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
            }
        }
    }

    ShowBitmap(window.context, ball.x - ball.rad, ball.y - ball.rad, 2 * ball.rad, 2 * ball.rad, ball.hBitmap, true);// шарик
    //float bx = ball.x;
    //float by = ball.y;
    //float X = ball.dx * ball.speed;
    //float Y = ball.dy * ball.speed;
    ///*for (int i = 1; i < 1000; i++)
    //{
    //    Paint_line(500, 500, i, 500, 0, 255,255,255);
    //}*/

    std::string word = "hello";


    TraceLine(500, 500, 1920-100, 1080);

}

void LimitRacket()
{
    racket.x = max(racket.x, racket.width / 2.);//если коодината левого угла ракетки меньше нуля, присвоим ей ноль
    racket.x = min(racket.x, window.width - racket.width / 2.);//аналогично для правого угла
}

void CheckWalls()
{
    if (ball.x < ball.rad || ball.x > window.width - ball.rad)
    {
        ball.dx *= -1;
        ProcessSound("bounce.wav");
    }
}

void CheckRoof()
{
    if (ball.y < ball.rad)
    {
        ball.dy *= -1;
        ProcessSound("bounce.wav");
    }
}

bool tail = false;

void CheckFloor()
{
    if (ball.y > window.height - ball.rad - racket.height)//шарик пересек линию отскока - горизонталь ракетки
    {
        if (!tail && ball.x >= racket.x - racket.width / 2. - ball.rad && ball.x <= racket.x + racket.width / 2. + ball.rad)//шарик отбит, и мы не в режиме обработки хвоста
        {
            game.score++;//за каждое отбитие даем одно очко
            ball.speed += 5. / game.score;//но увеличиваем сложность - прибавляем скорости шарику
            ball.dy *= -1;//отскок
            racket.width -= 10. / game.score;//дополнительно уменьшаем ширину ракетки - для сложности
            ProcessSound("bounce.wav");//играем звук отскока

            racket.speed -= 10 / game.score;

            if (game.score == 5) {
                racket.width -= 5.;
            }
            if (game.score == 10) {
                racket.width -= 5.;
            }
            if (game.score == 15) {
                racket.width -= 5.;
            }
            if (game.score == 20) {
                racket.width -= 5.;
            }
            if (game.score == 25) {
                racket.width -= 5.;
            }
            if (game.score == 30) {
                racket.width -= 5.;
            }
            if (game.score == 35) {
                racket.width -= 5.;
            }
        }
        else
        {//шарик не отбит
            tail = true;//дадим шарику упасть ниже ракетки

            if (ball.y - ball.rad > window.height)//если шарик ушел за пределы окна
            {
                game.balls--;//уменьшаем количество "жизней"

                ProcessSound("fail.wav");//играем звук

                if (game.balls < 0) { //проверка условия окончания "жизней"
                    MessageBoxA(window.hWnd, "game over", "", MB_OK);//выводим сообщение о проигрыше
                    InitGame();//переинициализируем игру
                }

                ball.dy = (rand() % 65 + 35) / 100.;//задаем новый случайный вектор для шарика
                ball.dx = -(1 - ball.dy);
                ball.x = racket.x;//инициализируем координаты шарика - ставим его на ракетку
                ball.y = racket.y - ball.rad;
                game.action = false;//приостанавливаем игру, пока игрок не нажмет пробел
                tail = false;
            }
        }
    }
}

void ProcessRoom()
{
    //обрабатываем стены, потолок и пол. принцип - угол падения равен углу отражения, а значит, для отскока мы можем просто инвертировать часть вектора движения шарика
    CheckWalls();
    CheckRoof();
    CheckFloor();
}

void ProcessBall()
{
    if (!game.action)
    {
        ball.x = racket.x;
        return;
    }

    int x0 = (int)ball.x;
    int y0 = (int)ball.y;

    int x1 = (int)(ball.x + ball.dx * ball.speed);
    int y1 = (int)(ball.y + ball.dy * ball.speed);

    std::vector<POINT> path = TraceLine(x0, y0, x1, y1);

    for (size_t i = 1; i < path.size(); i++)
    {
        ball.x = (float)path[i].x;
        ball.y = (float)path[i].y;
        if (CheckBlockCollisionAt(ball.x, ball.y))
        {
            break;
        }
    }
}

void InitWindow()
{
    SetProcessDPIAware();
    window.hWnd = CreateWindow("edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0);

    RECT r;
    GetClientRect(window.hWnd, &r);
    window.device_context = GetDC(window.hWnd);//из хэндла окна достаем хэндл контекста устройства для рисования
    window.width = r.right - r.left;//определяем размеры и сохраняем
    window.height = r.bottom - r.top;
    window.context = CreateCompatibleDC(window.device_context);//второй буфер
    SelectObject(window.context, CreateCompatibleBitmap(window.device_context, window.width, window.height));//привязываем окно к контексту
    GetClientRect(window.hWnd, &r);
}

int APIENTRY wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine,
    _In_ int nCmdShow)
{
    InitWindow();//здесь инициализируем все что нужно для рисования в окне
    InitGame();//здесь инициализируем переменные игры

    //mciSendString(TEXT("play ..\\Debug\\music.mp3 repeat"), NULL, 0, NULL);
    ShowCursor(NULL);

    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        ShowRacketAndBall();//рисуем фон, ракетку и шарик
        ShowScore();//рисуем очик и жизни
        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//копируем буфер в окно
        Sleep(16);//ждем 16 милисекунд (1/количество кадров в секунду)

        ProcessInput();//опрос клавиатуры
        LimitRacket();//проверяем, чтобы ракетка не убежала за экран
        ProcessBall();//перемещаем шарик
        ProcessRoom();//обрабатываем отскоки от стен и каретки, попадание шарика в картетку
    }
}