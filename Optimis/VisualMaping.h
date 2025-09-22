#ifndef VISUALMAPING_H_INCLUDED
#define VISUALMAPING_H_INCLUDED

#include <cmath>
#include <string>
#include <algorithm>
#include <windows.h>

// Предварительные объявления функций рисования
void DrawWall(HDC hdc, int x, int y, int size, COLORREF color, int blockX, int blockY, int variant);
void DrawFloor(HDC hdc, int x, int y, int size, COLORREF color, int blockX, int blockY, int variant);
void DrawPlayer(HDC hdc, int x, int y, int size, COLORREF color);
void DrawExit(HDC hdc, int x, int y, int size, COLORREF color);
void DrawWater(HDC hdc, int x, int y, int size, COLORREF color, int blockX, int blockY, int variant);
void DrawCaveFloor(HDC hdc, int x, int y, int size, COLORREF color, int blockX, int blockY, int variant);
void DrawCaveWall(HDC hdc, int x, int y, int size, COLORREF color, int blockX, int blockY, int variant);
void DrawMountain(HDC hdc, int x, int y, int size, COLORREF color, int blockX, int blockY, int variant);
void DrawDesertFloor(HDC hdc, int x, int y, int size, COLORREF color, int blockX, int blockY, int variant);
void DrawDesertWall(HDC hdc, int x, int y, int size, COLORREF color, int blockX, int blockY, int variant);
void DrawExitPoint(HDC hdc, int x, int y, int size, COLORREF color);



// Оптимизированная функция хэширования
inline unsigned int textureHash(int x, int y, int variant) {
    return (x * 73856093) ^ (y * 19349663) ^ (variant * 83492791);
}

// Оптимизированная функция псевдослучайных чисел
inline int textureRand(unsigned int &seed) {
    seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    return seed;
}

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
    COLORREF fogWithAlpha = RGB(
        GetRValue(fogColor),
        GetGValue(fogColor),
        GetBValue(fogColor)
    );
    HBRUSH tempFogBrush = CreateSolidBrush(fogWithAlpha);
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

    if (inBounds(newY, newX, n, m) &&
        Labir[newY][newX] != 1 &&
        Labir[newY][newX] != 6 &&
        Labir[newY][newX] != 7 &&
        Labir[newY][newX] != 9) {
        playerY = newY;
        playerX = newX;
        return true;
    }

    return false;
}

