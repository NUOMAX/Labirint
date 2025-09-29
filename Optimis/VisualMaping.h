#ifndef VISUALMAPING_H_INCLUDED
#define VISUALMAPING_H_INCLUDED

#include <cmath>
#include <string>
#include <algorithm>
#include <windows.h>
#include <conio.h>

// Предварительные объявления функций рисования
void DrawWall(HDC hdc, int x, int y, int size, COLORREF color);
void DrawFloor(HDC hdc, int x, int y, int size, COLORREF color);
void DrawPlayer(HDC hdc, int x, int y, int size, COLORREF color);
void DrawExit(HDC hdc, int x, int y, int size, COLORREF color);
void DrawWater(HDC hdc, int x, int y, int size, COLORREF color);
void DrawCaveFloor(HDC hdc, int x, int y, int size, COLORREF color);
void DrawCaveWall(HDC hdc, int x, int y, int size, COLORREF color);
void DrawMountain(HDC hdc, int x, int y, int size, COLORREF color);
void DrawDesertFloor(HDC hdc, int x, int y, int size, COLORREF color);
void DrawDesertWall(HDC hdc, int x, int y, int size, COLORREF color);
void DrawExitPoint(HDC hdc, int x, int y, int size, COLORREF color);
void DrawPineTree(HDC hdc, int x, int y, int size, COLORREF color);
void DrawBirchTree(HDC hdc, int x, int y, int size, COLORREF color);
void DrawBush(HDC hdc, int x, int y, int size, COLORREF color);
void DrawFlowers(HDC hdc, int x, int y, int size, COLORREF color);
void DrawJungle(HDC hdc, int x, int y, int size, COLORREF color);
void DrawMushroom(HDC hdc, int x, int y, int size, COLORREF color);
void DrawCaveStone(HDC hdc, int x, int y, int size, COLORREF color);
void DrawResource(HDC hdc, int x, int y, int size, int resourceType);

// Структура для хранения ресурсов
struct GraphicsResources {
    HDC hdc;
    HDC hdcBuffer;
    HBITMAP hBitmap;
    HFONT hFont;
    HBRUSH hBackgroundBrush;
    HBRUSH hFogBrush;
    RECT consoleRect;
};

// Функция для освобождения ресурсов
void CleanupResources(GraphicsResources& res) {
    if (res.hFont) DeleteObject(res.hFont);
    if (res.hBitmap) DeleteObject(res.hBitmap);
    if (res.hdcBuffer) DeleteDC(res.hdcBuffer);
    if (res.hBackgroundBrush) DeleteObject(res.hBackgroundBrush);
    if (res.hFogBrush) DeleteObject(res.hFogBrush);
    if (res.hdc) ReleaseDC(GetConsoleWindow(), res.hdc);
}

// Функция для рисования тумана
void DrawFog(HDC hdc, int x, int y, int size, double distance, int viewRadius, COLORREF fogColor) {
    int alpha = std::min(200, static_cast<int>((distance - viewRadius) / 2));
    HBRUSH tempFogBrush = CreateSolidBrush(fogColor);
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, tempFogBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, GetStockObject(NULL_PEN)));

    Rectangle(hdc, x, y, x + size, y + size);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(tempFogBrush);
}

// Функция для обработки ввода
bool ProcessInput(wchar_t& c, int& playerX, int& playerY, int n, int m, int** Labir, bool& exitRequested) {
    if (!_kbhit()) return false;

    c = _getwch();
    if (c == ' ') {
        exitRequested = true;
        return true;
    }

    if (Labir[playerY][playerX] == 3) return false;

    int newY = playerY, newX = playerX;
    switch (c) {
        case 'w': case 'W': newY--; break;
        case 's': case 'S': newY++; break;
        case 'a': case 'A': newX--; break;
        case 'd': case 'D': newX++; break;
        default: return false;
    }

    if (newY >= 0 && newY < n && newX >= 0 && newX < m &&
        Labir[newY][newX] != 1 &&
        Labir[newY][newX] != 6 &&
        Labir[newY][newX] != 7 &&
        Labir[newY][newX] != 9 &&
        Labir[newY][newX] != 22 && // Камень пещеры
        (Labir[newY][newX] < 13 || Labir[newY][newX] > 16)) { // Ресурсы (можно пройти)
        playerY = newY;
        playerX = newX;
        return true;
    }

    return false;
}

