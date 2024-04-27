#include <Windows.h>

#include "src/engine/engine.h"

#ifdef _DEBUG
int main() {
    Engine engine;
    engine.Start();
}
#else
int CALLBACK WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPSTR       lpCmdLine,
    int         nCmdShow
) {
    Engine engine;
    engine.Start();
}
#endif // !_DEBUG