// Основная функция визуализации
void VisualMap(int playerX, int playerY, int n, int m, int** Labir, wchar_t c) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    HWND hConsoleWnd = GetConsoleWindow();

    Sleep(100);

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
    const int viewRadius = 600;
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

    res.hBackgroundBrush = CreateSolidBrush(RGB(16, 12, 24));
    res.hFogBrush = CreateSolidBrush(fogColor);

    bool firstFrame = true;
    bool exitRequested = false;
    const int viewRadiusSquared = viewRadius * viewRadius;

    // Основной цикл рендеринга
    while (!exitRequested) {
        FillRect(res.hdcBuffer, &res.consoleRect, res.hBackgroundBrush);

        // Рендеринг карты
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int tileX = centerX + (j - playerX) * TILE_SIZE;
                int tileY = centerY + (i - playerY) * TILE_SIZE;

                // Отсечение невидимых тайлов
                if (tileX < -TILE_SIZE || tileX > screenWidth ||
                    tileY < -TILE_SIZE || tileY > screenHeight) {
                    continue;
                }

                // Проверка расстояния до центра
                int dx = tileX - centerX;
                int dy = tileY - centerY;
                int distanceSquared = dx * dx + dy * dy;

                if (distanceSquared > viewRadiusSquared) {
                    double distance = sqrt(static_cast<double>(distanceSquared));
                    DrawFog(res.hdcBuffer, tileX, tileY, TILE_SIZE, distance, viewRadius, fogColor);
                    continue;
                }

                // Рендеринг видимых тайлов
                if (!inBounds(i, j, n, m)) {
                    DrawWall(res.hdcBuffer, tileX, tileY, TILE_SIZE, darkWallColor, j, i, (i + j) % 8);
                    continue;
                }

                // Оптимизированный switch по типам блоков
                switch (Labir[i][j]) {
                    case 0:
                    case 2:
                        DrawFloor(res.hdcBuffer, tileX, tileY, TILE_SIZE, darkWallColor, j, i, (i + j) % 8);
                        break;
                    case 3:
                        DrawFloor(res.hdcBuffer, tileX, tileY, TILE_SIZE, darkWallColor, j, i, (i + j) % 8);
                        DrawExitPoint(res.hdcBuffer, tileX, tileY, TILE_SIZE, exitColor);
                        break;
                    case 4:
                        DrawWater(res.hdcBuffer, tileX, tileY, TILE_SIZE, wallColor, j, i, (i + j) % 8);
                        break;
                    case 5:
                        DrawCaveFloor(res.hdcBuffer, tileX, tileY, TILE_SIZE, wallColor, j, i, (i + j) % 8);
                        break;
                    case 6:
                        DrawCaveWall(res.hdcBuffer, tileX, tileY, TILE_SIZE, wallColor, j, i, (i + j) % 8);
                        break;
                    case 7:
                        DrawMountain(res.hdcBuffer, tileX, tileY, TILE_SIZE, wallColor, j, i, (i + j) % 8);
                        break;
                    case 8:
                        DrawDesertFloor(res.hdcBuffer, tileX, tileY, TILE_SIZE, wallColor, j, i, (i + j) % 8);
                        break;
                    case 9:
                        DrawDesertWall(res.hdcBuffer, tileX, tileY, TILE_SIZE, wallColor, j, i, (i + j) % 8);
                        break;
                    default:
                        DrawWall(res.hdcBuffer, tileX, tileY, TILE_SIZE, wallColor, j, i, (i * 7 + j * 13) % 8);
                        break;
                }
            }
        }

        // Рисуем игрока
        DrawPlayer(res.hdcBuffer, centerX, centerY, TILE_SIZE, playerColor);

        // Интерфейс
        std::string coordText = "X: " + std::to_string(playerX) + "  Y: " + std::to_string(playerY);
        TextOutA(res.hdcBuffer, 10, 10, coordText.c_str(), coordText.length());

        std::string controlsText = "WASD - Movement | SPACE - Exit";
        TextOutA(res.hdcBuffer, 10, screenHeight - 30, controlsText.c_str(), controlsText.length());

        if (Labir[playerY][playerX] == 3) {
            std::string winText = "TEMPLE ESCAPED!";
            TextOutA(res.hdcBuffer, screenWidth / 2 - 150, screenHeight / 2, winText.c_str(), winText.length());
        }

        // Вывод на экран
        BitBlt(res.hdc, 0, 0, screenWidth, screenHeight, res.hdcBuffer, 0, 0, SRCCOPY);

        // Обработка ввода
        if (_kbhit() || firstFrame) {
            firstFrame = false;
            ProcessInput(c, playerX, playerY, n, m, Labir, exitRequested);
        }

        Sleep(16);
    }

    // Очистка ресурсов
    CleanupResources(res);
    PostMessage(hConsoleWnd, WM_CLOSE, 0, 0);
}

// Оптимизированные функции рисования

void DrawWater(HDC hdc, int x, int y, int size, COLORREF color, int blockX, int blockY, int variant) {
    COLORREF waterColor = RGB(30, 60, 150);
    COLORREF darkWater = RGB(20, 40, 120);
    COLORREF lightWater = RGB(50, 100, 200);

    HBRUSH waterBrush = CreateSolidBrush(waterColor);
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, waterBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, GetStockObject(NULL_PEN)));

    Rectangle(hdc, x, y, x + size, y + size);

    unsigned int seed = textureHash(blockX, blockY, variant);

    // Волны
    HPEN wavePen = CreatePen(PS_SOLID, 2, lightWater);
    SelectObject(hdc, wavePen);

    int waveCount = 5 + textureRand(seed) % 8;
    for (int i = 0; i < waveCount; i++) {
        int waveY = y + 10 + textureRand(seed) % (size - 20);
        int waveStart = x + textureRand(seed) % (size / 4);
        int waveLength = size / 3 + textureRand(seed) % (size / 2);

        MoveToEx(hdc, waveStart, waveY, NULL);
        for (int j = 0; j < waveLength; j += 5) {
            int curve = textureRand(seed) % 5 - 2;
            LineTo(hdc, waveStart + j, waveY + curve);
        }
    }
    DeleteObject(wavePen);

    // Блики на воде
    HBRUSH highlightBrush = CreateSolidBrush(lightWater);
    SelectObject(hdc, highlightBrush);

    int highlightCount = 10 + textureRand(seed) % 15;
    for (int i = 0; i < highlightCount; i++) {
        int highlightX = x + 5 + textureRand(seed) % (size - 10);
        int highlightY = y + 5 + textureRand(seed) % (size - 10);
        int highlightSize = 3 + textureRand(seed) % 6;

        Ellipse(hdc, highlightX, highlightY,
                highlightX + highlightSize, highlightY + highlightSize);
    }

    // Восстановление исходных объектов
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(waterBrush);
    DeleteObject(highlightBrush);
}

