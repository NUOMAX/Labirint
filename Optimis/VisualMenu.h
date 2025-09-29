#ifndef VISUALMENU_H_INCLUDED
#define VISUALMENU_H_INCLUDED

// Функция для установки полноэкранного режима без рамки
void SetFullscreenNoBorder() {
    HWND hConsole = GetConsoleWindow();

    // Получаем размеры экрана
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Убираем рамку и заголовок
    SetWindowLong(hConsole, GWL_STYLE, WS_POPUP | WS_VISIBLE);

    // Устанавливаем на весь экран
    SetWindowPos(hConsole, HWND_TOP, 0, 0, screenWidth, screenHeight, SWP_SHOWWINDOW);
}

class OptimizedMenu {
private:
    HWND console;
    HDC hdc;
    HDC hdcBuffer;
    HBITMAP hBitmap;
    HFONT hFont;
    HFONT hSmallFont;
    HBRUSH hBackgroundBrush;
    HBRUSH hMenuBrush;
    HBRUSH hSelectedBrush;

    int width, height;
    int selectedItem;
    vector<wstring> menuItems;
    int animationFrame;
    bool inSubMenu;
    wstring subMenuTitle;
    vector<wstring> subMenuItems;
    int subMenuSelected;

    // Буферы для ввода текста
    wstring inputN, inputM, inputSeed;
    int inputField;

