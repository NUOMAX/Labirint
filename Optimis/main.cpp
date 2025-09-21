#include <iostream>
#include <cstdlib>
#include <conio.h>
#include <windows.h>
#include "gEneraTor.h"
#include "VisualMaping.h"

void SetConsole()
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    std::ios_base::sync_with_stdio(false);
    std::cout.tie(nullptr);
}

int main()
{
    SetConsole();
    int n, m;
    long long SEED;
    std::cout <<"Введите размер мира N x M: \n";
    std::cin >> n >> m;
    std::cout << "Пожалуйста, введите сид: \n";
    std::cin >> SEED;
    int** Labir = new int*[n];
    for (int i = 0; i < n; ++i)
    {
        Labir[i] = new int[m];
    }
    srand(SEED);
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            Labir[i][j] = rand() % 10;
        }
    }
    int X = 0, Y = 0;
    GeneratorMap(Labir, n, m, X, Y);
    int playerY = n - 3, playerX = 2;
    wchar_t c;
    VisualMap(playerX, playerY, n, m, Labir, c);

    for (int i = 0; i < n; ++i) {
        delete[] Labir[i];
    }

    return 0;
}