void DrawCaveFloor(HDC hdc, int x, int y, int size, COLORREF color, int blockX, int blockY, int variant) {
    COLORREF caveFloorColor = RGB(80, 60, 50);
    HBRUSH floorBrush = CreateSolidBrush(caveFloorColor);
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, floorBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, GetStockObject(NULL_PEN)));

    Rectangle(hdc, x, y, x + size, y + size);

    unsigned int seed = textureHash(blockX, blockY, variant);

    // Камни и неровности
    int stoneCount = 15 + textureRand(seed) % 20;
    for (int i = 0; i < stoneCount; i++) {
        int stoneX = x + 2 + textureRand(seed) % (size - 4);
        int stoneY = y + 2 + textureRand(seed) % (size - 4);
        int stoneSize = 2 + textureRand(seed) % 5;

        COLORREF stoneColor = RGB(
            60 + textureRand(seed) % 40,
            40 + textureRand(seed) % 30,
            30 + textureRand(seed) % 25
        );

        HBRUSH stoneBrush = CreateSolidBrush(stoneColor);
        SelectObject(hdc, stoneBrush);
        Ellipse(hdc, stoneX, stoneY, stoneX + stoneSize, stoneY + stoneSize);
        DeleteObject(stoneBrush);
    }

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(floorBrush);
}

void DrawCaveWall(HDC hdc, int x, int y, int size, COLORREF color, int blockX, int blockY, int variant) {
    COLORREF caveWallColor = RGB(60, 45, 35);
    HBRUSH wallBrush = CreateSolidBrush(caveWallColor);
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, wallBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, GetStockObject(NULL_PEN)));

    Rectangle(hdc, x, y, x + size, y + size);

    unsigned int seed = textureHash(blockX, blockY, variant);

    // Сталактиты/сталагмиты
    HPEN formationPen = CreatePen(PS_SOLID, 3, RGB(70, 55, 45));
    SelectObject(hdc, formationPen);

    int formationCount = 5 + textureRand(seed) % 8;
    for (int i = 0; i < formationCount; i++) {
        int startX = x + 5 + textureRand(seed) % (size - 10);
        int length = 5 + textureRand(seed) % 15;

        if (textureRand(seed) % 2 == 0) {
            // Сталактит (сверху)
            MoveToEx(hdc, startX, y + 2, NULL);
            LineTo(hdc, startX, y + 2 + length);
        } else {
            // Сталагмит (снизу)
            MoveToEx(hdc, startX, y + size - 2, NULL);
            LineTo(hdc, startX, y + size - 2 - length);
        }
    }

    DeleteObject(formationPen);
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(wallBrush);
}

void DrawMountain(HDC hdc, int x, int y, int size, COLORREF color, int blockX, int blockY, int variant) {
    COLORREF mountainColor = RGB(120, 100, 90);
    COLORREF snowColor = RGB(220, 220, 230);

    HBRUSH mountainBrush = CreateSolidBrush(mountainColor);
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, mountainBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, GetStockObject(NULL_PEN)));

    Rectangle(hdc, x, y, x + size, y + size);

    unsigned int seed = textureHash(blockX, blockY, variant);

    // Скалистая текстура
    HPEN rockPen = CreatePen(PS_SOLID, 2, RGB(100, 80, 70));
    SelectObject(hdc, rockPen);

    int rockCount = 20 + textureRand(seed) % 30;
    for (int i = 0; i < rockCount; i++) {
        int startX = x + 3 + textureRand(seed) % (size - 6);
        int startY = y + 3 + textureRand(seed) % (size - 6);
        int length = 4 + textureRand(seed) % 12;

        MoveToEx(hdc, startX, startY, NULL);
        for (int j = 0; j < length; j++) {
            int dirX = textureRand(seed) % 3 - 1;
            int dirY = textureRand(seed) % 3 - 1;
            LineTo(hdc, startX + dirX, startY + dirY);
        }
    }
    DeleteObject(rockPen);

    // Снежные вершины
    HBRUSH snowBrush = CreateSolidBrush(snowColor);
    SelectObject(hdc, snowBrush);

    int snowCount = 3 + textureRand(seed) % 5;
    for (int i = 0; i < snowCount; i++) {
        int snowX = x + textureRand(seed) % (size / 2);
        int snowY = y + textureRand(seed) % (size / 3);
        int snowSize = 4 + textureRand(seed) % 8;

        Ellipse(hdc, snowX, snowY, snowX + snowSize, snowY + snowSize);
    }

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(mountainBrush);
    DeleteObject(snowBrush);
}

