#include <stdio.h>
#include <time.h>
#include "raylib.h"
#include "raymath.h"

#include "util.h"

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

int main(int, char**){
    /*
    const Vector2 center = Vector2Scale(screenSize, 0.5f);
    float textMargin = 5;
    float margin = 50;

    */
    float fontSize = 20.f;
    float fontSpacing = 3.f;

    Color backgroundColor = BLACK;
    Color foregroundColor = WHITE;

    Vector2 boxSize = {32, 32};
    Vector2 boxMargin = {4, 4};
    Color boxColor = foregroundColor;

    float borderThickness = 8;
    Color borderColor = foregroundColor;

    Vector2 boxes = {20, 15};

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
    Texture2D bomb = LoadTexture("assets/bomb.png");

    if (!IsTextureReady(flag) || !IsTextureReady(bomb)) {
        TraceLog(LOG_ERROR, "textures not ready!");
        CloseWindow();
        return 1;
    }

    SetRandomSeed((unsigned int)time(NULL));

    SetTargetFPS(60);

    // close with ESC
    while(!WindowShouldClose()) {
        BeginDrawing(); {
            ClearBackground(backgroundColor);
            // draw border
            Rectangle borderRec = {0, 0, screenSize.x, screenSize.y};
            DrawRectangleLinesEx(borderRec, borderThickness, borderColor);

            Vector2 boxesOffset = {
                borderThickness + boxMargin.x,
                borderThickness + boxMargin.y
            };

            // draw boxes
            for (float x = boxesOffset.x; x + boxesOffset.x + boxSize.x <= screenSize.x; x += boxSize.x + boxMargin.x) {
                for (float y = boxesOffset.y; y + boxesOffset.y + boxSize.y <= screenSize.y; y += boxSize.y + boxMargin.y) {
                    DrawRectangleV({x, y}, boxSize, boxColor);
                    DrawTextureV(flag, {x, y}, WHITE);
                }
            }

        } EndDrawing();
    }

    UnloadTexture(flag);
    UnloadTexture(bomb);

    CloseWindow();
}