// ОПТИМИЗИРОВАННАЯ функция визуализации - рендерит только видимую область
void VisualMap(int playerX, int playerY, int n, int m, int** Labir, wchar_t c) {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    HWND hConsoleWnd = GetConsoleWindow();
    GraphicsResources res{};
    res.hdc = GetDC(hConsoleWnd);
    res.hdcBuffer = CreateCompatibleDC(res.hdc);

    GetClientRect(hConsoleWnd, &res.consoleRect);
    res.hBitmap = CreateCompatibleBitmap(res.hdc, res.consoleRect.right, res.consoleRect.bottom);
    SelectObject(res.hdcBuffer, res.hBitmap);

    // Настройка шрифта
    res.hFont = CreateFontA(28, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");
    SelectObject(res.hdcBuffer, res.hFont);
    SetTextColor(res.hdcBuffer, RGB(255, 255, 200));
    SetBkColor(res.hdcBuffer, RGB(0, 0, 0));

    const int TILE_SIZE = 60;
    const int viewRadius = 8; // Уменьшен радиус обзора для оптимизации
    const int screenWidth = res.consoleRect.right;
    const int screenHeight = res.consoleRect.bottom;
    const int centerX = screenWidth / 2;
    const int centerY = screenHeight / 2;

    // Цвета
    const COLORREF wallColor = RGB(88, 72, 56);
    const COLORREF floorColor = RGB(64, 56, 48);
    const COLORREF darkWallColor = RGB(64, 52, 40);
    const COLORREF playerColor = RGB(255, 200, 100);
    const COLORREF exitColor = RGB(200, 80, 80);
    const COLORREF fogColor = RGB(30, 30, 40);
    const COLORREF waterColor = RGB(30, 60, 150);
    const COLORREF caveFloorColor = RGB(80, 60, 50);
    const COLORREF caveWallColor = RGB(60, 45, 35);
    const COLORREF mountainColor = RGB(120, 100, 90);
    const COLORREF desertFloorColor = RGB(210, 180, 140);
    const COLORREF desertWallColor = RGB(180, 150, 110);

    res.hBackgroundBrush = CreateSolidBrush(RGB(16, 12, 24));
    res.hFogBrush = CreateSolidBrush(fogColor);

    bool firstFrame = true;
    bool exitRequested = false;

    // Основной цикл рендеринга - ОПТИМИЗИРОВАН
    while (!exitRequested) {
        FillRect(res.hdcBuffer, &res.consoleRect, res.hBackgroundBrush);

        // ОПТИМИЗАЦИЯ: рендерим только видимую область вокруг игрока
        int startI = std::max(0, playerY - viewRadius);
        int endI = std::min(n, playerY + viewRadius + 1);
        int startJ = std::max(0, playerX - viewRadius);
        int endJ = std::min(m, playerX + viewRadius + 1);

        // Рендерим только видимые тайлы
        for (int i = startI; i < endI; i++) {
            for (int j = startJ; j < endJ; j++) {
                int tileX = centerX + (j - playerX) * TILE_SIZE;
                int tileY = centerY + (i - playerY) * TILE_SIZE;

                // Быстрая проверка видимости
                if (tileX < -TILE_SIZE || tileX > screenWidth ||
                    tileY < -TILE_SIZE || tileY > screenHeight) {
                    continue;
                }

                // Рендеринг видимых тайлов
                int cellType = Labir[i][j];
                switch (cellType) {
                    case 0:  // Земля
                    case 2:  // Спавн игрока
                        DrawFloor(res.hdcBuffer, tileX, tileY, TILE_SIZE, floorColor);
                        break;
                    case 1:  // Стена
                        DrawWall(res.hdcBuffer, tileX, tileY, TILE_SIZE, wallColor);
                        break;
                    case 3:  // Выход
                        DrawFloor(res.hdcBuffer, tileX, tileY, TILE_SIZE, floorColor);
                        DrawExitPoint(res.hdcBuffer, tileX, tileY, TILE_SIZE, exitColor);
                        break;
                    case 4:  // Вода
                        DrawWater(res.hdcBuffer, tileX, tileY, TILE_SIZE, waterColor);
                        break;
                    case 5:  // Пещера (пол)
                        DrawCaveFloor(res.hdcBuffer, tileX, tileY, TILE_SIZE, caveFloorColor);
                        break;
                    case 6:  // Дуб (как стена)
                        DrawWall(res.hdcBuffer, tileX, tileY, TILE_SIZE, wallColor);
                        break;
                    case 7:  // Горы
                        DrawMountain(res.hdcBuffer, tileX, tileY, TILE_SIZE, mountainColor);
                        break;
                    case 8:  // Пустыня (пол)
                        DrawDesertFloor(res.hdcBuffer, tileX, tileY, TILE_SIZE, desertFloorColor);
                        break;
                    case 9:  // Пустыня (стена)
                        DrawDesertWall(res.hdcBuffer, tileX, tileY, TILE_SIZE, desertWallColor);
                        break;
                    case 16: // Кусты
                        DrawFloor(res.hdcBuffer, tileX, tileY, TILE_SIZE, floorColor);
                        DrawBush(res.hdcBuffer, tileX, tileY, TILE_SIZE, RGB(60, 120, 60));
                        break;
                    case 17: // Ель
                        DrawFloor(res.hdcBuffer, tileX, tileY, TILE_SIZE, floorColor);
                        DrawPineTree(res.hdcBuffer, tileX, tileY, TILE_SIZE, RGB(40, 100, 40));
                        break;
                    case 18: // Береза
                        DrawFloor(res.hdcBuffer, tileX, tileY, TILE_SIZE, floorColor);
                        DrawBirchTree(res.hdcBuffer, tileX, tileY, TILE_SIZE, RGB(160, 200, 160));
                        break;
                    case 19: // Цветы
                        DrawFlowers(res.hdcBuffer, tileX, tileY, TILE_SIZE, RGB(100, 180, 80));
                        break;
                    case 20: // Джунгли
                        DrawJungle(res.hdcBuffer, tileX, tileY, TILE_SIZE, RGB(30, 100, 40));
                        break;
                    case 21: // Грибы
                        DrawFloor(res.hdcBuffer, tileX, tileY, TILE_SIZE, floorColor);
                        DrawMushroom(res.hdcBuffer, tileX, tileY, TILE_SIZE, RGB(200, 50, 50));
                        break;
                    case 22: // Камень пещеры
                        DrawCaveStone(res.hdcBuffer, tileX, tileY, TILE_SIZE, RGB(80, 80, 90));
                        break;
                    case 13: // Уголь
                    case 14: // Железо
                    case 15: // Золото
                        DrawFloor(res.hdcBuffer, tileX, tileY, TILE_SIZE, floorColor);
                        DrawResource(res.hdcBuffer, tileX, tileY, TILE_SIZE, cellType);
                        break;
                    default: // Неизвестный тип - рисуем как стену
                        DrawWall(res.hdcBuffer, tileX, tileY, TILE_SIZE, wallColor);
                        break;
                }
            }
        }

        // Рисуем игрока
        DrawPlayer(res.hdcBuffer, centerX, centerY, TILE_SIZE, playerColor);

        // Интерфейс
        std::string coordText = "X: " + std::to_string(playerX) + "  Y: " + std::to_string(playerY);
        TextOutA(res.hdcBuffer, 10, 10, coordText.c_str(), coordText.length());

        // Используем wstring и TextOutW для русского текста
        std::wstring controlsText = L"WASD - \u0414\u0432\u0438\u0436\u0435\u043D\u0438\u0435 | \u041F\u0420\u041E\u0411\u0415\u041B - \u0412\u044B\u0445\u043E\u0434";
        TextOutW(res.hdcBuffer, 10, screenHeight - 30, controlsText.c_str(), controlsText.length());

        if (Labir[playerY][playerX] == 3) {
        std::wstring winText = L"\u0425\u0420\u0410\u041C \u041F\u041E\u041A\u0418\u041D\u0423\u0422!";
        TextOutW(res.hdcBuffer, screenWidth / 2 - 150, screenHeight / 2, winText.c_str(), winText.length());
}

        // Вывод на экран
        BitBlt(res.hdc, 0, 0, screenWidth, screenHeight, res.hdcBuffer, 0, 0, SRCCOPY);

        // Обработка ввода
        if (_kbhit() || firstFrame) {
            firstFrame = false;
            ProcessInput(c, playerX, playerY, n, m, Labir, exitRequested);
        }
        Sleep(5);
    }

    // Очистка ресурсов
    CleanupResources(res);
    PostMessage(hConsoleWnd, WM_CLOSE, 0, 0);
}

// Функции рисования
void DrawWater(HDC hdc, int x, int y, int size, COLORREF color) {
    HBRUSH waterBrush = CreateSolidBrush(color);
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, waterBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, GetStockObject(NULL_PEN)));

    Rectangle(hdc, x, y, x + size, y + size);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(waterBrush);
}