void DrawDesertFloor(HDC hdc, int x, int y, int size, COLORREF color, int blockX, int blockY, int variant) {
    COLORREF desertColor = RGB(210, 180, 140);
    HBRUSH desertBrush = CreateSolidBrush(desertColor);
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, desertBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, GetStockObject(NULL_PEN)));

    Rectangle(hdc, x, y, x + size, y + size);

    unsigned int seed = textureHash(blockX, blockY, variant);

    // Песчаные волны
    HPEN sandPen = CreatePen(PS_SOLID, 2, RGB(190, 160, 120));
    SelectObject(hdc, sandPen);

    int waveCount = 8 + textureRand(seed) % 12;
    for (int i = 0; i < waveCount; i++) {
        int waveY = y + 10 + textureRand(seed) % (size - 20);
        int waveStart = x + textureRand(seed) % (size / 4);
        int waveLength = size / 2 + textureRand(seed) % (size / 2);

        MoveToEx(hdc, waveStart, waveY, NULL);
        for (int j = 0; j < waveLength; j += 4) {
            int curve = textureRand(seed) % 3 - 1;
            LineTo(hdc, waveStart + j, waveY + curve);
        }
    }
    DeleteObject(sandPen);

    // Песчинки
    int grainCount = 50 + textureRand(seed) % 100;
    for (int i = 0; i < grainCount; i++) {
        int grainX = x + textureRand(seed) % size;
        int grainY = y + textureRand(seed) % size;
        COLORREF grainColor = RGB(
            180 + textureRand(seed) % 40,
            150 + textureRand(seed) % 40,
            110 + textureRand(seed) % 40
        );
        SetPixel(hdc, grainX, grainY, grainColor);
    }

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(desertBrush);
}

void DrawDesertWall(HDC hdc, int x, int y, int size, COLORREF color, int blockX, int blockY, int variant) {
    COLORREF desertWallColor = RGB(180, 150, 110);
    HBRUSH wallBrush = CreateSolidBrush(desertWallColor);
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, wallBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, GetStockObject(NULL_PEN)));

    Rectangle(hdc, x, y, x + size, y + size);

    unsigned int seed = textureHash(blockX, blockY, variant);

    // Трещины в скалах
    HPEN crackPen = CreatePen(PS_SOLID, 2, RGB(150, 120, 90));
    SelectObject(hdc, crackPen);

    int crackCount = 5 + textureRand(seed) % 10;
    for (int i = 0; i < crackCount; i++) {
        int startX = x + 5 + textureRand(seed) % (size - 10);
        int startY = y + 5 + textureRand(seed) % (size - 10);
        int length = 8 + textureRand(seed) % 20;

        MoveToEx(hdc, startX, startY, NULL);
        for (int j = 0; j < length; j++) {
            int dirX = textureRand(seed) % 3 - 1;
            int dirY = textureRand(seed) % 3 - 1;
            LineTo(hdc, startX + dirX, startY + dirY);
        }
    }
    DeleteObject(crackPen);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(wallBrush);
}

void DrawExitPoint(HDC hdc, int x, int y, int size, COLORREF color) {
    int centerX = x + size / 2;
    int centerY = y + size / 2;
    int radius = size * 0.3;

    // Внешний круг
    HBRUSH outerBrush = CreateSolidBrush(RGB(100, 200, 100));
    HPEN outerPen = CreatePen(PS_SOLID, 3, RGB(80, 180, 80));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, outerBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, outerPen));

    Ellipse(hdc, centerX - radius, centerY - radius,
            centerX + radius, centerY + radius);

    // Внутренний круг
    HBRUSH innerBrush = CreateSolidBrush(RGB(150, 250, 150));
    SelectObject(hdc, innerBrush);
    Ellipse(hdc, centerX - radius/1.8, centerY - radius/1.8,
            centerX + radius/1.8, centerY + radius/1.8);

    // Центральная точка
    HBRUSH centerBrush = CreateSolidBrush(RGB(200, 255, 200));
    SelectObject(hdc, centerBrush);
    Ellipse(hdc, centerX - radius/3, centerY - radius/3,
            centerX + radius/3, centerY + radius/3);

    // Свечение
    for (int i = 0; i < 25; i++) {
        int px = centerX - radius + rand() % (radius * 2);
        int py = centerY - radius + rand() % (radius * 2);

        if ((px - centerX) * (px - centerX) + (py - centerY) * (py - centerY) <= radius * radius) {
            SetPixel(hdc, px, py, RGB(180, 255, 180));
        }
    }

    // Восстановление исходных объектов
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(outerBrush);
    DeleteObject(outerPen);
    DeleteObject(innerBrush);
    DeleteObject(centerBrush);
}

