#ifndef GENERATOR_H_INCLUDED
#define GENERATOR_H_INCLUDED
using namespace std;

inline bool inBounds(int i, int j, int n, int m) {
    return i >= 0 && i < n && j >= 0 && j < m;
}

enum Biome {
    FOREST,
    CAVE,
    PLAINS,
    MOUNTAIN,
    WATER,
    DESERT
};

struct Cell {
    int value;
    Biome biome;
};

void floodFill(vector<vector<Cell>>& grid, int x, int y, Biome biome, int n, int m) {
    if (!inBounds(x, y, n, m) || grid[x][y].biome != FOREST) return;

    queue<pair<int, int>> q;
    q.push(make_pair(x, y));
    grid[x][y].biome = biome;

    int dx[] = {-1, 1, 0, 0};
    int dy[] = {0, 0, -1, 1};

    while (!q.empty()) {
        auto current = q.front();
        q.pop();
        int cx = current.first;
        int cy = current.second;

        for (int i = 0; i < 4; i++) {
            int nx = cx + dx[i];
            int ny = cy + dy[i];

            if (inBounds(nx, ny, n, m) && grid[nx][ny].biome == FOREST) {
                if (rand() % 100 < 65) {
                    grid[nx][ny].biome = biome;
                    q.push(make_pair(nx, ny));
                }
            }
        }
    }
}

void generateStructures(vector<vector<Cell>>& grid, int n, int m) {
    // Generate rivers
    for (int i = 0; i < 2; i++) {
        int startX = 5 + rand() % (n - 10);
        int currentX = startX;
        int currentY = 0;

        while (currentY < m) {
            if (inBounds(currentX, currentY, n, m)) {
                grid[currentX][currentY].value = 0;
                grid[currentX][currentY].biome = WATER;

                // Make river wider
                if (inBounds(currentX-1, currentY, n, m)) {
                    grid[currentX-1][currentY].value = 0;
                    grid[currentX-1][currentY].biome = WATER;
                }
                if (inBounds(currentX+1, currentY, n, m)) {
                    grid[currentX+1][currentY].value = 0;
                    grid[currentX+1][currentY].biome = WATER;
                }
            }

            int moveX = (rand() % 3) - 1;
            currentX = max(2, min(n-3, currentX + moveX));
            currentY++;
        }
    }

    // Generate mountains
    for (int i = 0; i < 4; i++) {
        int centerX = 5 + rand() % (n - 10);
        int centerY = 5 + rand() % (m - 10);
        int radius = 3 + rand() % 4;

        for (int dx = -radius; dx <= radius; dx++) {
            for (int dy = -radius; dy <= radius; dy++) {
                if (dx*dx + dy*dy <= radius*radius) {
                    int x = centerX + dx;
                    int y = centerY + dy;
                    if (inBounds(x, y, n, m)) {
                        if (rand() % 100 < 70) {
                            grid[x][y].value = 1;
                            grid[x][y].biome = MOUNTAIN;
                        }
                    }
                }
            }
        }
    }

    // Generate caves
    for (int i = 0; i < 3; i++) {
        int caveX = 5 + rand() % (n - 10);
        int caveY = 5 + rand() % (m - 10);
        int caveSize = 3 + rand() % 4;

        for (int dx = -caveSize; dx <= caveSize; dx++) {
            for (int dy = -caveSize; dy <= caveSize; dy++) {
                if (abs(dx) + abs(dy) <= caveSize) {
                    int x = caveX + dx;
                    int y = caveY + dy;
                    if (inBounds(x, y, n, m)) {
                        grid[x][y].value = 0;
                        grid[x][y].biome = CAVE;
                    }
                }
            }
        }
    }
}

bool isValidSpawnLocation(vector<vector<int>>& Labir, int x, int y, int n, int m) {
    if (!inBounds(x, y, n, m)) return false;

    // Проверяем, что сам тайл проходимый
    int tile = Labir[x][y];
    if (tile != 0 && tile != 5 && tile != 8) return false;

    // Проверяем область 3x3 вокруг
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            if (inBounds(x+dx, y+dy, n, m)) {
                int adjacentTile = Labir[x+dx][y+dy];
                // Если рядом есть непроходимый тайл - не подходит
                if (adjacentTile == 1 || adjacentTile == 4 ||
                    adjacentTile == 6 || adjacentTile == 7 || adjacentTile == 9) {
                    return false;
                }
            }
        }
    }

    return true;
}

