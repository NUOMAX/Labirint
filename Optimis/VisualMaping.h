#ifndef VISUALMAPING_H_INCLUDED
#define VISUALMAPING_H_INCLUDED

void DrawWall(HDC hdc, int x, int y, int size, COLORREF color, int blockX, int blockY, int variant);
void DrawFloor(HDC hdc, int x, int y, int size, COLORREF color);
void DrawPlayer(HDC hdc, int x, int y, int size, COLORREF color);
void DrawExit(HDC hdc, int x, int y, int size, COLORREF color);

void VisualMap(int playerX, int playerY, int n, int m, int** Labir, wchar_t c)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    HWND HConsole = GetConsoleWindow();

    RECT consoleRect;
    GetWindowRect(HConsole, &consoleRect);

    MoveWindow(HConsole, consoleRect.left, consoleRect.top, 1000, 1000, TRUE);

    Sleep(100);

    HDC hdc = GetDC(HConsole);

    HDC hdcBuffer = CreateCompatibleDC(hdc);
    GetClientRect(HConsole, &consoleRect);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdc, consoleRect.right, consoleRect.bottom);
    SelectObject(hdcBuffer, hBitmap);

    HFONT hFont = CreateFontA(28, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
    DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Consolas");
    SelectObject(hdcBuffer, hFont);
    SetTextColor(hdcBuffer, RGB(255, 255, 200));
    SetBkColor(hdcBuffer, RGB(0, 0, 0));

    const int TILE_SIZE = 120;

    COLORREF wallColor = RGB(88, 72, 56);
    COLORREF floorColor = RGB(64, 56, 48);
    COLORREF darkWallColor = RGB(64, 52, 40);
    COLORREF playerColor = RGB(255, 200, 100);
    COLORREF exitColor = RGB(200, 80, 80);

    HBRUSH hBackgroundBrush = CreateSolidBrush(RGB(16, 12, 24));

    int screenWidth = consoleRect.right;
    int screenHeight = consoleRect.bottom;

    bool firstFrame = true;

    while (true)
    {
        FillRect(hdcBuffer, &consoleRect, hBackgroundBrush);

        int centerX = screenWidth / 2;
        int centerY = screenHeight / 2;
        int viewRadius = 400;

        for (int i = playerY - 10; i <= playerY + 10; i++)
        {
            for (int j = playerX - 10; j <= playerX + 10; j++)
            {
                int tileX = centerX + (j - playerX) * TILE_SIZE;
                int tileY = centerY + (i - playerY) * TILE_SIZE;

                int dx = tileX - centerX;
                int dy = tileY - centerY;
                if (dx*dx + dy*dy > viewRadius*viewRadius) {
                    continue;
                }

                if (tileX < -TILE_SIZE || tileX > screenWidth ||
                    tileY < -TILE_SIZE || tileY > screenHeight) {
                    continue;
                }

                if (!inBounds(i, j, n, m))
                {
                    DrawWall(hdcBuffer, tileX, tileY, TILE_SIZE, darkWallColor, j, i, (i+j) % 8);
                    continue;
                }

                if (Labir[i][j] == 0)
                {
                    DrawFloor(hdcBuffer, tileX, tileY, TILE_SIZE, floorColor);
                    //DrawShadow(hdcBuffer, tileX, tileY, TILE_SIZE);
                }
                else if (Labir[i][j] == 2)
                {
                    DrawFloor(hdcBuffer, tileX, tileY, TILE_SIZE, floorColor);
                    DrawExit(hdcBuffer, tileX, tileY, TILE_SIZE, exitColor);
                }
                else
                {
                    int variant = (i * 7 + j * 13) % 8;
                    DrawWall(hdcBuffer, tileX, tileY, TILE_SIZE, wallColor, j, i, variant);
                }
            }
        }
        int playerTileX = centerX;
        int playerTileY = centerY;
        DrawPlayer(hdcBuffer, playerTileX, playerTileY, TILE_SIZE, playerColor);

        std::string coordText = "X: " + std::to_string(playerX) + "  Y: " + std::to_string(playerY);
        TextOutA(hdcBuffer, 10, 10, coordText.c_str(), coordText.length());

        std::string controlsText = "WASD - Äâèæåíèå | ÏÐÎÁÅË - Âûõîä";
        TextOutA(hdcBuffer, 10, screenHeight - 30, controlsText.c_str(), controlsText.length());

        if (Labir[playerY][playerX] == 2)
        {
            std::string winText = "ÕÐÀÌ ÏÎÊÈÍÓÒ!";
            TextOutA(hdcBuffer, screenWidth/2 - 150, screenHeight/2, winText.c_str(), winText.length());
        }

        BitBlt(hdc, 0, 0, screenWidth, screenHeight, hdcBuffer, 0, 0, SRCCOPY);
        if (_kbhit() || firstFrame) {
            firstFrame = false;

            if (_kbhit()) {
                c = _getwch();
                if (c == ' ') break;
                if(Labir[playerY][playerX] == 2)
                    continue;
                int newY = playerY, newX = playerX;
                switch (c)
                {
                    case 'w': case 'W': case 'ö': case 'Ö': newY--; break;
                    case 's': case 'S': case 'û': case 'Û': newY++; break;
                    case 'a': case 'A': case 'ô': case 'Ô': newX--; break;
                    case 'd': case 'D': case 'â': case 'Â': newX++; break;
                }

                if (inBounds(newY, newX, n, m) && Labir[newY][newX] != 1)
                {
                    playerY = newY;
                    playerX = newX;
                }
            }
        }

        Sleep(16);
    }

    DeleteObject(hFont);
    DeleteObject(hBitmap);
    DeleteDC(hdcBuffer);
    DeleteObject(hBackgroundBrush);
    ReleaseDC(HConsole, hdc);
}

