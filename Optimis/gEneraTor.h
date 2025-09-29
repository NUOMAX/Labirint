#ifndef GENERATOR_H_INCLUDED
#define GENERATOR_H_INCLUDED

#include <vector>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <functional>
#include <queue>
using namespace std;

inline bool inBounds(int i, int j, int n, int m) {
    return i >= 0 && i < n && j >= 0 && j < m;
}

enum Biome {
    FOREST,
    PINE_FOREST,
    BIRCH_FOREST,
    PLAINS,
    MOUNTAIN,
    RIVER,
    LAKE,
    DESERT,
    SNOW,
    JUNGLE,
    SWAMP,
    BEACH,
    OCEAN,
    CAVE,
    VOLCANO,
    FLOWER_FIELDS,
    MUSHROOM_FOREST
};

struct Cell {
    int value;
    Biome biome;
    int elevation;
    float moisture;
    float temperature;
    bool hasTree;
    bool hasRock;
    bool hasFlowers;
    int resourceType;
};

// Быстрая хэш-функция с защитой от переполнения
inline int fastHash(int x, int y, int seed) {
    // Используем меньшие простые числа для избежания переполнения
    return (x * 374761393 + y * 668265263 + seed * 2246822519) & 0x7FFFFFFF;
}

// Функция интерполяции для плавности
inline float interpolate(float a, float b, float t) {
    return a + (b - a) * (t * t * (3 - 2 * t));
}

// Оптимизированный плавный шум с кэшированием
inline float smoothNoise(float x, float y, int seed) {
    // Для больших координат используем модульную арифметику чтобы избежать больших чисел
    int ix = (int)floor(x);
    int iy = (int)floor(y);
    float fx = x - ix;
    float fy = y - iy;

    // Ограничиваем координаты разумными пределами
    ix = ix % 1000000;
    iy = iy % 1000000;

    float n0 = (fastHash(ix, iy, seed) % 10000) / 10000.0f;
    float n1 = (fastHash(ix + 1, iy, seed) % 10000) / 10000.0f;
    float n2 = (fastHash(ix, iy + 1, seed) % 10000) / 10000.0f;
    float n3 = (fastHash(ix + 1, iy + 1, seed) % 10000) / 10000.0f;

    float i0 = interpolate(n0, n1, fx);
    float i1 = interpolate(n2, n3, fx);
    return interpolate(i0, i1, fy);
}

// Оптимизированная генерация высоты для больших карт
inline float generateElevation(int i, int j, int n, int m, int seed) {
    // Для очень больших карт используем более агрессивное масштабирование
    float scale_factor = min(1000.0f, max(n, m) / 100.0f);

    float continentScale = 0.01f / scale_factor;
    float continent = smoothNoise(i * continentScale, j * continentScale, seed + 100);
    continent = pow(continent, 1.5f);

    // Для больших карт уменьшаем детализацию гор
    float mountainScale = 0.02f / scale_factor;
    float mountains = smoothNoise(i * mountainScale, j * mountainScale, seed + 200);
    mountains = max(0.0f, mountains - 0.4f) * 2.0f;

    // Упрощаем холмы для больших карт
    float hillScale = 0.05f / scale_factor; // Увеличиваем масштаб холмов
    float hills = smoothNoise(i * hillScale, j * hillScale, seed + 300) * 0.2f;

    float height = continent * 0.7f + mountains * 0.2f + hills * 0.1f;

    return height;
}

// Оптимизированная генерация влажности
inline float generateMoisture(int i, int j, int n, int m, int seed) {
    float scale_factor = min(1000.0f, max(n, m) / 100.0f);
    float largeScaleX = 10.0f / scale_factor;
    float largeScaleY = 10.0f / scale_factor;

    float largeScale = smoothNoise(i * largeScaleX, j * largeScaleY, seed + 100);
    float smallScale = smoothNoise(i * 0.1f, j * 0.1f, seed + 200) * 0.3f;

    // Упрощаем эффект океана для больших карт
    float centerX = n / 2.0f;
    float centerY = m / 2.0f;
    float dist = sqrt(pow(i - centerX, 2) + pow(j - centerY, 2));
    float maxDist = sqrt(centerX * centerX + centerY * centerY);
    float oceanEffect = maxDist > 0 ? 1.0f - (dist / maxDist) * 0.3f : 1.0f;

    return (largeScale * 0.7f + smallScale * 0.3f) * oceanEffect;
}