void DrawCaveFloor(HDC hdc, int x, int y, int size, COLORREF color) {
    HBRUSH floorBrush = CreateSolidBrush(color);
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, floorBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, GetStockObject(NULL_PEN)));

    Rectangle(hdc, x, y, x + size, y + size);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(floorBrush);
}

void DrawCaveWall(HDC hdc, int x, int y, int size, COLORREF color) {
    HBRUSH wallBrush = CreateSolidBrush(color);
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, wallBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, GetStockObject(NULL_PEN)));

    Rectangle(hdc, x, y, x + size, y + size);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(wallBrush);
}

void DrawMountain(HDC hdc, int x, int y, int size, COLORREF color) {
    HBRUSH mountainBrush = CreateSolidBrush(color);
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, mountainBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, GetStockObject(NULL_PEN)));

    Rectangle(hdc, x, y, x + size, y + size);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(mountainBrush);
}

void DrawDesertFloor(HDC hdc, int x, int y, int size, COLORREF color) {
    HBRUSH desertBrush = CreateSolidBrush(color);
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, desertBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, GetStockObject(NULL_PEN)));

    Rectangle(hdc, x, y, x + size, y + size);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(desertBrush);
}

void DrawDesertWall(HDC hdc, int x, int y, int size, COLORREF color) {
    HBRUSH wallBrush = CreateSolidBrush(color);
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, wallBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, GetStockObject(NULL_PEN)));

    Rectangle(hdc, x, y, x + size, y + size);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(wallBrush);
}