void DrawWall(HDC hdc, int x, int y, int size, COLORREF color, int blockX, int blockY, int variant) {
    // Базовый цвет стены
    COLORREF wallBase = RGB(100, 80, 60);
    COLORREF wallDark = RGB(80, 60, 40);
    COLORREF wallLight = RGB(120, 100, 80);

    HBRUSH wallBrush = CreateSolidBrush(wallBase);
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, wallBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, GetStockObject(NULL_PEN)));

    Rectangle(hdc, x, y, x + size, y + size);

    unsigned int seed = textureHash(blockX, blockY, variant);

    // Кирпичная кладка - более четкая и структурированная
    if (variant % 2 == 0) {
        int brickWidth = size / 4;
        int brickHeight = size / 8;
        int mortarSize = 2;

        HPEN mortarPen = CreatePen(PS_SOLID, mortarSize, wallDark);
        SelectObject(hdc, mortarPen);

        // Горизонтальные линии
        for (int row = y + brickHeight; row < y + size; row += brickHeight) {
            MoveToEx(hdc, x, row, NULL);
            LineTo(hdc, x + size, row);
        }

        // Вертикальные линии со смещением
        for (int row = y; row < y + size; row += brickHeight) {
            int offset = (row / brickHeight) % 2 == 0 ? 0 : brickWidth / 2;
            for (int col = x + offset; col < x + size; col += brickWidth) {
                MoveToEx(hdc, col, std::max(y, row), NULL);
                LineTo(hdc, col, std::min(y + size, row + brickHeight));
            }
        }

        DeleteObject(mortarPen);
    }

    // Каменная текстура - более натуральная
    else {
        HBRUSH stoneBrush = CreateSolidBrush(wallLight);
        SelectObject(hdc, stoneBrush);

        // Крупные камни
        int stoneCount = 8 + textureRand(seed) % 6;
        for (int i = 0; i < stoneCount; i++) {
            int stoneX = x + 5 + textureRand(seed) % (size - 10);
            int stoneY = y + 5 + textureRand(seed) % (size - 10);
            int stoneW = 8 + textureRand(seed) % 12;
            int stoneH = 6 + textureRand(seed) % 10;

            // Немного неправильная форма для натуральности
            POINT stone[6] = {
                {stoneX, stoneY},
                {stoneX + stoneW, stoneY},
                {stoneX + stoneW + 2, stoneY + stoneH/2},
                {stoneX + stoneW, stoneY + stoneH},
                {stoneX, stoneY + stoneH},
                {stoneX - 2, stoneY + stoneH/2}
            };

            Polygon(hdc, stone, 6);
        }
        DeleteObject(stoneBrush);
    }

    // Тени и блики для объема
    HPEN shadowPen = CreatePen(PS_SOLID, 1, wallDark);
    HPEN highlightPen = CreatePen(PS_SOLID, 1, wallLight);

    // Тени снизу и справа
    SelectObject(hdc, shadowPen);
    for (int i = 0; i < 15; i++) {
        int lineX = x + textureRand(seed) % size;
        int lineY = y + textureRand(seed) % size;
        MoveToEx(hdc, lineX, lineY, NULL);
        LineTo(hdc, lineX + 3, lineY + 3);
    }

    // Блики сверху и слева
    SelectObject(hdc, highlightPen);
    for (int i = 0; i < 12; i++) {
        int lineX = x + textureRand(seed) % size;
        int lineY = y + textureRand(seed) % size;
        MoveToEx(hdc, lineX, lineY, NULL);
        LineTo(hdc, lineX - 2, lineY - 2);
    }

    // Шероховатости
    int roughnessCount = 30 + textureRand(seed) % 40;
    for (int i = 0; i < roughnessCount; i++) {
        int px = x + 2 + textureRand(seed) % (size - 4);
        int py = y + 2 + textureRand(seed) % (size - 4);

        COLORREF pixelColor = textureRand(seed) % 100 < 70 ? wallDark : wallLight;
        SetPixel(hdc, px, py, pixelColor);
    }

    DeleteObject(shadowPen);
    DeleteObject(highlightPen);
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(wallBrush);
}

