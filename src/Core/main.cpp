#define SDL_MAIN_HANDLED
#include "Application.h"

/**
 * @brief The absolute starting point of the program, which creates the application instance 
 *        and triggers its main execution loop.
 */
int main(int argc, char* argv[]) {
    Application app;

    if (app.Init()) {
        app.Run();
    }

    return 0;
}