void GeneratorMap(vector<vector<int>>& Labir, int n, int m, int& spawnX, int& spawnY) {
    // Создаем временную сетку с расширенной информацией
    vector<vector<Cell>> grid(n, vector<Cell>(m));

    // Инициализация шумом
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            grid[i][j].value = (rand() % 100 < 40) ? 1 : 0;
            grid[i][j].biome = FOREST;
        }
    }

    // Клеточный автомат для сглаживания
    for (int iter = 0; iter < 4; iter++) {
        vector<vector<Cell>> temp = grid;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                int wallCount = 0;
                int neighborCount = 0;

                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        if (dx == 0 && dy == 0) continue;
                        if (inBounds(i+dx, j+dy, n, m)) {
                            wallCount += temp[i+dx][j+dy].value;
                            neighborCount++;
                        }
                    }
                }

                if (neighborCount > 0) {
                    if (wallCount > neighborCount * 0.6) {
                        grid[i][j].value = 1;
                    } else if (wallCount < neighborCount * 0.4) {
                        grid[i][j].value = 0;
                    }
                }
            }
        }
    }

    // Генерация биомов
    vector<Biome> biomes = {PLAINS, CAVE, MOUNTAIN, DESERT};
    for (const auto& biome : biomes) {
        int startX = 8 + rand() % (n - 16);
        int startY = 8 + rand() % (m - 16);
        floodFill(grid, startX, startY, biome, n, m);
    }

    // Добавление структур
    generateStructures(grid, n, m);

    // Конвертация обратно в int
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            // Присваиваем значения на основе биома
            switch (grid[i][j].biome) {
                case WATER:
                    Labir[i][j] = 4; // Вода
                    break;
                case CAVE:
                    Labir[i][j] = (grid[i][j].value == 0) ? 5 : 6; // Пещера
                    break;
                case MOUNTAIN:
                    Labir[i][j] = (grid[i][j].value == 1) ? 7 : 0; // Горы
                    break;
                case DESERT:
                    Labir[i][j] = (grid[i][j].value == 0) ? 8 : 9; // Пустыня
                    break;
                default:
                    Labir[i][j] = grid[i][j].value; // Обычная земля
            }
        }
    }

    // Поиск точки спавна с улучшенной проверкой
    vector<pair<int, int>> potentialSpawns;
    for (int i = 8; i < n-8; i++) {
        for (int j = 8; j < m-8; j++) {
            if (isValidSpawnLocation(Labir, i, j, n, m)) {
                int openSpace = 0;
                for (int dx = -2; dx <= 2; dx++) {
                    for (int dy = -2; dy <= 2; dy++) {
                        if (inBounds(i+dx, j+dy, n, m) &&
                           (Labir[i+dx][j+dy] == 0 || Labir[i+dx][j+dy] == 5 || Labir[i+dx][j+dy] == 8)) {
                            openSpace++;
                        }
                    }
                }
                if (openSpace >= 8) {
                    potentialSpawns.push_back(make_pair(i, j));
                }
            }
        }
    }

    if (!potentialSpawns.empty()) {
        auto spawn = potentialSpawns[rand() % potentialSpawns.size()];
        spawnX = spawn.first;
        spawnY = spawn.second;
        Labir[spawnX][spawnY] = 2; // Точка спавна

        // Дополнительная проверка: убедимся, что вокруг спавна нет стен
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (inBounds(spawnX+dx, spawnY+dy, n, m)) {
                    int tile = Labir[spawnX+dx][spawnY+dy];
                    if (tile == 1 || tile == 4 || tile == 6 || tile == 7 || tile == 9) {
                        // Заменяем непроходимые тайлы вокруг спавна на землю
                        Labir[spawnX+dx][spawnY+dy] = 0;
                    }
                }
            }
        }
    } else {
        // Аварийный поиск: если не нашли подходящего места, ищем любую землю
        for (int i = n/2-5; i <= n/2+5; i++) {
            for (int j = m/2-5; j <= m/2+5; j++) {
                if (inBounds(i, j, n, m) &&
                   (Labir[i][j] == 0 || Labir[i][j] == 5 || Labir[i][j] == 8)) {
                    spawnX = i;
                    spawnY = j;
                    Labir[i][j] = 2;

                    // Очищаем область вокруг
                    for (int dx = -2; dx <= 2; dx++) {
                        for (int dy = -2; dy <= 2; dy++) {
                            if (inBounds(i+dx, j+dy, n, m) &&
                               (Labir[i+dx][j+dy] == 1 || Labir[i+dx][j+dy] == 4 ||
                                Labir[i+dx][j+dy] == 6 || Labir[i+dx][j+dy] == 7 ||
                                Labir[i+dx][j+dy] == 9)) {
                                Labir[i+dx][j+dy] = 0;
                            }
                        }
                    }
                    // Выходим из функции после аварийного поиска
                    return;
                }
            }
        }

        // Последний резерв: создаем безопасную зону в центре
        spawnX = n/2;
        spawnY = m/2;

        // Создаем очищенную область 5x5
        for (int dx = -2; dx <= 2; dx++) {
            for (int dy = -2; dy <= 2; dy++) {
                if (inBounds(spawnX+dx, spawnY+dy, n, m)) {
                    Labir[spawnX+dx][spawnY+dy] = 0;
                }
            }
        }
        Labir[spawnX][spawnY] = 2;
    }

    // Генерация точки выхода
    int exitX, exitY;
    int attempts = 0;
    do {
        exitX = 10 + rand() % (n-20);
        exitY = 10 + rand() % (m-20);
        attempts++;
    } while ((abs(exitX - spawnX) + abs(exitY - spawnY) < min(n, m)/3) &&
             attempts < 100 && Labir[exitX][exitY] != 0);

    if (attempts < 100 && inBounds(exitX, exitY, n, m) && Labir[exitX][exitY] == 0) {
        Labir[exitX][exitY] = 3; // Точка выхода
    } else {
        // Резервное расположение
        Labir[n-3][m-3] = 3;
    }

    // Гарантируем, что спавн и выход соединены
    int curX = spawnX, curY = spawnY;
    int targetX = exitX, targetY = exitY;

    while (abs(curX - targetX) > 1 || abs(curY - targetY) > 1) {
        if (curX < targetX) curX++;
        else if (curX > targetX) curX--;
        if (curY < targetY) curY++;
        else if (curY > targetY) curY--;

        if (inBounds(curX, curY, n, m) && Labir[curX][curY] != 0 && Labir[curX][curY] != 2 && Labir[curX][curY] != 3) {
            Labir[curX][curY] = 0;
        }
    }
}

#endif // GENERATOR_H_INCLUDED