    // Переменные для управления временем и вводом
    long long lastInputTime;
    long long lastFrameTime;
    const long long INPUT_COOLDOWN = 150; // ms
    const long long FRAME_TIME = 16; // ~60 FPS

public:
    OptimizedMenu() {
        console = GetConsoleWindow();
        hdc = GetDC(console);

        RECT consoleRect;
        GetClientRect(console, &consoleRect);
        width = consoleRect.right;
        height = consoleRect.bottom;

        hdcBuffer = CreateCompatibleDC(hdc);
        hBitmap = CreateCompatibleBitmap(hdc, width, height);
        SelectObject(hdcBuffer, hBitmap);

        // Основной шрифт для меню
        hFont = CreateFontW(42, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            RUSSIAN_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial");

        // Мелкий шрифт для текста
        hSmallFont = CreateFontW(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            RUSSIAN_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial");

        SelectObject(hdcBuffer, hFont);

        hBackgroundBrush = CreateSolidBrush(RGB(10, 8, 20));
        hMenuBrush = CreateSolidBrush(RGB(40, 35, 60));
        hSelectedBrush = CreateSolidBrush(RGB(80, 70, 120));

        selectedItem = 0;
        animationFrame = 0;
        inSubMenu = false;
        subMenuSelected = 0;
        inputField = 0;

        inputN = L"40";
        inputM = L"40";
        inputSeed = to_wstring(time(NULL));

        menuItems = {
            L"\u041D\u041E\u0412\u0410\u042F \u0418\u0413\u0420\u0410",
            L"\u041D\u0410\u0421\u0422\u0420\u041E\u0419\u041A\u0418",
            L"\u0420\u0415\u041A\u041E\u0420\u0414\u042B",
            L"\u041E\u0411 \u0410\u0412\u0422\u041E\u0420\u0415",
            L"\u0412\u042B\u0425\u041E\u0414"
        };

        lastInputTime = 0;
        lastFrameTime = 0;
    }

    ~OptimizedMenu() {
        CleanupResources();
    }

    void CleanupResources() {
        if (hFont) DeleteObject(hFont);
        if (hSmallFont) DeleteObject(hSmallFont);
        if (hBitmap) DeleteObject(hBitmap);
        if (hdcBuffer) DeleteDC(hdcBuffer);
        if (hBackgroundBrush) DeleteObject(hBackgroundBrush);
        if (hMenuBrush) DeleteObject(hMenuBrush);
        if (hSelectedBrush) DeleteObject(hSelectedBrush);
        if (hdc) ReleaseDC(console, hdc);
    }

    long long GetCurrentTime() {
        return chrono::duration_cast<chrono::milliseconds>(
            chrono::steady_clock::now().time_since_epoch()).count();
    }

    void ClearScreen() {
        RECT screenRect = {0, 0, width, height};
        FillRect(hdcBuffer, &screenRect, hBackgroundBrush);
    }

    void DrawText(int x, int y, const wstring& text, COLORREF color, bool small = false) {
        if (small) {
            SelectObject(hdcBuffer, hSmallFont);
        } else {
            SelectObject(hdcBuffer, hFont);
        }
        SetTextColor(hdcBuffer, color);
        SetBkMode(hdcBuffer, TRANSPARENT);
        TextOutW(hdcBuffer, x, y, text.c_str(), static_cast<int>(text.length()));
    }

    void DrawAnimatedBackground() {
        const int PARTICLE_COUNT = 25;

        for (int i = 0; i < PARTICLE_COUNT; i++) {
            int x = (animationFrame * 1 + i * 67) % width;
            int y = height / 4 + sin(animationFrame * 0.03 + i * 0.7) * 80;
            int size = 3 + (i % 2);

            COLORREF color = RGB(
                100 + sin(animationFrame * 0.05 + i) * 100,
                80 + cos(animationFrame * 0.03 + i * 1.5) * 80,
                150 + sin(animationFrame * 0.06 + i * 2) * 100
            );

            HBRUSH particleBrush = CreateSolidBrush(color);
            HPEN particlePen = CreatePen(PS_SOLID, 1, color);
            HPEN oldPen = (HPEN)SelectObject(hdcBuffer, particlePen);
            HBRUSH oldBrush = (HBRUSH)SelectObject(hdcBuffer, particleBrush);

            Ellipse(hdcBuffer, x - size, y - size, x + size, y + size);

            SelectObject(hdcBuffer, oldPen);
            SelectObject(hdcBuffer, oldBrush);
            DeleteObject(particleBrush);
            DeleteObject(particlePen);
        }

        for (int i = 0; i < 6; i++) {
            int x1 = (animationFrame * 2 + i * 120) % width;
            int y1 = height / 3 + cos(animationFrame * 0.02 + i) * 100;
            int x2 = x1 + 50 + sin(animationFrame * 0.05 + i) * 30;
            int y2 = y1 + 30 + cos(animationFrame * 0.04 + i) * 40;

            COLORREF color = RGB(
                150 + sin(animationFrame * 0.08 + i) * 105,
                120 + cos(animationFrame * 0.06 + i) * 135,
                200 + sin(animationFrame * 0.09 + i * 3) * 55
            );

            HPEN linePen = CreatePen(PS_SOLID, 2, color);
            HPEN oldPen = (HPEN)SelectObject(hdcBuffer, linePen);

            MoveToEx(hdcBuffer, x1, y1, NULL);
            LineTo(hdcBuffer, x2, y2);

            SelectObject(hdcBuffer, oldPen);
            DeleteObject(linePen);
        }
    }

    void DrawAnimatedTitle() {
        wstring title = L"\u041B\u0410\u0411\u0418\u0420\u0418\u041D\u0422";
        int titleWidth = title.length() * 45;
        int titleX = (width - titleWidth) / 2;
        int titleY = 60;

        for (size_t i = 0; i < title.length(); i++) {
            int charX = titleX + i * 55;
            int charY = titleY + sin(animationFrame * 0.06 + i * 1.2) * 15;

            COLORREF color = RGB(
                255,
                150 + cos(animationFrame * 0.05 + i) * 105,
                50 + sin(animationFrame * 0.08 + i * 2) * 205
            );

            wstring charStr(1, title[i]);
            DrawText(charX, charY, charStr, color);
        }
    }

    void DrawMainMenuItem(int index) {
        int centerX = width / 2;
        int itemY = 270 + index * 75;

        bool isSelected = (index == selectedItem);

        float pulse = (sin(animationFrame * 0.08f) + 1) * 0.4f;

        int textWidth = 350;
        int textHeight = 55;
        RECT itemRect = {
            centerX - textWidth / 2,
            itemY,
            centerX + textWidth / 2,
            itemY + textHeight
        };

        if (isSelected) {
            int glow = 8 + static_cast<int>(pulse * 10);
            RECT glowRect = {
                itemRect.left - glow,
                itemRect.top - glow,
                itemRect.right + glow,
                itemRect.bottom + glow
            };

            HBRUSH glowBrush = CreateSolidBrush(RGB(100, 80, 150));
            FillRect(hdcBuffer, &glowRect, glowBrush);
            DeleteObject(glowBrush);

            FillRect(hdcBuffer, &itemRect, hSelectedBrush);

            HPEN borderPen = CreatePen(PS_SOLID, 3, RGB(255, 220, 80));
            HPEN oldPen = (HPEN)SelectObject(hdcBuffer, borderPen);
            HBRUSH oldBrush = (HBRUSH)SelectObject(hdcBuffer, GetStockObject(NULL_BRUSH));

            Rectangle(hdcBuffer, itemRect.left, itemRect.top, itemRect.right, itemRect.bottom);

            SelectObject(hdcBuffer, oldPen);
            SelectObject(hdcBuffer, oldBrush);
            DeleteObject(borderPen);
        } else {
            FillRect(hdcBuffer, &itemRect, hMenuBrush);
        }

        COLORREF textColor = isSelected ? RGB(255, 255, 180) : RGB(220, 220, 220);
        wstring menuItem = menuItems[index];
        int textX = centerX - (menuItem.length() * 16);
        DrawText(textX + 30, itemY + 8, menuItem, textColor);

        if (isSelected && (animationFrame % 20 < 12)) {
            DrawText(centerX - 200, itemY + 8, L"\u00BB", RGB(255, 255, 0));
            DrawText(centerX + 182, itemY + 8, L"\u00AB", RGB(255, 255, 0));
        }
    }

    void DrawInputField(int x, int y, const wstring& label, const wstring& value, bool isSelected) {
        int fieldWidth = 350;
        int fieldHeight = 35;
        RECT fieldRect = {x, y, x + fieldWidth, y + fieldHeight};

        HBRUSH fieldBrush = CreateSolidBrush(isSelected ? RGB(60, 55, 80) : RGB(50, 45, 70));
        FillRect(hdcBuffer, &fieldRect, fieldBrush);
        DeleteObject(fieldBrush);

        HPEN borderPen = CreatePen(PS_SOLID, 2, isSelected ? RGB(255, 200, 50) : RGB(100, 90, 130));
        HPEN oldPen = (HPEN)SelectObject(hdcBuffer, borderPen);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdcBuffer, GetStockObject(NULL_BRUSH));
        Rectangle(hdcBuffer, fieldRect.left, fieldRect.top, fieldRect.right, fieldRect.bottom);
        SelectObject(hdcBuffer, oldPen);
        SelectObject(hdcBuffer, oldBrush);
        DeleteObject(borderPen);

        DrawText(x - 220, y + 5, label, RGB(200, 200, 200), true);
        DrawText(x + 10, y + 5, value, isSelected ? RGB(255, 255, 150) : RGB(200, 200, 200), true);

        if (isSelected && (animationFrame % 40 < 25)) {
            DrawText(x + 10 + value.length() * 12, y + 5, L"|", RGB(255, 255, 0), true);
        }
    }

    void DrawInputMenu() {
        HBRUSH darkenBrush = CreateSolidBrush(RGB(20, 15, 30));
        RECT screenRect = {0, 0, width, height};
        FillRect(hdcBuffer, &screenRect, darkenBrush);
        DeleteObject(darkenBrush);

        int menuWidth = 800;
        int menuHeight = 500;
        int menuX = (width - menuWidth) / 2;
        int menuY = (height - menuHeight) / 2;

        RECT menuRect = {menuX, menuY, menuX + menuWidth, menuY + menuHeight};
        FillRect(hdcBuffer, &menuRect, hMenuBrush);

        HPEN borderPen = CreatePen(PS_SOLID, 4, RGB(120, 100, 180));
        HPEN oldPen = (HPEN)SelectObject(hdcBuffer, borderPen);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdcBuffer, GetStockObject(NULL_BRUSH));
        Rectangle(hdcBuffer, menuRect.left, menuRect.top, menuRect.right, menuRect.bottom);
        SelectObject(hdcBuffer, oldPen);
        SelectObject(hdcBuffer, oldBrush);
        DeleteObject(borderPen);

        wstring title = L"\u0421\u041E\u0417\u0414\u0410\u041D\u0418\u0415 \u041D\u041E\u0412\u041E\u0419 \u0418\u0413\u0420\u042B";
        int titleWidth = title.length() * 20;
        int titleX = menuX + (menuWidth - titleWidth) / 2;
        DrawText(titleX, menuY + 30, title, RGB(255, 255, 200));

        int fieldY = menuY + 100;
        DrawInputField(menuX + 250, fieldY, L"\u0420\u0430\u0437\u043C\u0435\u0440 \u043C\u0438\u0440\u0430 N:", inputN, inputField == 0);
        DrawInputField(menuX + 250, fieldY + 60, L"\u0420\u0430\u0437\u043C\u0435\u0440 \u043C\u0438\u0440\u0430 M:", inputM, inputField == 1);
        DrawInputField(menuX + 250, fieldY + 120, L"\u0421\u0435\u0435\u0434:", inputSeed, inputField == 2);

        int buttonY = menuY + 200;
        RECT buttonRect = {menuX + 520, buttonY + 140, menuX + 690, buttonY + 200};
        HBRUSH buttonBrush = CreateSolidBrush(inputField == 3 ? RGB(80, 120, 80) : RGB(60, 90, 60));
        FillRect(hdcBuffer, &buttonRect, buttonBrush);
        DeleteObject(buttonBrush);

        HPEN buttonPen = CreatePen(PS_SOLID, 3, RGB(100, 200, 100));
        oldPen = (HPEN)SelectObject(hdcBuffer, buttonPen);
        oldBrush = (HBRUSH)SelectObject(hdcBuffer, GetStockObject(NULL_BRUSH));
        Rectangle(hdcBuffer, buttonRect.left, buttonRect.top, buttonRect.right, buttonRect.bottom);
        SelectObject(hdcBuffer, oldPen);
        SelectObject(hdcBuffer, oldBrush);
        DeleteObject(buttonPen);

        DrawText(menuX + 550, buttonY + 160, L"\u041D\u0410\u0427\u0410\u0422\u042C \u0418\u0413\u0420\u0423", inputField == 3 ? RGB(200, 255, 200) : RGB(150, 200, 150), true);

        vector<wstring> instructions = {
            L"TAB - \u043F\u0435\u0440\u0435\u043A\u043B\u044E\u0447\u0435\u043D\u0438\u0435 \u043C\u0435\u0436\u0434\u0443 \u043F\u043E\u043B\u044F\u043C\u0438",
            L"\u0426\u0418\u0424\u0420\u042B - \u0432\u0432\u043E\u0434 \u0437\u043D\u0430\u0447\u0435\u043D\u0438\u0439",
            L"BACKSPACE - \u0443\u0434\u0430\u043B\u0435\u043D\u0438\u0435 \u0441\u0438\u043C\u0432\u043E\u043B\u0430",
            L"ENTER - \u0432\u044B\u0431\u043E\u0440 \u043F\u043E\u043B\u044F/\u043F\u043E\u0434\u0442\u0432\u0435\u0440\u0436\u0434\u0435\u043D\u0438\u0435",
            L"ESC - \u043E\u0442\u043C\u0435\u043D\u0430",
            L"\u041C\u0430\u043A\u0441. \u0440\u0430\u0437\u043C\u0435\u0440: 250x250"
        };

        int instY = menuY + 280;
        for (size_t i = 0; i < instructions.size(); i++) {
            DrawText(menuX + 50, instY + i * 25, instructions[i], RGB(150, 150, 180), true);
        }
    }