// Оптимизированная генерация температуры
inline float generateTemperature(int i, int j, int n, int m, int seed) {
    if (n <= 0) return 0.5f;

    float baseTemp = 1.0f - abs((float)i / n - 0.5f) * 2.0f;

    float scale_factor = min(1000.0f, max(n, m) / 100.0f);
    float noiseScale = 0.05f / scale_factor;

    float noise = smoothNoise(i * noiseScale, j * noiseScale, seed + 500) * 0.2f;
    float seasonal = sin(i * 0.02f) * 0.1f; // Уменьшаем амплитуду для больших карт

    return baseTemp + noise + seasonal;
}

// Динамическое определение биома
inline Biome determineBiome(float elevation, float moisture, float temperature, int i, int j, int seed) {
    if (elevation < 0.2f) {
        if (temperature < 0.3f) return SNOW;
        if (moisture > 0.7f) return SWAMP;
        if (moisture > 0.4f) return BEACH;
        return OCEAN;
    }
    else if (elevation < 0.4f) {
        if (temperature < 0.2f) return SNOW;
        if (temperature > 0.8f) {
            if (moisture < 0.2f) return DESERT;
            return JUNGLE;
        }
        if (moisture > 0.7f) return SWAMP;
        if (moisture > 0.5f) return FOREST;
        if (moisture > 0.3f) return PLAINS;
        return DESERT;
    }
    else if (elevation < 0.7f) {
        if (temperature < 0.3f) {
            return PINE_FOREST;
        }
        if (moisture > 0.6f) {
            return FOREST;
        }
        if (moisture > 0.3f) return PLAINS;
        return MOUNTAIN;
    }
    else {
        if (temperature < 0.4f) return SNOW;
        return MOUNTAIN;
    }
}

// Упрощенная генерация растительности для больших карт
inline void generateVegetation(vector<vector<int>>& Labir, Biome biome, int i, int j, int seed) {
    // Используем более быстрый шум для больших карт
    float noise = (fastHash(i, j, seed + 1300) % 10000) / 10000.0f;

    switch (biome) {
        case FOREST:
            if (noise < 0.3f) Labir[i][j] = 6;
            break;
        case PINE_FOREST:
            if (noise < 0.25f) Labir[i][j] = 17;
            break;
        case BIRCH_FOREST:
            if (noise < 0.2f) Labir[i][j] = 18;
            break;
        case PLAINS:
            if (noise < 0.1f) Labir[i][j] = 16;
            break;
        case JUNGLE:
            if (noise < 0.4f) Labir[i][j] = 20;
            break;
        case DESERT:
            if (noise < 0.05f) Labir[i][j] = 8;
            break;
        default:
            break;
    }
}