void DrawExitPoint(HDC hdc, int x, int y, int size, COLORREF color) {
    int centerX = x + size / 2;
    int centerY = y + size / 2;
    int radius = size / 3;

    HBRUSH outerBrush = CreateSolidBrush(RGB(100, 200, 100));
    HPEN outerPen = CreatePen(PS_SOLID, 3, RGB(80, 180, 80));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, outerBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, outerPen));

    Ellipse(hdc, centerX - radius, centerY - radius,
            centerX + radius, centerY + radius);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(outerBrush);
    DeleteObject(outerPen);
}

void DrawWall(HDC hdc, int x, int y, int size, COLORREF color) {
    HBRUSH wallBrush = CreateSolidBrush(color);
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, wallBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, GetStockObject(NULL_PEN)));

    Rectangle(hdc, x, y, x + size, y + size);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(wallBrush);
}

void DrawFloor(HDC hdc, int x, int y, int size, COLORREF color) {
    HBRUSH floorBrush = CreateSolidBrush(color);
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, floorBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, GetStockObject(NULL_PEN)));

    Rectangle(hdc, x, y, x + size, y + size);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(floorBrush);
}

void DrawPlayer(HDC hdc, int x, int y, int size, COLORREF color) {
    int radius = size / 3;
    int centerX = x + size / 2;
    int centerY = y + size / 2;

    HBRUSH playerBrush = CreateSolidBrush(color);
    HPEN playerPen = CreatePen(PS_SOLID, 2, RGB(180, 140, 80));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, playerBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, playerPen));

    Ellipse(hdc, centerX - radius, centerY - radius,
            centerX + radius, centerY + radius);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(playerBrush);
    DeleteObject(playerPen);
}