    void DrawMenu() {
        ClearScreen();
        DrawAnimatedBackground();

        if (inSubMenu && subMenuTitle == L"\u0421\u041E\u0417\u0414\u0410\u041D\u0418\u0415 \u041D\u041E\u0412\u041E\u0419 \u0418\u0413\u0420\u042B") {
            DrawInputMenu();
        } else if (inSubMenu) {
            // Другие подменю
        } else {
            DrawAnimatedTitle();

            for (int i = 0; i < menuItems.size(); i++) {
                DrawMainMenuItem(i);
            }

            wstring instruction = L"\u0418\u0441\u043F\u043E\u043B\u044C\u0437\u0443\u0439\u0442\u0435 \u0421\u0422\u0420\u0415\u041B\u041A\u0418 \u0412\u0412\u0415\u0420\u0425/\u0412\u041D\u0418\u0425 \u0434\u043B\u044F \u0432\u044B\u0431\u043E\u0440\u0430 \u0438 ENTER \u0434\u043B\u044F \u043F\u043E\u0434\u0442\u0432\u0435\u0440\u0436\u0434\u0435\u043D\u0438\u044F";
            int instWidth = instruction.length() * 10;
            int instX = (width - instWidth) / 2;
            if (instX < 0) instX = 0;
            DrawText(instX, height - 40, instruction, RGB(150, 150, 200), true);
        }

        BitBlt(hdc, 0, 0, width, height, hdcBuffer, 0, 0, SRCCOPY);
        animationFrame++;
    }

