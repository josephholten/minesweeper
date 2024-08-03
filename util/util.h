#include "raylib.h"

int GetRandomSign();
void CustomLog(int logLevel, const char *text, va_list args);
void DrawLineExDashed(Vector2 startPos, Vector2 endPos, float thick, Color color, float dashLength);
void DrawTextCentered(const char* text, Vector2 pos, int fontSize, Color color);