void DrawExit(HDC hdc, int x, int y, int size, COLORREF color) {
    int centerX = x + size / 2;
    int centerY = y + size / 2;
    int radius = size / 3;

    HBRUSH exitBrush = CreateSolidBrush(RGB(200, 80, 80));
    HPEN exitPen = CreatePen(PS_SOLID, 3, RGB(150, 50, 50));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, exitBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, exitPen));

    Ellipse(hdc, centerX - radius, centerY - radius,
            centerX + radius, centerY + radius);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(exitBrush);
    DeleteObject(exitPen);
}

void DrawPineTree(HDC hdc, int x, int y, int size, COLORREF color) {
    int centerX = x + size / 2;
    int trunkWidth = size / 6;
    int trunkHeight = size / 3;

    // Рисуем ствол
    HBRUSH trunkBrush = CreateSolidBrush(RGB(101, 67, 33));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, trunkBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, GetStockObject(NULL_PEN)));

    Rectangle(hdc, centerX - trunkWidth/2, y + size - trunkHeight,
              centerX + trunkWidth/2, y + size);

    // Рисуем хвою (треугольники)
    HBRUSH pineBrush = CreateSolidBrush(RGB(40, 100, 40));
    SelectObject(hdc, pineBrush);

    POINT triangle1[3] = {
        {centerX, y + size/4},
        {x + size/4, y + size/2},
        {x + 3*size/4, y + size/2}
    };
    POINT triangle2[3] = {
        {centerX, y + size/6},
        {x + size/3, y + size/3},
        {x + 2*size/3, y + size/3}
    };
    POINT triangle3[3] = {
        {centerX, y},
        {x + size/3, y + size/4},
        {x + 2*size/3, y + size/4}
    };

    Polygon(hdc, triangle1, 3);
    Polygon(hdc, triangle2, 3);
    Polygon(hdc, triangle3, 3);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(trunkBrush);
    DeleteObject(pineBrush);
}

void DrawBirchTree(HDC hdc, int x, int y, int size, COLORREF color) {
    int centerX = x + size / 2;
    int trunkWidth = size / 8;
    int trunkHeight = size * 2/3;

    // Рисуем ствол березы
    HBRUSH trunkBrush = CreateSolidBrush(RGB(220, 220, 220));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, trunkBrush));
    HPEN trunkPen = CreatePen(PS_SOLID, 1, RGB(150, 150, 150));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, trunkPen));

    Rectangle(hdc, centerX - trunkWidth/2, y + size - trunkHeight,
              centerX + trunkWidth/2, y + size);

    // Рисуем черные полосы на березе
    HPEN stripePen = CreatePen(PS_SOLID, 1, RGB(80, 80, 80));
    SelectObject(hdc, stripePen);
    for (int i = 0; i < 3; i++) {
        int stripeY = y + size - trunkHeight + (i * trunkHeight / 3) + 5;
        MoveToEx(hdc, centerX - trunkWidth/2 + 2, stripeY, NULL);
        LineTo(hdc, centerX + trunkWidth/2 - 2, stripeY);
    }

    // Рисуем листву
    HBRUSH foliageBrush = CreateSolidBrush(RGB(160, 200, 160));
    SelectObject(hdc, foliageBrush);
    SelectObject(hdc, GetStockObject(NULL_PEN));

    int foliageSize = size / 2;
    Ellipse(hdc, centerX - foliageSize, y + size/4,
            centerX + foliageSize, y + size/2 + foliageSize/2);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(trunkBrush);
    DeleteObject(foliageBrush);
    DeleteObject(trunkPen);
    DeleteObject(stripePen);
}