    int HandleMainInput() {
        long long currentTime = GetCurrentTime();

        if (currentTime - lastInputTime < INPUT_COOLDOWN) {
            return -1;
        }

        bool keyPressed = false;
        int result = -1;

        if (GetAsyncKeyState(VK_UP) & 0x8000) {
            selectedItem = (selectedItem - 1 + menuItems.size()) % menuItems.size();
            keyPressed = true;
        }
        else if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
            selectedItem = (selectedItem + 1) % menuItems.size();
            keyPressed = true;
        }
        else if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
            result = selectedItem;
            keyPressed = true;
        }

        if (keyPressed) {
            lastInputTime = currentTime;
        }

        return result;
    }

    bool HandleGameCreationInput() {
        long long currentTime = GetCurrentTime();

        if (currentTime - lastInputTime < INPUT_COOLDOWN) {
            return true;
        }

        bool keyPressed = false;
        bool continueCreating = true;

        if (GetAsyncKeyState(VK_TAB) & 0x8000) {
            inputField = (inputField + 1) % 4;
            keyPressed = true;
        }
        else if (GetAsyncKeyState(VK_UP) & 0x8000) {
            inputField = (inputField - 1 + 4) % 4;
            keyPressed = true;
        }
        else if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
            inputField = (inputField + 1) % 4;
            keyPressed = true;
        }
        else if (inputField < 3) {
            wstring* currentField = nullptr;
            switch (inputField) {
                case 0: currentField = &inputN; break;
                case 1: currentField = &inputM; break;
                case 2: currentField = &inputSeed; break;
            }

            for (wchar_t c = L'0'; c <= L'9'; c++) {
                if (GetAsyncKeyState(c) & 0x8000) {
                    if (currentField->length() < 10) {
                        *currentField += c;
                    }
                    keyPressed = true;
                    break;
                }
            }

            if (!keyPressed && GetAsyncKeyState(VK_BACK) & 0x8000) {
                if (!currentField->empty()) {
                    currentField->pop_back();
                }
                keyPressed = true;
            }
        }

        if (GetAsyncKeyState(VK_RETURN) & 0x8000) {
            if (inputField == 3) {
                continueCreating = false;
            } else {
                inputField = (inputField + 1) % 4;
            }
            keyPressed = true;
        }

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            inputField = -1;
            continueCreating = false;
            keyPressed = true;
        }

        if (keyPressed) {
            lastInputTime = currentTime;
        }

        return continueCreating;
    }

    bool ShowGameCreationMenu(int& n, int& m, long long& SEED) {
        inSubMenu = true;
        subMenuTitle = L"\u0421\u041E\u0417\u0414\u0410\u041D\u0418\u0415 \u041D\u041E\u0412\u041E\u0419 \u0418\u0413\u0420\u042B";
        inputField = 0;
        lastInputTime = GetCurrentTime();

        bool creating = true;
        while (creating) {
            long long currentTime = GetCurrentTime();

            if (currentTime - lastFrameTime >= FRAME_TIME) {
                DrawMenu();
                creating = HandleGameCreationInput();
                lastFrameTime = currentTime;
            }
        }

        inSubMenu = false;

        if (inputField == -1) {
            return false;
        }

        try {
            n = stoi(inputN);
            m = stoi(inputM);
            SEED = stoll(inputSeed);

            // Только минимальные ограничения
            if (n < 5) n = 5;
            if (m < 5) m = 5;
            // Максимальные ограничения убраны
            // if (n > 100) n = 100;
            // if (m > 100) m = 100;

            return true;
        } catch (...) {
            n = 15;
            m = 15;
            SEED = time(NULL);
            return true;
        }
    }

    int RunMenu() {
        lastInputTime = GetCurrentTime();
        lastFrameTime = GetCurrentTime();

        while (true) {
            long long currentTime = GetCurrentTime();

            if (currentTime - lastFrameTime >= FRAME_TIME) {
                DrawMenu();

                int selection = HandleMainInput();
                if (selection != -1) {
                    return selection;
                }

                lastFrameTime = currentTime;
            }
        }
    }
};