unsigned int textureHash(int x, int y, int variant) {
    unsigned int hash = (x * 73856093) ^ (y * 19349663) ^ (variant * 83492791);
    hash = (hash >> 16) ^ (hash & 0xFFFF);
    return hash;
}

int textureRand(unsigned int &seed) {
    seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    return seed;
}

void DrawWall(HDC hdc, int x, int y, int size, COLORREF color, int blockX, int blockY, int variant)
{
    HBRUSH backgroundBrush = CreateSolidBrush(color);
    SelectObject(hdc, backgroundBrush);
    SelectObject(hdc, GetStockObject(NULL_PEN));
    Rectangle(hdc, x, y, x + size, y + size);
    unsigned int seed = textureHash(blockX, blockY, variant);

    int numPixels = 60 + textureRand(seed) % 80;
    int numLines = 15 + textureRand(seed) % 20;
    int numShapes = 3 + textureRand(seed) % 6;
    int numCracks = 2 + textureRand(seed) % 5;

    COLORREF darkColor = RGB(
        std::max(0, GetRValue(color) - 30),
        std::max(0, GetGValue(color) - 25),
        std::max(0, GetBValue(color) - 20)
    );

    COLORREF lightColor = RGB(
        std::min(255, GetRValue(color) + 30),
        std::min(255, GetGValue(color) + 25),
        std::min(255, GetBValue(color) + 20)
    );

    HBRUSH darkBrush = CreateSolidBrush(darkColor);
    HBRUSH lightBrush = CreateSolidBrush(lightColor);

    if (variant % 3 == 0) {
        int brickWidth = size / 4 + textureRand(seed) % (size / 8);
        int brickHeight = size / 8 + textureRand(seed) % (size / 16);

        HPEN mortarPen = CreatePen(PS_SOLID, 1, darkColor);
        SelectObject(hdc, mortarPen);

        for (int row = 0; row < size; row += brickHeight) {
            for (int col = 0; col < size; col += brickWidth) {
                int offset = (row / brickHeight) % 2 == 0 ? 0 : brickWidth / 2;

                if (col + offset < size) {
                    int drawX = x + col + offset;
                    int drawY = y + row;

                    if (drawX < x + size && drawY < y + size) {
                        if (drawY < y + size) {
                            MoveToEx(hdc, std::max(x, drawX), drawY, NULL);
                            LineTo(hdc, std::min(x + size, drawX + brickWidth), drawY);
                        }
                        if (drawX < x + size && row < size) {
                            MoveToEx(hdc, drawX, std::max(y, drawY), NULL);
                            LineTo(hdc, drawX, std::min(y + size, drawY + brickHeight));
                        }
                    }
                }
            }
        }
        DeleteObject(mortarPen);
    }
    if (variant % 3 == 1) {
        int stoneSize = size / 6 + textureRand(seed) % (size / 12);

        for (int i = 0; i < 15; i++) {
            int stoneX = x + 5 + textureRand(seed) % (size - 10);
            int stoneY = y + 5 + textureRand(seed) % (size - 10);
            stoneX = std::max(x + 2, std::min(x + size - stoneSize - 2, stoneX));
            stoneY = std::max(y + 2, std::min(y + size - stoneSize - 2, stoneY));

            POINT points[4] = {
                { stoneX, stoneY },
                { stoneX + stoneSize/2, stoneY - stoneSize/4 },
                { stoneX + stoneSize, stoneY },
                { stoneX + stoneSize/2, stoneY + stoneSize/2 }
            };

            Polygon(hdc, points, 4);
        }
    }
    for (int i = 0; i < numCracks; i++) {
        int startX = x + 2 + textureRand(seed) % (size - 4);
        int startY = y + 2 + textureRand(seed) % (size - 4);
        int length = 5 + textureRand(seed) % 15;

        HPEN crackPen = CreatePen(PS_SOLID, 1, darkColor);
        SelectObject(hdc, crackPen);

        MoveToEx(hdc, startX, startY, NULL);
        for (int j = 0; j < length; j++) {
            int dirX = textureRand(seed) % 3 - 1;
            int dirY = textureRand(seed) % 3 - 1;

            startX += dirX;
            startY += dirY;
            startX = std::max(x + 1, std::min(x + size - 2, startX));
            startY = std::max(y + 1, std::min(y + size - 2, startY));

            LineTo(hdc, startX, startY);
        }

        DeleteObject(crackPen);
    }

    for (int i = 0; i < 8 + textureRand(seed) % 12; i++) {
        int spotX = x + 3 + textureRand(seed) % (size - 6);
        int spotY = y + 3 + textureRand(seed) % (size - 6);
        int spotSize = 2 + textureRand(seed) % 4;

        COLORREF spotColor = textureRand(seed) % 100 < 70 ? darkColor : lightColor;

        for (int dx = -spotSize; dx <= spotSize; dx++) {
            for (int dy = -spotSize; dy <= spotSize; dy++) {
                if (dx*dx + dy*dy <= spotSize*spotSize) {
                    int px = spotX + dx;
                    int py = spotY + dy;
                    if (px >= x && px < x + size && py >= y && py < y + size) {
                        SetPixel(hdc, px, py, spotColor);
                    }
                }
            }
        }
    }

    for (int i = 0; i < numPixels; i++) {
        int px = x + 2 + textureRand(seed) % (size - 4);
        int py = y + 2 + textureRand(seed) % (size - 4);

        if (px >= x && px < x + size && py >= y && py < y + size) {
            if (textureRand(seed) % 100 < 60) {
                SetPixel(hdc, px, py, RGB(
                    std::max(0, GetRValue(color) - 20 - textureRand(seed) % 35),
                    std::max(0, GetGValue(color) - 15 - textureRand(seed) % 30),
                    std::max(0, GetBValue(color) - 10 - textureRand(seed) % 25)
                ));
            }
            else if (textureRand(seed) % 100 < 80) {
                SetPixel(hdc, px, py, RGB(
                    std::min(255, GetRValue(color) + 15 + textureRand(seed) % 30),
                    std::min(255, GetGValue(color) + 10 + textureRand(seed) % 25),
                    std::min(255, GetBValue(color) + 5 + textureRand(seed) % 20)
                ));
            }
        }
    }

    for (int i = 0; i < numLines; i++) {
        int startX = x + 2 + textureRand(seed) % (size - 4);
        int startY = y + 2 + textureRand(seed) % (size - 4);
        int length = 3 + textureRand(seed) % 15;
        int thickness = 1 + textureRand(seed) % 2;
        COLORREF lineColor = textureRand(seed) % 100 < 70 ? darkColor : lightColor;
        HPEN linePen = CreatePen(PS_SOLID, thickness, lineColor);
        SelectObject(hdc, linePen);

        int endX = startX;
        int endY = startY;

        MoveToEx(hdc, startX, startY, NULL);
        for (int j = 0; j < length; j++) {
            int dirX = textureRand(seed) % 3 - 1;
            int dirY = textureRand(seed) % 3 - 1;

            endX += dirX;
            endY += dirY;

            endX = std::max(x + 1, std::min(x + size - 2, endX));
            endY = std::max(y + 1, std::min(y + size - 2, endY));

            LineTo(hdc, endX, endY);
        }

        DeleteObject(linePen);
    }

    for (int i = 0; i < numShapes; i++) {
        int shapeX = x + 3 + textureRand(seed) % (size - 6);
        int shapeY = y + 3 + textureRand(seed) % (size - 6);
        int shapeSize = 2 + textureRand(seed) % 5;

        shapeX = std::max(x + 2, std::min(x + size - shapeSize - 2, shapeX));
        shapeY = std::max(y + 2, std::min(y + size - shapeSize - 2, shapeY));

        if (textureRand(seed) % 100 < 50) {
            Rectangle(hdc, shapeX, shapeY, shapeX + shapeSize, shapeY + shapeSize);
        } else {
            Ellipse(hdc, shapeX, shapeY, shapeX + shapeSize, shapeY + shapeSize);
        }
    }

    DeleteObject(darkBrush);
    DeleteObject(lightBrush);
    DeleteObject(backgroundBrush);
}