void DrawFloor(HDC hdc, int x, int y, int size, COLORREF color, int blockX, int blockY, int variant) {
    HBRUSH wallBrush = CreateSolidBrush(color);
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, wallBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, GetStockObject(NULL_PEN)));

    Rectangle(hdc, x, y, x + size, y + size);

    int step = 4;
    for (int py = y; py < y + size; py += step) {
        for (int px = x; px < x + size; px += step) {
            unsigned int seed = textureHash(blockX, blockY,
                                          (px - x) + (py - y) * size + variant * 1000);

            if (textureRand(seed) % 100 < 70) {
                int grainSize = 1 + textureRand(seed) % 3;
                int offsetX = textureRand(seed) % step;
                int offsetY = textureRand(seed) % step;

                int variation = 30 - textureRand(seed) % 60;
                COLORREF grainColor = RGB(
                    std::min(255, std::max(0, GetRValue(color) + variation)),
                    std::min(255, std::max(0, GetGValue(color) + variation)),
                    std::min(255, std::max(0, GetBValue(color) + variation))
                );

                HBRUSH grainBrush = CreateSolidBrush(grainColor);
                SelectObject(hdc, grainBrush);
                Rectangle(hdc, px + offsetX, py + offsetY,
                         px + offsetX + grainSize, py + offsetY + grainSize);
                DeleteObject(grainBrush);
            }
        }
    }

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(wallBrush);
}

void DrawPlayer(HDC hdc, int x, int y, int size, COLORREF color) {
    int radius = size * 0.3;
    int centerX = x + size / 2;
    int centerY = y + size / 2;

    HBRUSH playerBrush = CreateSolidBrush(color);
    HPEN playerPen = CreatePen(PS_SOLID, 2, RGB(180, 140, 80));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, playerBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, playerPen));

    Ellipse(hdc, centerX - radius, centerY - radius,
            centerX + radius, centerY + radius);

    HBRUSH highlightBrush = CreateSolidBrush(RGB(255, 230, 200));
    SelectObject(hdc, highlightBrush);
    SelectObject(hdc, GetStockObject(NULL_PEN));
    Ellipse(hdc, centerX - radius/3, centerY - radius/3,
            centerX - radius/6, centerY - radius/6);

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(playerBrush);
    DeleteObject(playerPen);
    DeleteObject(highlightBrush);
}

void DrawExit(HDC hdc, int x, int y, int size, COLORREF color) {
    int centerX = x + size / 2;
    int centerY = y + size / 2;
    int radius = size * 0.35;

    HBRUSH outerBrush = CreateSolidBrush(RGB(180, 70, 70));
    HPEN outerPen = CreatePen(PS_SOLID, 3, RGB(150, 50, 50));
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, outerBrush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, outerPen));

    Ellipse(hdc, centerX - radius, centerY - radius,
            centerX + radius, centerY + radius);

    HBRUSH innerBrush = CreateSolidBrush(RGB(220, 100, 100));
    SelectObject(hdc, innerBrush);
    Ellipse(hdc, centerX - radius/1.5, centerY - radius/1.5,
            centerX + radius/1.5, centerY + radius/1.5);

    HBRUSH centerBrush = CreateSolidBrush(RGB(255, 150, 150));
    SelectObject(hdc, centerBrush);
    Ellipse(hdc, centerX - radius/3, centerY - radius/3,
            centerX + radius/3, centerY + radius/3);

    for (int i = 0; i < 20; i++) {
        int px = centerX - radius + rand() % (radius * 2);
        int py = centerY - radius + rand() % (radius * 2);

        if ((px - centerX) * (px - centerX) + (py - centerY) * (py - centerY) <= radius * radius) {
            SetPixel(hdc, px, py, RGB(255, 200, 200));
        }
    }

    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(outerBrush);
    DeleteObject(outerPen);
    DeleteObject(innerBrush);
    DeleteObject(centerBrush);
}

#endif // VISUALMAPING_H_INCLUDED