void DrawBush(HDC hdc, int x, int y, int size, COLORREF color) {
    int centerX = x + size / 2;
    int centerY = y + size / 2;
    int bushSize = size / 2;

    HBRUSH bushBrush = CreateSolidBrush(RGB(60, 120, 60));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, bushBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, GetStockObject(NULL_PEN)));

    // Рисуем несколько перекрывающихся кругов для куста
    Ellipse(hdc, centerX - bushSize, centerY - bushSize/2,
            centerX, centerY + bushSize/2);
    Ellipse(hdc, centerX - bushSize/2, centerY - bushSize,
            centerX + bushSize/2, centerY);
    Ellipse(hdc, centerX, centerY - bushSize/2,
            centerX + bushSize, centerY + bushSize/2);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(bushBrush);
}

void DrawFlowers(HDC hdc, int x, int y, int size, COLORREF color) {
    // Рисуем основу травы
    HBRUSH grassBrush = CreateSolidBrush(RGB(100, 180, 80));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, grassBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, GetStockObject(NULL_PEN)));

    Rectangle(hdc, x, y, x + size, y + size);

    // Рисуем цветочки
    int flowerCount = 3 + (x + y) % 3; // Детерминированное случайное число
    for (int i = 0; i < flowerCount; i++) {
        int flowerX = x + size/4 + (i * size / 4);
        int flowerY = y + size/4 + ((i % 2) * size / 3);

        // Лепестки
        HBRUSH petalBrush = CreateSolidBrush(i == 0 ? RGB(255, 100, 100) :
                                           i == 1 ? RGB(255, 255, 100) :
                                           RGB(100, 100, 255));
        SelectObject(hdc, petalBrush);

        int petalSize = size / 8;
        Ellipse(hdc, flowerX - petalSize, flowerY - petalSize,
                flowerX + petalSize, flowerY + petalSize);

        DeleteObject(petalBrush);
    }

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(grassBrush);
}

void DrawJungle(HDC hdc, int x, int y, int size, COLORREF color) {
    // Основа - густая зелень
    HBRUSH jungleBrush = CreateSolidBrush(RGB(30, 100, 40));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, jungleBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, GetStockObject(NULL_PEN)));

    Rectangle(hdc, x, y, x + size, y + size);

    // Рисуем пальмовые листья
    HBRUSH palmBrush = CreateSolidBrush(RGB(60, 140, 60));
    SelectObject(hdc, palmBrush);

    int centerX = x + size / 2;
    int centerY = y + size / 2;

    // Несколько пальмовых ветвей
    for (int angle = 0; angle < 360; angle += 60) {
        double rad = angle * 3.14159 / 180;
        int endX = centerX + (int)(size/3 * cos(rad));
        int endY = centerY + (int)(size/3 * sin(rad));

        POINT leaf[5] = {
            {centerX, centerY},
            {endX - size/8, endY - size/8},
            {endX + size/4, endY},
            {endX - size/8, endY + size/8},
            {centerX, centerY}
        };
        Polygon(hdc, leaf, 5);
    }

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(jungleBrush);
    DeleteObject(palmBrush);
}