void DrawFloor(HDC hdc, int x, int y, int size, COLORREF baseColor)
{
    HBRUSH floorBrush = CreateSolidBrush(baseColor);
    SelectObject(hdc, floorBrush);
    SelectObject(hdc, GetStockObject(NULL_PEN));
    Rectangle(hdc, x, y, x + size, y + size);

    int step = 4;
    int index = 0;

    for (int py = y; py < y + size; py += step)
    {
        for (int px = x; px < x + size; px += step)
        {
            unsigned int seed = textureHash(px, py, index++);

            if (textureRand(seed) % 100 < 70)
            {
                int grainSize = 1 + textureRand(seed) % 2;
                int offsetX = textureRand(seed) % step;
                int offsetY = textureRand(seed) % step;

                int variation = 25 - textureRand(seed) % 50;
                COLORREF grainColor = RGB(
                    std::min(255, std::max(0, GetRValue(baseColor) + variation)),
                    std::min(255, std::max(0, GetGValue(baseColor) + variation)),
                    std::min(255, std::max(0, GetBValue(baseColor) + variation))
                );

                HBRUSH grainBrush = CreateSolidBrush(grainColor);
                SelectObject(hdc, grainBrush);
                Ellipse(hdc, px + offsetX, py + offsetY,
                        px + offsetX + grainSize, py + offsetY + grainSize);
                DeleteObject(grainBrush);
            }
        }
    }

    DeleteObject(floorBrush);
}

