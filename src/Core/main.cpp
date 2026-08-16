#define SDL_MAIN_HANDLED
#include "Application.h"

int main(int argc, char* argv[]) {
    Application app;

    if (app.Init()) {
        app.Run();
    }

    return 0;
}