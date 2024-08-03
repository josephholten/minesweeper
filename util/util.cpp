#include "raylib.h"
#include "raymath.h"
#include <time.h>
#include <stdio.h>

int GetRandomSign() {
    return GetRandomValue(0, 1) ? 1 : -1;
}

// Custom logging function
void CustomLog(int logLevel, const char *text, va_list args) {
    char timeStr[64] = { 0 };
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);
    printf("[%s ", timeStr);

    switch (logLevel)
    {
        case LOG_INFO: printf("INFO] "); break;
        case LOG_ERROR: printf("ERROR] "); break;
        case LOG_WARNING: printf("WARN]  "); break;
        case LOG_DEBUG: printf("DEBUG] "); break;
        default: break;
    }

    vprintf(text, args);
    printf("\n");
}

void DrawLineExDashed(Vector2 startPos, Vector2 endPos, float thick, Color color, float dashLength) {
    float totalLength = Vector2Length(Vector2Subtract(endPos, startPos));

    // we want the space to be about half of the dash, but we adjust so dashes and spaces fit exactly into total length
    float approxSpaceLength = dashLength/2;

    // get max number of dashes that fit into totalLength, not accounting for last space
    int numDashes = (totalLength + approxSpaceLength) / (dashLength + approxSpaceLength);

    // now to determine the exact space length
    // know: totalLength + spaceLength = numDashes * (dashLength + spaceLength)
    // implies: totalLength - numDashes*dashLength = (numDashes-1)*spaceLength
    // i.e.
    float spaceLength = (totalLength - numDashes*dashLength) / (numDashes-1);

    Vector2 direction = Vector2Normalize(Vector2Subtract(endPos, startPos));
    Vector2 pos = { startPos.x, startPos.y};
    for (int i = 0; i < numDashes; i++) {
        Vector2 dash_end = Vector2Add(pos, Vector2Scale(direction, dashLength));
        DrawLineEx(pos, dash_end, thick, color);
        pos = dash_end;
        Vector2 space_end = Vector2Add(pos, Vector2Scale(direction, spaceLength));
        // not draw
        pos = space_end;
    }
}

void DrawTextCentered(const char* text, Vector2 pos, int fontSize, Color color) {
    int text_width = MeasureText(text, fontSize);
    DrawText(text, pos.x-text_width/2., pos.y-fontSize/2., fontSize, color);
}