void DrawPlayer(HDC hdc, int x, int y, int size, COLORREF color)
{
    int radius = size * 0.3;
    int centerX = x + size / 2;
    int centerY = y + size / 2;

    HBRUSH playerBrush = CreateSolidBrush(color);
    HPEN playerPen = CreatePen(PS_SOLID, 2, RGB(180, 140, 80));

    SelectObject(hdc, playerBrush);
    SelectObject(hdc, playerPen);

    Ellipse(hdc, centerX - radius, centerY - radius,
            centerX + radius, centerY + radius);

    HBRUSH highlightBrush = CreateSolidBrush(RGB(255, 230, 200));
    SelectObject(hdc, highlightBrush);
    SelectObject(hdc, GetStockObject(NULL_PEN));
    Ellipse(hdc, centerX - radius/3, centerY - radius/3,
            centerX - radius/6, centerY - radius/6);

    DeleteObject(playerBrush);
    DeleteObject(playerPen);
    DeleteObject(highlightBrush);
}

void DrawExit(HDC hdc, int x, int y, int size, COLORREF color)
{
    int centerX = x + size / 2;
    int centerY = y + size / 2;
    int radius = size * 0.35;

    HBRUSH outerBrush = CreateSolidBrush(RGB(180, 70, 70));
    HPEN outerPen = CreatePen(PS_SOLID, 3, RGB(150, 50, 50));

    SelectObject(hdc, outerBrush);
    SelectObject(hdc, outerPen);
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

    DeleteObject(outerBrush);
    DeleteObject(outerPen);
    DeleteObject(innerBrush);
    DeleteObject(centerBrush);
}

#endif // VISUALMAPING_H_INCLUDED