void SetConsole()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);

    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
}

int** convertToPointerArray(vector<vector<int>>& vec) {
    int n = vec.size();
    int m = vec[0].size();
    int** arr = new int*[n];
    for (int i = 0; i < n; i++) {
        arr[i] = new int[m];
        for (int j = 0; j < m; j++) {
            arr[i][j] = vec[i][j];
        }
    }
    return arr;
}

void freePointerArray(int** arr, int n) {
    for (int i = 0; i < n; i++) {
        delete[] arr[i];
    }
    delete[] arr;
}

void StartNewGame(OptimizedMenu& menu) {
    int n, m;
    long long SEED;

    if (menu.ShowGameCreationMenu(n, m, SEED)) {
        vector<vector<int>> Labir(n, vector<int>(m));
        srand(static_cast<unsigned int>(SEED));

        int spawnX = 0, spawnY = 0;
        GeneratorMap(Labir, n, m, spawnX, spawnY);

        int** labirPtr = convertToPointerArray(Labir);
        int playerY = spawnY;
        int playerX = spawnX;
        wchar_t c;

        VisualMap(playerX, playerY, n, m, labirPtr, c);
        freePointerArray(labirPtr, n);
    }
}

void ShowSettings(OptimizedMenu& menu) {
    // Заглушка для настроек
}

void ShowRecords(OptimizedMenu& menu) {
    // Заглушка для рекордов
}

void ShowAbout(OptimizedMenu& menu) {
    // Заглушка для информации
}

#endif // VISUALMENU_H_INCLUDED
