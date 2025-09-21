#ifndef GENERATOR_H_INCLUDED
#define GENERATOR_H_INCLUDED

inline bool inBounds(int i, int j, int n, int m)
{
    return i >= 0 && i < n && j >= 0 && j < m;
}

void GeneratorMap(int** Labir, int n, int m, int X, int Y)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            int left = (j > 0) ? Labir[i][j-1] : 0;
            if ((left + Labir[i][j]) >= 7)
            {
                Labir[i][j] = 0;
            }
            else
            {
                Labir[i][j] = 1;
            }
        }
    }
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            if (inBounds(i, j, n, m) && inBounds(i+2, j, n, m) && inBounds(i, j+2, n, m) && inBounds(i+1, j, n, m) && inBounds(i+1, j+1, n, m))
                {
                int sum = Labir[i][j] + Labir[i][j+1] + Labir[i][j+2] + Labir[i+1][j] + Labir[i+1][j+1] + Labir[i+2][j];
                if (sum >= 4)
                {
                    Labir[i][j] = 0;
                }
                else
                {
                    Labir[i][j] = 1;
                }
            }
        }
    }
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
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
                }
                else
                {
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
            if (inBounds(i, j, n, m))
                Labir[i][j] = 0;
        }
    }
    bool found = false;
    srand(time(NULL));
    while (!found)
    {
        int x = rand() % n, y = rand() % m;
        int sum = 0;
        for (int i = y-4; i <= y+4; i++)
        {
            for (int j = x-4; j <= x+4; j++)
            {
                if (inBounds(i, j, n, m)) sum += Labir[i][j];
            }
        }
        if (sum == 0)
        {
            X = x;
            Y = y;
            Labir[y][x] = 2;
            found = true;
        }
    }
}

#endif // GENERATOR_H_INCLUDED