void DrawMushroom(HDC hdc, int x, int y, int size, COLORREF color) {
    int centerX = x + size / 2;

    // Ножка гриба
    HBRUSH stemBrush = CreateSolidBrush(RGB(220, 200, 180));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, stemBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, GetStockObject(NULL_PEN)));

    int stemWidth = size / 6;
    int stemHeight = size / 2;
    Rectangle(hdc, centerX - stemWidth/2, y + size - stemHeight,
              centerX + stemWidth/2, y + size - stemHeight/3);

    // Шляпка гриба
    HBRUSH capBrush = CreateSolidBrush(RGB(200, 50, 50));
    SelectObject(hdc, capBrush);

    int capSize = size / 2;
    Ellipse(hdc, centerX - capSize, y + size - stemHeight - capSize/2,
            centerX + capSize, y + size - stemHeight + capSize/2);

    // Белые точки на шляпке
    HBRUSH dotBrush = CreateSolidBrush(RGB(255, 255, 255));
    SelectObject(hdc, dotBrush);

    int dotSize = size / 12;
    Ellipse(hdc, centerX - dotSize, y + size - stemHeight - dotSize/2,
            centerX, y + size - stemHeight + dotSize/2);
    Ellipse(hdc, centerX + dotSize/2, y + size - stemHeight,
            centerX + dotSize, y + size - stemHeight + dotSize);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(stemBrush);
    DeleteObject(capBrush);
    DeleteObject(dotBrush);
}

void DrawCaveStone(HDC hdc, int x, int y, int size, COLORREF color) {
    HBRUSH stoneBrush = CreateSolidBrush(RGB(80, 80, 90));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, stoneBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, GetStockObject(NULL_PEN)));

    // Рисуем неровный камень
    POINT stone[6] = {
        {x + size/4, y},
        {x + 3*size/4, y + size/4},
        {x + size, y + size/2},
        {x + 3*size/4, y + 3*size/4},
        {x + size/4, y + size},
        {x, y + size/2}
    };

    Polygon(hdc, stone, 6);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(stoneBrush);
}

void DrawResource(HDC hdc, int x, int y, int size, int resourceType) {
    COLORREF resourceColor;

    switch(resourceType) {
        case 13: // Уголь
            resourceColor = RGB(40, 40, 40);
            break;
        case 14: // Железо
            resourceColor = RGB(180, 100, 60);
            break;
        case 15: // Золото
            resourceColor = RGB(255, 215, 0);
            break;
        case 16: // Кристаллы
            resourceColor = RGB(100, 200, 255);
            break;
        default:
            resourceColor = RGB(128, 128, 128);
    }

    HBRUSH resourceBrush = CreateSolidBrush(resourceColor);
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, resourceBrush));
    HPEN resourcePen = CreatePen(PS_SOLID, 1,
        resourceType == 16 ? RGB(150, 220, 255) : RGB(100, 100, 100));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, resourcePen));

    // Рисуем ресурс в виде скопления кристаллов/камней
    int centerX = x + size / 2;
    int centerY = y + size / 2;

    if (resourceType == 16) { // Кристаллы
        POINT crystal1[3] = {{centerX, y + size/4}, {x + size/3, y + 3*size/4}, {x + 2*size/3, y + 3*size/4}};
        POINT crystal2[3] = {{centerX, y + 3*size/4}, {x + size/3, y + size/4}, {x + 2*size/3, y + size/4}};
        Polygon(hdc, crystal1, 3);
        Polygon(hdc, crystal2, 3);
    } else { // Руды
        int stoneSize = size / 3;
        Ellipse(hdc, centerX - stoneSize, centerY - stoneSize,
                centerX, centerY);
        Ellipse(hdc, centerX, centerY,
                centerX + stoneSize, centerY + stoneSize);
        Ellipse(hdc, centerX - stoneSize/2, centerY + stoneSize/2,
                centerX + stoneSize/2, centerY + stoneSize);
    }

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(resourceBrush);
    DeleteObject(resourcePen);
}

#endif // VISUALMAPING_H_INCLUDED
