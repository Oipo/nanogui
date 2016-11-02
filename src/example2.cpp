/*
    src/example2.cpp -- C++ version of an example application that shows
    how to use the form helper class. For a Python implementation, see
    '../python/example2.py'.

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/nanogui.h>
#include <iostream>
#include <thread>

#define GLFW_INCLUDE_NONE

#if defined(_WIN32)
#  define NOMINMAX
#  undef APIENTRY

#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>

#  define GLFW_EXPOSE_NATIVE_WGL
#  define GLFW_EXPOSE_NATIVE_WIN32
#  include <GLFW/glfw3native.h>
#endif

#include <GLFW/glfw3.h>

#define SCREEN_ID 0

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

nanogui::WindowHandlerConstants constants(GLFW_MOUSE_BUTTON_1, GLFW_MOUSE_BUTTON_2, GLFW_PRESS, GLFW_RELEASE,
GLFW_KEY_LEFT, GLFW_KEY_RIGHT, GLFW_KEY_UP, GLFW_KEY_DOWN, GLFW_KEY_HOME, GLFW_KEY_END, GLFW_KEY_BACKSPACE,
GLFW_KEY_DELETE, GLFW_KEY_ENTER, GLFW_KEY_A, GLFW_KEY_X, GLFW_KEY_C, GLFW_KEY_V, GLFW_MOD_SHIFT,
GLFW_MOD_SUPER);

static float get_pixel_ratio(GLFWwindow *window) {
    Eigen::Vector2i fbSize, size;
    glfwGetFramebufferSize(window, &fbSize[0], &fbSize[1]);
    glfwGetWindowSize(window, &size[0], &size[1]);
    return (float)fbSize[0] / (float)size[0];
}

int main(int /* argc */, char ** /* argv */) {
    nanogui::init(&constants);

    {
        bool use_gl_4_1 = false;// Set to true to create an OpenGL 4.1 context.
        Screen *screen = nullptr;

        glfwSetErrorCallback(
            [](int error, const char *descr) {
                std::cerr << "GLFW error " << error << ": " << descr << std::endl;
            }
        );

        if (!glfwInit())
            throw std::runtime_error("Could not initialize GLFW!");

        glfwSetTime(0);

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        glfwWindowHint(GLFW_SAMPLES, 0);
        glfwWindowHint(GLFW_RED_BITS, 8);
        glfwWindowHint(GLFW_GREEN_BITS, 8);
        glfwWindowHint(GLFW_BLUE_BITS, 8);
        glfwWindowHint(GLFW_ALPHA_BITS, 8);
        glfwWindowHint(GLFW_STENCIL_BITS, 8);
        glfwWindowHint(GLFW_DEPTH_BITS, 24);
        glfwWindowHint(GLFW_VISIBLE, GL_TRUE);
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

        auto mGLFWWindow = glfwCreateWindow(500, 700, "example2", nullptr, nullptr);

        if (!mGLFWWindow) {
            throw std::runtime_error("Could not create an OpenGL 3.3 context!");
        }

        glfwMakeContextCurrent(mGLFWWindow);

        if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
            throw std::runtime_error("Could not initialize GLAD!");
        glGetError(); // pull and ignore unhandled errors like GL_INVALID_ENUM

        std::array<int, 2> mFBSize;
        glfwGetFramebufferSize(mGLFWWindow, &mFBSize[0], &mFBSize[1]);
        glViewport(0, 0, mFBSize[0], mFBSize[1]);
        glClearColor(0.3f, 0.3f, 0.32f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glfwSwapInterval(0);
        glfwPollEvents(); //apple stuff

        constants.setGetTimeCallback([]() {
            return glfwGetTime();
        });
        constants.setGetWindowVisibleCallback([&mGLFWWindow](int) {
            return glfwGetWindowAttrib(mGLFWWindow, GLFW_VISIBLE) != 0;
        });
        constants.setSetClipboardCallback([&mGLFWWindow](int, std::string content) {
            glfwSetClipboardString(mGLFWWindow, content.c_str());
        });
        constants.setGetClipboardCallback([&mGLFWWindow](int) {
            return std::string(glfwGetClipboardString(mGLFWWindow));
        });

        glfwSetCursorPosCallback(mGLFWWindow, [](GLFWwindow*, double x, double y) {
            constants.handleCursorPosEvent(SCREEN_ID, x, y);
        });

        glfwSetMouseButtonCallback(mGLFWWindow, [](GLFWwindow*, int button, int action, int modifiers) {
            constants.handleMouseButtonEvent(SCREEN_ID, button, action, modifiers);
        });

        glfwSetKeyCallback(mGLFWWindow, [](GLFWwindow*, int key, int scancode, int action, int mods) {
            constants.handleKeyEvent(SCREEN_ID, key, scancode, action, mods);
        });

        glfwSetCharCallback(mGLFWWindow, [](GLFWwindow*, unsigned int codepoint) {
            constants.handleUnicodeEvent(SCREEN_ID, codepoint);
        });

        glfwSetDropCallback(mGLFWWindow, [](GLFWwindow*, int count, const char **filenames) {
            constants.handleDropEvent(SCREEN_ID, count, filenames);
        });

        glfwSetScrollCallback(mGLFWWindow, [](GLFWwindow*, double x, double y) {
            constants.handleScrollEvent(SCREEN_ID, x, y);
        });

        /* React to framebuffer size events -- includes window
           size events and also catches things like dragging
           a window from a Retina-capable screen to a normal
           screen on Mac OS X */
        glfwSetFramebufferSizeCallback(mGLFWWindow, [](GLFWwindow *w, int width, int height) {
            auto ratio = get_pixel_ratio(w);
            constants.handleFramebufferSizeEvent(SCREEN_ID, width, height, ratio);
        });

        screen = new Screen(SCREEN_ID, Vector2i(500, 700), get_pixel_ratio(mGLFWWindow));

        bool enabled = true;
        FormHelper *gui = new FormHelper(screen);
        ref<Window> window = gui->addWindow(Eigen::Vector2i(10, 10), "Form helper example");
        gui->addGroup("Basic types");
        gui->addVariable("bool", bvar);
        gui->addVariable("string", strval);

        gui->addGroup("Validating fields");
        gui->addVariable("int", ivar)->setSpinnable(true);
        gui->addVariable("float", fvar);
        gui->addVariable("double", dvar)->setSpinnable(true);

        gui->addGroup("Complex types");
        gui->addVariable("Enumeration", enumval, enabled)
           ->setItems({"Item 1", "Item 2", "Item 3"});
        gui->addVariable("Color", colval);

        gui->addGroup("Other widgets");
        gui->addButton("A button", []() { std::cout << "Button pressed." << std::endl; });

        screen->setVisible(true);
        screen->performLayout();
        window->center();

        {
            screen->drawAll();
            screen->setVisible(true);

            std::thread refresh_thread;
            bool mainloop_active = true;
            int refresh = 50;
            if (refresh > 0) {
                /* If there are no mouse/keyboard events, try to refresh the
                   view roughly every 50 ms (default); this is to support animations
                   such as progress bars while keeping the system load
                   reasonably low */
                refresh_thread = std::thread(
                    [refresh, &mainloop_active]() {
                        std::chrono::milliseconds time(refresh);
                        while (mainloop_active) {
                            std::this_thread::sleep_for(time);
                            glfwPostEmptyEvent();
                        }
                    }
                );
            }


            while (mainloop_active) {
                if (glfwWindowShouldClose(mGLFWWindow)) {
                    mainloop_active = false;
                    break;
                }
                glClearColor(0.3f, 0.3f, 0.32f, 1.f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

                screen->drawAll();

                glfwSwapBuffers(mGLFWWindow);
                /* Wait for mouse/keyboard or empty refresh events */
                glfwWaitEvents();
            }

            refresh_thread.join();

            /* Process events once more */
            glfwPollEvents();
        }

        nanogui::shutdown();

        glfwDestroyWindow(mGLFWWindow);
    }

    glfwTerminate();
    return 0;
}
