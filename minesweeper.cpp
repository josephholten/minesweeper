#include <stdio.h>
#include <vector>
#include <array>
#include <random>
#include <algorithm>
#include <time.h>
#include <fmt/core.h>
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

size_t uncover(minesweeper::Matrix<TileState> &tileStates, minesweeper::Matrix<size_t> &mineCounts, size_t row, size_t col)
{
    size_t uncovered = 1;
    tileStates[row][col] = TileState::Empty;
    if (mineCounts[row][col] > 0)
        return uncovered;

    if (row > 0 && tileStates[row-1][col] != TileState::Empty)
        uncovered += uncover(tileStates, mineCounts, row - 1, col);
    // NE
    if (row > 0 && col + 1 < mineCounts.cols && tileStates[row-1][col+1] != TileState::Empty)
        uncovered += uncover(tileStates, mineCounts, row - 1, col + 1);
    // E
    if (col + 1 < mineCounts.cols && tileStates[row][col+1] != TileState::Empty)
        uncovered += uncover(tileStates, mineCounts, row, col + 1);
    // SE
    if (row + 1 < mineCounts.rows && col + 1 < mineCounts.cols && tileStates[row+1][col+1] != TileState::Empty)
        uncovered += uncover(tileStates, mineCounts, row + 1, col + 1);
    // S
    if (row + 1 < mineCounts.rows && tileStates[row+1][col] != TileState::Empty)
        uncovered += uncover(tileStates, mineCounts, row + 1, col);
    // SW
    if (row + 1 < mineCounts.rows && col > 0 && tileStates[row+1][col-1] != TileState::Empty)
        uncovered += uncover(tileStates, mineCounts, row + 1, col - 1);
    // W
    if (col > 0 && tileStates[row][col-1] != TileState::Empty)
        uncovered += uncover(tileStates, mineCounts, row, col - 1);
    // NW
    if (row > 0 && col > 0 && tileStates[row-1][col-1] != TileState::Empty)
        uncovered += uncover(tileStates, mineCounts, row - 1, col - 1);

    return uncovered;
}

int main(int, char**){
    /*
    const Vector2 center = Vector2Scale(screenSize, 0.5f);
    float textMargin = 5;
    float margin = 50;

    */
    float fontSize = 32.f;
    float fontSpacing = 3.f;

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
    Texture2D numbers[10];
    char path[] = "assets/?.png";
    for (size_t i = 1; i <= 5; i++) {
        path[7] = '0' + i;
        numbers[i] = LoadTexture(path);
    }

    if (!IsTextureReady(flag) || !IsTextureReady(mine)) {
        TraceLog(LOG_ERROR, "textures not ready!");
        CloseWindow();
        return 1;
    }

    SetTargetFPS(60);

    size_t totalMines = 15;
    bool UNHIDE_MINES = false;

    minesweeper::Matrix<uint8_t> mines(boxes.y, boxes.x, false);
    mines_rand_m(mines.data, totalMines);

    minesweeper::Matrix<TileState> tileStates((size_t)boxes.y, (size_t)boxes.x, TileState::Untouched);

    // count mines
    minesweeper::Matrix<size_t> mineCounts((size_t)boxes.y, (size_t)boxes.x, 0);
    for (size_t row = 0; row < mines.rows; row++) {
        for (size_t col = 0; col < mines.cols; col++) {
            // N
            if (row > 0 && mines[row-1][col])
                mineCounts[row][col]++;
            // NE
            if (row > 0 && col + 1 < mines.cols && mines[row-1][col+1])
                mineCounts[row][col]++;
            // E
            if (col + 1 < mines.cols && mines[row][col+1])
                mineCounts[row][col]++;
            // SE
            if (row + 1 < mines.rows && col + 1 < mines.cols && mines[row+1][col+1])
                mineCounts[row][col]++;
            // S
            if (row + 1 < mines.rows && mines[row+1][col])
                mineCounts[row][col]++;
            // SW
            if (row + 1 < mines.rows && col > 0 && mines[row+1][col-1])
                mineCounts[row][col]++;
            // W
            if (col > 0 && mines[row][col-1])
                mineCounts[row][col]++;
            // NW
            if (row > 0 && col > 0 && mines[row-1][col-1])
                mineCounts[row][col]++;
        }
    }

    bool dead = false;
    bool won = false;
    size_t uncovered = 0;

    // close with ESC
    while(!WindowShouldClose()) {
        // handle input
        Vector2 mousePos = GetMousePosition();
        if (!dead && mousePos.x >= boxesOffset.x && mousePos.x <= screenSize.x - boxesOffset.x
            && mousePos.y >= boxesOffset.y && mousePos.y <= screenSize.y - boxesOffset.y)
        {
            size_t mouseBoxX = (mousePos.x - boxesOffset.x) / (boxSize.x + boxMargin.x);
            size_t mouseBoxY = (mousePos.y - boxesOffset.y) / (boxSize.y + boxMargin.y);

            if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                tileStates[mouseBoxY][mouseBoxX] = (tileStates[mouseBoxY][mouseBoxX] != TileState::Flag ? TileState::Flag : TileState::Empty);
            }

            else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (mines[mouseBoxY][mouseBoxX]) {
                    tileStates[mouseBoxY][mouseBoxX] = TileState::Mine;
                    dead = true;
                    TraceLog(LOG_INFO, "you died! %d", dead);
                } else {
                    uncovered += uncover(tileStates, mineCounts, mouseBoxY, mouseBoxX);
                    if (uncovered >= mines.cols*mines.rows-totalMines)
                        won = true;
                }
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
                        if (UNHIDE_MINES && mines[iy][ix])
                            DrawTextureV(mine, {x, y}, backgroundColor);
                        break;

                    case TileState::Empty:
                        DrawRectangleV({x, y}, boxSize, emptyColor);
                        if (mineCounts[iy][ix] > 0) {
                            DrawTextureV(numbers[mineCounts[iy][ix]], {x, y}, WHITE);
                        }
                        break;

                    case TileState::Flag:
                        DrawRectangleV({x, y}, boxSize, boxColor);
                        DrawTextureV(flag, {x, y}, WHITE);
                        break;

                    case TileState::Mine:
                        DrawRectangleV({x, y}, boxSize, emptyColor);
                        DrawTextureV(mine, {x, y}, WHITE);
                        break;

                    default:
                        TraceLog(LOG_FATAL, "unrecognized tile state!");
                        abort();
                        break;
                    }
                }
            }

            if (dead) {
                const char* text = "You died!";
                int fontSize = 60;
                int margin = 5;
                Color color = RED;
                Vector2 pos = {screenSize.x/2, screenSize.y/2};

                int text_width = MeasureText(text, fontSize);
                DrawRectangle(pos.x-text_width/2.-margin, pos.y-fontSize/2.-margin, text_width + 2*margin, fontSize, {0, 0, 0, 128});
                DrawText(text, pos.x-text_width/2., pos.y-fontSize/2., fontSize, color);
            }

            if (won) {
                const char* text = "You won!";
                int fontSize = 60;
                int margin = 5;
                Color color = GREEN;
                Vector2 pos = {screenSize.x/2, screenSize.y/2};

                int text_width = MeasureText(text, fontSize);
                DrawRectangle(pos.x-text_width/2.-margin, pos.y-fontSize/2.-margin, text_width + 2*margin, fontSize, {0, 0, 0, 128});
                DrawText(text, pos.x-text_width/2., pos.y-fontSize/2., fontSize, GREEN);
            }
        } EndDrawing();
    }

    UnloadTexture(flag);
    UnloadTexture(mine);

    CloseWindow();
}
