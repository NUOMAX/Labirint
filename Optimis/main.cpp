#include <iostream>
#include <cstdlib>
#include <conio.h>
#include <windows.h>
#include <vector>
#include <queue>
#include <cmath>
#include <functional>
#include <string>
#include "gEneraTor.h"
#include "VisualMaping.h"

using namespace std;

class ConsoleMenu {
private:
    HWND hConsole;
    HDC hdc;
    int width;
    int height;
    int selectedItem;
    std::vector<std::string> menuItems;
    std::vector<bool> enabledItems;
    std::vector<std::function<void()>> callbacks;

public:
    ConsoleMenu();
    ~ConsoleMenu();

    void initialize();
    void addMenuItem(const std::string& text, bool enabled, std::function<void()> callback);
    void drawMenu();
    void handleInput();
    void run();
};

void SetConsole()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    ios_base::sync_with_stdio(false);
    cout.tie(nullptr);
}

// Функция для преобразования vector<vector<int>> в int**
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

// Функция для освобождения памяти
void freePointerArray(int** arr, int n) {
    for (int i = 0; i < n; i++) {
        delete[] arr[i];
    }
    delete[] arr;
}

int main()
{
    HWND HConsole = GetConsoleWindow();
    ShowWindow(HConsole, SW_MAXIMIZE);
    SetConsole();

    int n, m;
    long long SEED;

    cout << "Введите размер мира N x M: \n";
    cin >> n >> m;
    cout << "Пожалуйста, введите сид: \n";
    cin >> SEED;

    vector<vector<int>> Labir(n, vector<int>(m));

    srand(SEED);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            Labir[i][j] = rand() % 10;
        }
    }

    int spawnX = 0, spawnY = 0;
    GeneratorMap(Labir, n, m, spawnX, spawnY);

    // Преобразуем вектор в int** для VisualMap
    int** labirPtr = convertToPointerArray(Labir);

    int playerY = spawnY;
    int playerX = spawnX;
    wchar_t c;

    VisualMap(playerX, playerY, n, m, labirPtr, c);

    // Освобождаем память
    freePointerArray(labirPtr, n);

    return 0;
}