// Оптимизированная основная функция генерации
void GeneratorMap(vector<vector<int>>& Labir, int n, int m, int& spawnX, int& spawnY) {
    int seed = rand();

    // Предварительная проверка на слишком большие размеры
    if (n > 1000 || m > 1000) {
        // Для очень больших карт используем упрощенную генерацию
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                float elevation = generateElevation(i, j, n, m, seed);

                if (elevation < 0.3f) {
                    Labir[i][j] = 4; // Вода
                } else if (elevation > 0.7f) {
                    Labir[i][j] = 7; // Горы
                } else {
                    Labir[i][j] = 0; // Земля

                    // Упрощенная растительность
                    float moisture = generateMoisture(i, j, n, m, seed);
                    float temperature = generateTemperature(i, j, n, m, seed);
                    Biome biome = determineBiome(elevation, moisture, temperature, i, j, seed);
                    generateVegetation(Labir, biome, i, j, seed);
                }
            }
        }
    } else {
        // Стандартная генерация для карт до 1000x1000
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                float elevation = generateElevation(i, j, n, m, seed);
                float moisture = generateMoisture(i, j, n, m, seed);
                float temperature = generateTemperature(i, j, n, m, seed);

                Biome biome = determineBiome(elevation, moisture, temperature, i, j, seed);

                if (elevation < 0.2f) {
                    Labir[i][j] = 4;
                } else if (elevation > 0.8f) {
                    Labir[i][j] = 7;
                } else {
                    Labir[i][j] = 0;
                }
            }
        }

        // Растительность только для земли
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                if (Labir[i][j] == 0) {
                    float elevation = generateElevation(i, j, n, m, seed);
                    float moisture = generateMoisture(i, j, n, m, seed);
                    float temperature = generateTemperature(i, j, n, m, seed);
                    Biome biome = determineBiome(elevation, moisture, temperature, i, j, seed);
                    generateVegetation(Labir, biome, i, j, seed);
                }
            }
        }
    }

    // Оптимизированный поиск спавна
    vector<pair<int, int>> goodSpawns;

    // Используем разреженный поиск для больших карт
    int step = max(1, min(n, m) / 50);
    for (int i = n/4; i < 3*n/4; i += step) {
        for (int j = m/4; j < 3*m/4; j += step) {
            if (Labir[i][j] == 0) {
                goodSpawns.emplace_back(i, j);
            }
        }
    }

    // Если не нашли хороших позиций, ищем любую землю
    if (goodSpawns.empty()) {
        for (int i = 0; i < n && goodSpawns.size() < 10; i += max(1, n/10)) {
            for (int j = 0; j < m && goodSpawns.size() < 10; j += max(1, m/10)) {
                if (Labir[i][j] == 0) {
                    goodSpawns.emplace_back(i, j);
                }
            }
        }
    }

    // Выбираем точку спавна
    if (!goodSpawns.empty()) {
        auto [x, y] = goodSpawns[goodSpawns.size() / 2];
        spawnX = x;
        spawnY = y;
    } else {
        // Фолбэк: центр карты
        spawnX = n/2;
        spawnY = m/2;
        // Гарантируем, что в центре есть земля
        if (spawnX < n && spawnY < m) {
            Labir[spawnX][spawnY] = 0;
        }
    }

    if (spawnX < n && spawnY < m) {
        Labir[spawnX][spawnY] = 2;
    }

    // Оптимизированный поиск выхода
    int exitX = -1, exitY = -1;
    float maxDist = -1;

    // Используем разреженный поиск для больших карт
    step = max(1, min(n, m) / 30);
    for (int i = 0; i < n; i += step) {
        for (int j = 0; j < m; j += step) {
            if (Labir[i][j] == 0 && (i != spawnX || j != spawnY)) {
                float dist = sqrt(pow(i - spawnX, 2) + pow(j - spawnY, 2));
                if (dist > maxDist) {
                    maxDist = dist;
                    exitX = i;
                    exitY = j;
                }
            }
        }
    }

    // Если не нашли разреженно, ищем подробно но в ограниченной области
    if (exitX == -1) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                if (Labir[i][j] == 0 && (i != spawnX || j != spawnY)) {
                    float dist = sqrt(pow(i - spawnX, 2) + pow(j - spawnY, 2));
                    if (dist > maxDist) {
                        maxDist = dist;
                        exitX = i;
                        exitY = j;
                    }
                }
            }
        }
    }

    // Финальный фолбэк
    if (exitX == -1) {
        exitX = (spawnX == 0 && n > 1) ? n-1 : 0;
        exitY = (spawnY == 0 && m > 1) ? m-1 : 0;
    }

    if (exitX < n && exitY < m) {
        Labir[exitX][exitY] = 3;
    }
}

#endif //GENERATOR_H_INCLUDED
