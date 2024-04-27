#include <Windows.h>

#include "src/engine/engine.h"

int CALLBACK WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPSTR       lpCmdLine,
    int         nCmdShow
) {
    Engine engine;
    engine.Start();
}