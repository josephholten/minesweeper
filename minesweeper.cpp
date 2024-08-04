#include <stdio.h>
#include <vector>
#include <random>
#include <algorithm>
#include <time.h>
#include "raylib.h"
#include "raymath.h"

#include "util.h"

namespace minesweeper {
template<typename T>
struct Matrix {
    size_t rows;
    size_t cols;
    std::vector<T> data;

    Matrix(size_t _rows, size_t _cols, T init) : rows{_rows}, cols{_cols}, data(rows*cols, init) {};

    T* operator[](size_t i) {
        return data.data() + i * cols;
    }
};
}

void DrawNet(Vector2 screenSize, Vector2 margin, float thick, Color color) {
    Vector2 nw {margin.x, margin.y};
    Vector2 ne {screenSize.x-margin.x, margin.y};
    Vector2 sw {margin.x, screenSize.y-margin.y};
    Vector2 se {screenSize.x-margin.x, screenSize.y-margin.x};

    float dashLength = 3*thick;

    Vector2 horizontalLineOffset = {0, thick/2};
    Vector2 verticalLineOffset = {thick/2, 0};
    // top
    DrawLineEx(Vector2Add(nw, horizontalLineOffset), Vector2Add(ne, horizontalLineOffset), thick, color);
    // right
    DrawLineExDashed(Vector2Subtract(ne, verticalLineOffset), Vector2Subtract(se, verticalLineOffset), thick, color, dashLength);
    // bottom
    DrawLineEx(Vector2Subtract(se, horizontalLineOffset), Vector2Subtract(sw, horizontalLineOffset), thick, color);
    // left
    DrawLineExDashed(Vector2Add(sw, verticalLineOffset), Vector2Add(nw, verticalLineOffset), thick, color, dashLength);
}

void mines_rand_p(std::vector<uint8_t>& mines, double p) {
    std::default_random_engine random_engine(std::random_device{}());
    std::bernoulli_distribution bernoulli(p);
    for (size_t i = 0; i < mines.size(); i++)
        mines[i] = bernoulli(random_engine);
}

void mines_rand_m(std::vector<uint8_t>& mines, size_t m) {
    std::vector<size_t> idxs(mines.size(), 0);
    for (size_t i = 0; i < mines.size(); i++)
        idxs[i] = i;
    std::shuffle(
        idxs.begin(),
        idxs.end(),
        std::random_device()
    );
    for (size_t i = 0; i < m; i++)
        mines[idxs[i]] = 1;
}

enum class TileState {
    Untouched,
    Empty,
    Mine,
    Flag,
};

int main(int, char**){
    /*
    const Vector2 center = Vector2Scale(screenSize, 0.5f);
    float textMargin = 5;
    float margin = 50;

    float fontSize = 20.f;
    float fontSpacing = 3.f;
    */

    Color backgroundColor = BLACK;
    Color foregroundColor = WHITE;

    Vector2 boxSize = {32, 32};
    Vector2 boxMargin = {4, 4};
    Color boxColor = foregroundColor;
    Color emptyColor = GRAY;

    float borderThickness = 8;
    Color borderColor = foregroundColor;

    Vector2 boxes = {20, 15};

    Vector2 boxesOffset = {
        borderThickness + boxMargin.x,
        borderThickness + boxMargin.y
    };

    Vector2 screenSize = {
        2*borderThickness + boxMargin.x + boxes.x * (boxSize.x+boxMargin.x),
        2*borderThickness + boxMargin.y + boxes.y * (boxSize.y+boxMargin.y)
    };

    SetTraceLogCallback(CustomLog);
    SetTraceLogLevel(LOG_DEBUG);
    InitWindow(screenSize.x, screenSize.y, "minesweeper");

    Font font = GetFontDefault();
    if (!IsFontReady(font)) {
        TraceLog(LOG_ERROR, "font is not ready!");
        CloseWindow();
        return 1;
    }

    Texture2D flag = LoadTexture("assets/flag.png");
    Texture2D mine = LoadTexture("assets/mine.png");

    if (!IsTextureReady(flag) || !IsTextureReady(mine)) {
        TraceLog(LOG_ERROR, "textures not ready!");
        CloseWindow();
        return 1;
    }

    SetTargetFPS(60);

    size_t totalMines = 15;

    minesweeper::Matrix<uint8_t> mines(boxes.y, boxes.x, false);
    mines_rand_m(mines.data, totalMines);

    minesweeper::Matrix<TileState> tileStates((size_t)boxes.y, (size_t)boxes.x, TileState::Untouched);
    minesweeper::Matrix<size_t> mineCounts((size_t)boxes.y, (size_t)boxes.x, 0);

    // close with ESC
    while(!WindowShouldClose()) {
        // handle input
        Vector2 mousePos = GetMousePosition();
        if (mousePos.x >= boxesOffset.x && mousePos.x <= screenSize.x - boxesOffset.x
            && mousePos.y >= boxesOffset.y && mousePos.y <= screenSize.y - boxesOffset.y)
        {
            size_t mouseBoxX = (mousePos.x - boxesOffset.x) / (boxSize.x + boxMargin.x);
            size_t mouseBoxY = (mousePos.y - boxesOffset.y) / (boxSize.y + boxMargin.y);

            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                tileStates[mouseBoxY][mouseBoxX] = TileState::Flag;
            }

            else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                tileStates[mouseBoxY][mouseBoxX] = (mines[mouseBoxY][mouseBoxX] ? TileState::Mine : TileState::Empty);
            }
        }

        BeginDrawing(); {
            ClearBackground(backgroundColor);
            // draw border
            Rectangle borderRec = {0, 0, screenSize.x, screenSize.y};
            DrawRectangleLinesEx(borderRec, borderThickness, borderColor);

            for (size_t iy = 0; iy < boxes.y; iy++) {
                for (size_t ix = 0; ix < boxes.x; ix++) {
                    float x = boxesOffset.x + ix*(boxSize.x + boxMargin.x);
                    float y = boxesOffset.y + iy*(boxSize.y + boxMargin.y);

                    switch (tileStates[iy][ix]) {
                    case TileState::Untouched:
                        DrawRectangleV({x, y}, boxSize, boxColor);
                        if (mines[iy][ix])
                            DrawTextureV(mine, {x, y}, BLACK);
                        break;

                    case TileState::Empty:
                        DrawRectangleV({x, y}, boxSize, emptyColor);
                        break;

                    case TileState::Flag:
                        DrawRectangleV({x, y}, boxSize, boxColor);
                        DrawTextureV(flag, {x, y}, WHITE);
                        break;

                    case TileState::Mine:
                        DrawRectangleV({x, y}, boxSize, boxColor);
                        DrawTextureV(mine, {x, y}, WHITE);
                        break;

                    default:
                        TraceLog(LOG_FATAL, "unrecognized tile state!");
                        abort();
                        break;
                    }
                }
            }

        } EndDrawing();
    }

    UnloadTexture(flag);
    UnloadTexture(mine);

    CloseWindow();
}
