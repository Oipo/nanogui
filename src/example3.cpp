/*
    src/example3.cpp -- C++ version of an example application that shows
    how to use nanogui in an SDL2 application.

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#if defined(NANOGUI_SHARED) && !defined(GLAD_GLAPI_EXPORT)
    #define GLAD_GLAPI_EXPORT
#endif

#include <glad/glad.h>

#include <SDL.h>
#include <SDL_opengl.h>

#include <nanogui/nanogui.h>
#include <iostream>

using namespace nanogui;

enum test_enum {
    Item1 = 0,
    Item2,
    Item3
};

bool bvar = true;
int ivar = 12345678;
double dvar = 3.1415926;
float fvar = (float)dvar;
std::string strval = "A string";
test_enum enumval = Item2;
Color colval(0.5f, 0.5f, 0.7f, 1.f);

SDL_Window *window = nullptr;
SDL_GLContext context = nullptr;
Screen *screen = nullptr;

#define SCREEN_ID 0

using namespace std;

nanogui::WindowHandlerConstants constants(SDL_BUTTON_LEFT, SDL_BUTTON_RIGHT, SDL_PRESSED, SDL_RELEASED,
SDLK_LEFT, SDLK_RIGHT, SDLK_UP, SDLK_DOWN, SDLK_HOME, SDLK_END, SDLK_BACKSPACE,
SDLK_DELETE, SDLK_RETURN, SDLK_a, SDLK_x, SDLK_c, SDLK_v, KMOD_SHIFT,
KMOD_GUI);

int main(int /* argc */, char ** /* argv */) {

    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        cout << "SDL Init went wrong: " << SDL_GetError() << endl;
        exit(1);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    window = SDL_CreateWindow("example3", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if(window == nullptr) {
        cout << "Couldn't initialize window: " << SDL_GetError() << endl;
        throw std::runtime_error("Could not initialize window!");
    }

    context = SDL_GL_CreateContext(window);
    if(context == nullptr) {
        cout << "Couldn't initialize context: " << SDL_GetError() << endl;
        throw std::runtime_error("Could not initialize context!");
    }

    if (!gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress))
        throw std::runtime_error("Could not initialize GLAD!");
    glGetError(); // pull and ignore unhandled errors like GL_INVALID_ENUM

    if(SDL_GL_SetSwapInterval(1) < 0) {
        cout << "[main] Couldn't initialize vsync: " << SDL_GetError() << endl;
        throw std::runtime_error("Could not initialize vsync!");
    }

    if(SDL_GL_MakeCurrent(window, context) < 0) {
        cout << "[main] Couldn't make OpenGL context current: " << SDL_GetError() << endl;
        throw std::runtime_error("Could not make OpenGL context current!");
    }

    glViewport(0, 0, 800, 600);

    constants.setGetTimeCallback([]() {
        return SDL_GetTicks()/1000.f;
    });
    constants.setGetWindowVisibleCallback([](int) {
        return (SDL_GetWindowFlags(window) & SDL_WINDOW_SHOWN) == SDL_WINDOW_SHOWN;
    });
    constants.setSetClipboardCallback([](int, std::string content) {
        SDL_SetClipboardText(content.c_str());
    });
    constants.setGetClipboardCallback([](int) {
        auto chrArr = SDL_GetClipboardText();
        auto str = std::string(chrArr);
        SDL_free(chrArr);
        return str;
    });

    nanogui::init(&constants);

    screen = new Screen(SCREEN_ID, Vector2i(800, 600), 1);

    // Create nanogui gui
    bool enabled = true;
    FormHelper *gui = new FormHelper(screen);
    nanogui::ref<Window> nanoguiWindow = gui->addWindow(Eigen::Vector2i(10, 10), "Form helper example");
    gui->addGroup("Basic types");
    gui->addVariable("bool", bvar)->setTooltip("Test tooltip.");
    gui->addVariable("string", strval);

    gui->addGroup("Validating fields");
    gui->addVariable("int", ivar)->setSpinnable(true);
    gui->addVariable("float", fvar)->setTooltip("Test.");
    gui->addVariable("double", dvar)->setSpinnable(true);

    gui->addGroup("Complex types");
    gui->addVariable("Enumeration", enumval, enabled)->setItems({ "Item 1", "Item 2", "Item 3" });
    gui->addVariable("Color", colval);

    gui->addGroup("Other widgets");
    gui->addButton("A button", []() { std::cout << "Button pressed." << std::endl; })->setTooltip("Testing a much longer tooltip, that will wrap around to new lines multiple times.");;

    screen->setVisible(true);
    screen->performLayout();
    nanoguiWindow->center();

    // Game loop
    bool quit = false;
    SDL_Event e;
    auto keyboardState = SDL_GetKeyboardState(nullptr);
    SDL_StartTextInput();
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    while (!quit) {
        while(SDL_PollEvent(&e) != 0) {
            if(e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)) {
                quit = true;
            } else if(e.type == SDL_TEXTINPUT) {
                for(int i = 0; i < 32; i++) {
                    if(e.text.text[i] == 0) {
                        break;
                    }
                    constants.handleUnicodeEvent(SCREEN_ID, e.text.text[i]);
                }
            } else if(e.type == SDL_MOUSEMOTION) {
                constants.handleCursorPosEvent(SCREEN_ID, e.motion.x, e.motion.y);
            } else if(e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) {
                int modifiers = 0;
                if(keyboardState[SDL_SCANCODE_LSHIFT] || keyboardState[SDL_SCANCODE_RSHIFT]) {
                    modifiers = KMOD_SHIFT;
                }
                if(keyboardState[SDL_SCANCODE_RGUI] || keyboardState[SDL_SCANCODE_LGUI]) {
                    modifiers |= KMOD_GUI;
                }
                constants.handleMouseButtonEvent(SCREEN_ID, e.button.button, e.button.state, modifiers);
            } else if(e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
                constants.handleKeyEvent(SCREEN_ID, e.key.keysym.sym, e.key.keysym.scancode, e.key.state, e.key.keysym.mod);
            } else if(e.type == SDL_DROPFILE) {
                auto dropped_filedir = e.drop.file;
                const char** filedir = const_cast<const char**>(&dropped_filedir);
                cout << "dropped file " << filedir[0] << endl;
                constants.handleDropEvent(SCREEN_ID, 1, static_cast<const char**>(filedir));
                SDL_free(dropped_filedir);
            } else if(e.type == SDL_MOUSEWHEEL) {
                constants.handleScrollEvent(SCREEN_ID, e.wheel.x, e.wheel.y);
            } /* no SDL2 handleFramebufferSizeEvent equivalent? */
        }

        glClearColor(0.2f, 0.25f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw nanogui
        screen->drawContents();
        screen->drawWidgets();

        SDL_GL_SwapWindow(window);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
