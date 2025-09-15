#include <iostream>
#include <cstdlib>
#include <windows.h>
#include <conio.h>

void SetConsole() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}

inline bool inBounds(int i, int j, int n, int m)
{
    return i >= 0 && i < n && j >= 0 && j < m;
}

int main()
{
    std::ios_base::sync_with_stdio(false);
    std::cout.tie(nullptr);
    SetConsole();

    int n, m;
    long long SEED;
    std::cout <<"Введите размер мира N x M:::::::::::jvgwjegvegvyewvweejk \n";
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
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            int left = (j > 0) ? Labir[i][j-1] : 0;
            if ((left + Labir[i][j]) >= 7)
            {
                Labir[i][j] = 0;
            } else {
                Labir[i][j] = 1;
            }
        }
    }
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (inBounds(i, j, n, m) && inBounds(i+2, j, n, m) && inBounds(i, j+2, n, m) && inBounds(i+1, j, n, m) && inBounds(i+1, j+1, n, m))
            {
                int sum = Labir[i][j] + Labir[i][j+1] + Labir[i][j+2] + Labir[i+1][j] + Labir[i+1][j+1] + Labir[i+2][j];
                if (sum >= 4)
                {
                    Labir[i][j] = 0;
                } else {
                    Labir[i][j] = 1;
                }
            }
        }
    }
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (inBounds(i-1, j-1, n, m) && inBounds(i-1, j+1, n, m) && inBounds(i+1, j-1, n, m) && inBounds(i+1, j, n, m))
                {
                int count = Labir[i-1][j] + Labir[i-1][j-1] + Labir[i-1][j+1] + Labir[i][j-1] + Labir[i][j+1] + Labir[i+1][j] + Labir[i+1][j-1] + Labir[i][j];
                if (count <= 1)
                {
                    Labir[i][j] = 0;
                }
                if (Labir[i][j+1] && Labir[i-1][j] == 1)
                {
                    Labir[i][j+1] = 0;
                } else {
                    if (Labir[i][j] && Labir[i+1][j+1] == 1)
                    {
                        Labir[i][j] = 0;
                    }
                }

                if (Labir[i][j] == 0)
                    {
                    int neighbors = (inBounds(i-1, j, n, m) ? Labir[i-1][j] : 0) + (inBounds(i, j+1, n, m) ? Labir[i][j+1] : 0) + (inBounds(i, j-1, n, m) ? Labir[i][j-1] : 0) + (inBounds(i+1, j, n, m) ? Labir[i+1][j] : 0);
                    if (neighbors == 4)
                    {
                        Labir[i][j] = 1;
                    }

                    neighbors += (inBounds(i+1, j-1, n, m) ? Labir[i+1][j-1] : 0);
                    if (neighbors == 5)
                    {
                        Labir[i][j] = 1;
                    }
                }

                if (Labir[i][j] == 1 && inBounds(i, j+1, n, m) && Labir[i][j+1] == 0 && inBounds(i+1, j, n, m) && Labir[i+1][j] == 0 && inBounds(i+1, j+1, n, m) && Labir[i+1][j+1] == 1)
                {
                    Labir[i+1][j+1] = 0;
                }

                if (Labir[i][j] == 0 && inBounds(i, j+1, n, m) && Labir[i][j+1] == 1 && inBounds(i+1, j, n, m) && Labir[i+1][j] == 1 && inBounds(i+1, j+1, n, m) && Labir[i+1][j+1] == 0)
                {
                    Labir[i][j+1] = 0;
                }
            }
        }
    }
    for (int i = 0; i < 5; i++)
    {
        for (int j = m-5; j < m; j++)
        {
            if (inBounds(i, j, n, m)) Labir[i][j] = 0;
        }
    }
    int X = 0, Y = 0;
    bool found = false;
    srand(time(NULL));
    while (!found)
    {
        int x = rand() % n, y = rand() % m;
        int sum = 0;
        for (int i = y-2; i <= y+2; i++)
        {
            for (int j = x-2; j <= x+2; j++)
            {
                if (inBounds(i, j, n, m)) sum += Labir[i][j];
            }
        }
        if (sum == 0)
        {
            X = x;
            Y = y;
            found = true;
        }
    }
    Labir[Y][X] = 2;
    int playerY = n - 3, playerX = 2;
    char c;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    while (true)
    {
        COORD coord = {0, 0};
        SetConsoleCursorPosition(hConsole, coord);
        for (int i = playerY - 8; i <= playerY + 8; i++)
        {
            for (int j = playerX - 6; j <= playerX + 6; j++)
            {
                if (!inBounds(i, j, n, m))
                {
                    std::cout << "████";
                    continue;
                }
                if (i == playerY && j == playerX)
                {
                    std::cout << "TTTT";
                } else if (Labir[i][j] == 0)
                {
                    std::cout << "░░░░";
                } else if (Labir[i][j] == 2)
                {
                    std::cout << "++++";
                } else {
                    if (inBounds(i, j+1, n, m) && Labir[i][j+1] == 0)
                    {
                        std::cout << "███▓";
                    } else if (inBounds(i, j-1, n, m) && Labir[i][j-1] == 0)
                    {
                        std::cout << "░███";
                    } else if ((inBounds(i+1, j, n, m) && Labir[i+1][j] == 0) || (inBounds(i-1, j, n, m) && Labir[i-1][j] == 0) || (inBounds(i, j-1, n, m) && Labir[i][j-1] == 0) || (inBounds(i, j+1, n, m) && Labir[i][j+1] == 0))
                    {
                        std::cout << "▓▓▓▓";
                    } else {
                        std::cout << "████";
                    }
                }
            }
            std::cout << "\n";
        }
        std::cout << "\n Y=" << playerY << " X=" << playerX<<"  ";
        if(Labir[playerY][playerX] == 2)
        {
            std::cout<<"\n\n\n Cпасибо за игру! :) \n\n\n";
            break;
        }
        std::cout.flush();
        c = _getch();
        if (c == ' ') break;

        int newY = playerY, newX = playerX;
        switch (c) {
            case 'w': newY--; break;
            case 'W': newY--; break;
            case 'ц': newY--; break;
            case 'Ц': newY--; break;
            case 's': newY++; break;
            case 'S': newY++; break;
            case 'ы': newY++; break;
            case 'Ы': newY++; break;
            case 'a': newX--; break;
            case 'A': newX--; break;
            case 'ф': newX--; break;
            case 'Ф': newX--; break;
            case 'd': newX++; break;
            case 'D': newX++; break;
            case 'в': newX++; break;
            case 'В': newX++; break;
        }

        if (inBounds(newY, newX, n, m) && Labir[newY][newX] != 1) {
            playerY = newY;
            playerX = newX;
        }
    }
    for (int i = 0; i < n; ++i) {
        delete[] Labir[i];
    }
    delete[] Labir;

    return 0;
}
