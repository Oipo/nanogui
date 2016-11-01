/*
    nanogui/common.h -- common definitions used by NanoGUI

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
/** \file */

#pragma once

#include <Eigen/Core>
#include <stdint.h>
#include <array>
#include <vector>
#include <functional>
#include <set>

/* Set to 1 to draw boxes around widgets */
//#define NANOGUI_SHOW_WIDGET_BOUNDS 1

#if !defined(NAMESPACE_BEGIN) || defined(DOXYGEN_DOCUMENTATION_BUILD)
    /**
     * \brief Convenience macro for namespace declarations
     *
     * The macro ``NAMESPACE_BEGIN(nanogui)`` will expand to ``namespace
     * nanogui {``. This is done to hide the namespace scope from editors and
     * C++ code formatting tools that may otherwise indent the entire file.
     * The corresponding ``NAMESPACE_END`` macro also lists the namespace
     * name for improved readability.
     *
     * \param name
     *     The name of the namespace scope to open
     */
    #define NAMESPACE_BEGIN(name) namespace name {
#endif
#if !defined(NAMESPACE_END) || defined(DOXYGEN_DOCUMENTATION_BUILD)
    /**
     * \brief Convenience macro for namespace declarations
     *
     * Closes a namespace (counterpart to ``NAMESPACE_BEGIN``)
     * ``NAMESPACE_END(nanogui)`` will expand to only ``}``.
     *
     * \param name
     *     The name of the namespace scope to close
     */
    #define NAMESPACE_END(name) }
#endif

#if defined(NANOGUI_SHARED)
#  if defined(_WIN32)
#    if defined(NANOGUI_BUILD)
#      define NANOGUI_EXPORT __declspec(dllexport)
#    else
#      define NANOGUI_EXPORT __declspec(dllimport)
#    endif
#  elif defined(NANOGUI_BUILD)
#    define NANOGUI_EXPORT __attribute__ ((visibility("default")))
#  else
#    define NANOGUI_EXPORT
#  endif
#else
     /**
      * If the build flag ``NANOGUI_SHARED`` is defined, this directive will expand
      * to be the platform specific shared library import / export command depending
      * on the compilation stage.  If undefined, it expands to nothing. **Do not**
      * define this directive on your own.
      */
#    define NANOGUI_EXPORT
#endif

/* Force usage of discrete GPU on laptops (macro must be invoked in main application) */
#if defined(_WIN32)
#define NANOGUI_FORCE_DISCRETE_GPU() \
    extern "C" { \
        __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1; \
        __declspec(dllexport) int NvOptimusEnablement = 1; \
    }
#else
/**
 * On Windows, exports ``AmdPowerXpressRequestHighPerformance`` and
 * ``NvOptimusEnablement`` as ``1``.
 */
#define NANOGUI_FORCE_DISCRETE_GPU()
#endif

#if defined(_WIN32)
#if defined(NANOGUI_BUILD)
/* Quench a few warnings on when compiling NanoGUI on Windows */
#pragma warning(disable : 4127) // warning C4127: conditional expression is constant
#pragma warning(disable : 4244) // warning C4244: conversion from X to Y, possible loss of data
#endif
#pragma warning(disable : 4251) // warning C4251: class X needs to have dll-interface to be used by clients of class Y
#pragma warning(disable : 4714) // warning C4714: function X marked as __forceinline not inlined
#endif

// These will produce broken links in the docs build
#ifndef DOXYGEN_SHOULD_SKIP_THIS

struct NVGcontext { /* Opaque handle type, never de-referenced within NanoGUI */ };

struct NVGcolor;
struct NVGglyphPosition;

#endif // DOXYGEN_SHOULD_SKIP_THIS

// Define command key for windows/mac/linux
#ifdef __APPLE__
/// If on OSX, maps to ``GLFW_MOD_SUPER``.  Otherwise, maps to ``GLFW_MOD_CONTROL``.
#define SYSTEM_COMMAND_MOD GLFW_MOD_SUPER
#else
/// If on OSX, maps to ``GLFW_MOD_SUPER``.  Otherwise, maps to ``GLFW_MOD_CONTROL``.
#define SYSTEM_COMMAND_MOD GLFW_MOD_CONTROL
#endif

NAMESPACE_BEGIN(nanogui)

/// Cursor shapes available to use in GLFW.
enum class Cursor {
    Arrow = 0,
    IBeam,
    Crosshair,
    Hand,
    HResize,
    VResize,
    /// Not a cursor --- should always be last: enables a loop over the cursor types.
    CursorCount
};

/* Import some common Eigen types */
using Eigen::Vector2f;
using Eigen::Vector3f;
using Eigen::Vector4f;
using Eigen::Vector2i;
using Eigen::Vector3i;
using Eigen::Vector4i;
using Eigen::Matrix3f;
using Eigen::Matrix4f;
using Eigen::VectorXf;
using Eigen::MatrixXf;

/**
 * Convenience typedef for things like index buffers.  You would use it the same
 * as ``Eigen::MatrixXf``, only it is storing ``uint32_t`` instead of ``float``.
 */
typedef Eigen::Matrix<uint32_t, Eigen::Dynamic, Eigen::Dynamic> MatrixXu;

/**
 * \class Color common.h nanogui/common.h
 *
 * \brief Stores an RGBA floating point color value.
 *
 * This class simply wraps around an ``Eigen::Vector4f``, providing some convenient
 * methods and terminology for thinking of it as a color.  The data operates in the
 * same way as ``Eigen::Vector4f``, and the following values are identical:
 *
 * \rst
 * +---------+-------------+-----------------------+-------------+
 * | Channel | Array Index | Eigen::Vector4f Value | Color Value |
 * +=========+=============+=======================+=============+
 * | Red     | ``0``       | x()                   | r()         |
 * +---------+-------------+-----------------------+-------------+
 * | Green   | ``1``       | y()                   | g()         |
 * +---------+-------------+-----------------------+-------------+
 * | Blue    | ``2``       | z()                   | b()         |
 * +---------+-------------+-----------------------+-------------+
 * | Alpha   | ``3``       | w()                   | w()         |
 * +---------+-------------+-----------------------+-------------+
 *
 * .. note::
 *    The method for the alpha component is **always** ``w()``.
 * \endrst
 *
 * You can and should still use the various convenience methods such as ``any()``,
 * ``all()``, ``head<index>()``, etc provided by Eigen.
 */
class Color : public Eigen::Vector4f {
    typedef Eigen::Vector4f Base;
public:
    /// Default constructor: represents black (``r, g, b, a = 0``)
    Color() : Color(0, 0, 0, 0) {}

    /**
     * Makes an exact copy of the data represented by the input parameter.
     *
     * \param color
     * The four dimensional float vector being copied.
     */
    Color(const Eigen::Vector4f &color) : Eigen::Vector4f(color) { }

    /**
     * Copies (x, y, z) from the input vector, and uses the value specified by
     * the ``alpha`` parameter for this Color object's alpha component.
     *
     * \param color
     * The three dimensional float vector being copied.
     *
     * \param alpha
     * The value to set this object's alpha component to.
     */
    Color(const Eigen::Vector3f &color, float alpha)
        : Color(color(0), color(1), color(2), alpha) { }

    /**
     * Copies (x, y, z) from the input vector, casted as floats first and then
     * divided by ``255.0``, and uses the value specified by the ``alpha``
     * parameter, casted to a float and divided by ``255.0`` as well, for this
     * Color object's alpha component.
     *
     * \param color
     * The three dimensional integer vector being copied, will be divided by ``255.0``.
     *
     * \param alpha
     * The value to set this object's alpha component to, will be divided by ``255.0``.
     */
    Color(const Eigen::Vector3i &color, int alpha)
        : Color(color.cast<float>() / 255.f, alpha / 255.f) { }

    /**
     * Copies (x, y, z) from the input vector, and sets the alpha of this color
     * to be ``1.0``.
     *
     * \param color
     * The three dimensional float vector being copied.
     */
    Color(const Eigen::Vector3f &color) : Color(color, 1.0f) {}

    /**
     * Copies (x, y, z) from the input vector, casting to floats and dividing by
     * ``255.0``.  The alpha of this color will be set to ``1.0``.
     *
     * \param color
     * The three dimensional integer vector being copied, will be divided by ``255.0``.
     */
    Color(const Eigen::Vector3i &color)
        : Color((Vector3f)(color.cast<float>() / 255.f)) { }

    /**
     * Copies (x, y, z, w) from the input vector, casting to floats and dividing
     * by ``255.0``.
     *
     * \param color
     * The three dimensional integer vector being copied, will be divided by ``255.0``.
     */
    Color(const Eigen::Vector4i &color)
        : Color((Vector4f)(color.cast<float>() / 255.f)) { }

    /**
     * Creates the Color ``(intensity, intensity, intensity, alpha)``.
     *
     * \param intensity
     * The value to be used for red, green, and blue.
     *
     * \param alpha
     * The alpha component of the color.
     */
    Color(float intensity, float alpha)
        : Color(Vector3f::Constant(intensity), alpha) { }

    /**
     * Creates the Color ``(intensity, intensity, intensity, alpha) / 255.0``.
     * Values are casted to floats before division.
     *
     * \param intensity
     * The value to be used for red, green, and blue, will be divided by ``255.0``.
     *
     * \param alpha
     * The alpha component of the color, will be divided by ``255.0``.
     */
    Color(int intensity, int alpha)
        : Color(Vector3i::Constant(intensity), alpha) { }

    /**
     * Explicit constructor: creates the Color ``(r, g, b, a)``.
     *
     * \param r
     * The red component of the color.
     *
     * \param g
     * The green component of the color.
     *
     * \param b
     * The blue component of the color.
     *
     * \param a
     * The alpha component of the color.
     */
    Color(float r, float g, float b, float a) : Color(Vector4f(r, g, b, a)) { }

    /**
     * Explicit constructor: creates the Color ``(r, g, b, a) / 255.0``.
     * Values are casted to floats before division.
     *
     * \param r
     * The red component of the color, will be divided by ``255.0``.
     *
     * \param g
     * The green component of the color, will be divided by ``255.0``.
     *
     * \param b
     * The blue component of the color, will be divided by ``255.0``.
     *
     * \param a
     * The alpha component of the color, will be divided by ``255.0``.
     */
    Color(int r, int g, int b, int a) : Color(Vector4i(r, g, b, a)) { }

    /// Construct a color vector from MatrixBase (needed to play nice with Eigen)
    template <typename Derived> Color(const Eigen::MatrixBase<Derived>& p)
        : Base(p) { }

    /// Assign a color vector from MatrixBase (needed to play nice with Eigen)
    template <typename Derived> Color &operator=(const Eigen::MatrixBase<Derived>& p) {
        this->Base::operator=(p);
        return *this;
    }

    /// Return a reference to the red channel
    float &r() { return x(); }
    /// Return a reference to the red channel (const version)
    const float &r() const { return x(); }
    /// Return a reference to the green channel
    float &g() { return y(); }
    /// Return a reference to the green channel (const version)
    const float &g() const { return y(); }
    /// Return a reference to the blue channel
    float &b() { return z(); }
    /// Return a reference to the blue channel (const version)
    const float &b() const { return z(); }

    /**
     * Computes the luminance as ``l = 0.299r + 0.587g + 0.144b + 0.0a``.  If
     * the luminance is less than 0.5, white is returned.  If the luminance is
     * greater than or equal to 0.5, black is returned.  Both returns will have
     * an alpha component of 1.0.
     */
    Color contrastingColor() const {
        float luminance = cwiseProduct(Color(0.299f, 0.587f, 0.144f, 0.f)).sum();
        return Color(luminance < 0.5f ? 1.f : 0.f, 1.f);
    }

    /// Allows for conversion between this Color and NanoVG's representation.
    inline operator const NVGcolor &() const;
};

class WindowHandlerConstants {
private:
    int mPrimaryMouseButton;
    int mSecondaryMouseButton;
    int mMousePress;
    int mMouseRelease;
    int mKeyLeft;
    int mKeyRight;
    int mKeyUp;
    int mKeyDown;
    int mKeyHome;
    int mKeyEnd;
    int mKeyBackspace;
    int mKeyDelete;
    int mKeyEnter;
    int mKeyA;
    int mKeyX;
    int mKeyC;
    int mKeyV;
    int mModShift;
    int mModControl;
    int mModCommand;

    /* common */
    std::function<double()> getTimeCallback;
    std::function<bool(int)> getWindowVisibleCallback;
    std::function<void(int, std::string)> setClipboardCallback;
    std::function<std::string(int)> getClipboardCallback;

    /* events */
    std::vector<std::pair<int, std::function<void(double, double)>>> cursorPosCallbacks;
    std::vector<std::pair<int, std::function<void(int, int, int)>>> mouseButtonCallbacks;
    std::vector<std::pair<int, std::function<void(int, int, int, int)>>> keyCallbacks;
    std::vector<std::pair<int, std::function<void(unsigned int)>>> unicodeCallbacks;
    std::vector<std::pair<int, std::function<void(int, const char**)>>> dropCallbacks;
    std::vector<std::pair<int, std::function<void(double, double)>>> scrollCallbacks;
    std::vector<std::pair<int, std::function<void(int, int, float)>>> framebufferSizeCallbacks;
public:
    WindowHandlerConstants() : mPrimaryMouseButton(-1), mSecondaryMouseButton(-1), mMousePress(-1),
        mMouseRelease(-1), mKeyLeft(-1), mKeyRight(-1), mKeyUp(-1), mKeyDown(-1), mKeyHome(-1), mKeyEnd(-1),
        mKeyBackspace(-1), mKeyDelete(-1), mKeyEnter(-1), mKeyA(-1), mKeyX(-1), mKeyC(-1), mKeyV(-1), mModShift(-1),
        mModControl(-1), mModCommand(-1) { }
    /*
     * \param primaryButton
     *     Set to the value of your window handler. E.g. SDL_BUTTON_LEFT in SDL2 or
     *     GLFW_MOUSE_BUTTON_1 in GLFW
     */
    WindowHandlerConstants(int const primaryMouseButton, int const secondaryMouseButton,
        int const mousePress, int const mouseRelease, int const keyLeft, int const keyRight, int const keyUp,
        int const keyDown, int const keyHome, int const keyEnd, int const keyBackspace, int const keyDelete,
        int const keyEnter, int const keyA, int const keyX, int const keyC, int const keyV, int const modShift,
        int const modCommand)
        : mPrimaryMouseButton(primaryMouseButton), mSecondaryMouseButton(secondaryMouseButton),
        mMousePress(mousePress), mMouseRelease(mouseRelease), mKeyLeft(keyLeft), mKeyRight(keyRight), mKeyUp(keyUp),
        mKeyDown(keyDown), mKeyHome(keyHome), mKeyEnd(keyEnd), mKeyBackspace(keyBackspace), mKeyDelete(keyDelete),
        mKeyEnter(keyEnter), mKeyA(keyA), mKeyX(keyX), mKeyC(keyC), mKeyV(keyV), mModShift(modShift),
        mModCommand(modCommand)
    { }

    WindowHandlerConstants(WindowHandlerConstants&& constants) = delete;
    WindowHandlerConstants(const WindowHandlerConstants&) = delete;

    //WindowHandlerConstants& operator=(const WindowHandlerConstants& other);

    /* mouse buttons */

    int primaryMouseButton() const;

    int secondaryMouseButton() const;

    int mousePress() const;

    int mouseRelease() const;

    /* keyboard buttons */

    int leftKey() const;

    int rightKey() const;

    int downKey() const;

    int upKey() const;

    int homeKey() const;

    int endKey() const;

    int backspaceKey() const;

    int deleteKey() const;

    int enterKey() const;

    int aKey() const;

    int xKey() const;

    int cKey() const;

    int vKey() const;

    int shiftMod() const;

    int controlMod() const;

    int commandMod() const;

    /* common functions needed from window handling libraries */

    double getTime() const;

    void setGetTimeCallback(decltype(getTimeCallback) callback);

    bool getWindowVisible(int windowId) const;

    void setGetWindowVisibleCallback(decltype(getWindowVisibleCallback) callback);

    void setClipboard(int windowId, std::string text);

    void setSetClipboardCallback(decltype(setClipboardCallback) callback);

    std::string getClipboard(int windowId) const;

    void setGetClipboardCallback(decltype(getClipboardCallback) callback);

    /* callbacks */

    void addCursorPosCallback(int id, decltype(cursorPosCallbacks)::value_type::second_type callback);

    void removeCursorPosCallback(int id);

    void handleCursorPosEvent(int screenId, double x, double y);

    void addMouseButtonCallback(int id, decltype(mouseButtonCallbacks)::value_type::second_type callback);

    void removeMouseButtonCallback(int id);

    void handleMouseButtonEvent(int screenId, int button, int action, int modifiers);

    void addKeyCallback(int id, decltype(keyCallbacks)::value_type::second_type callback);

    void removeKeyCallback(int id);

    void handleKeyEvent(int screenId, int key, int scancode, int action, int mods);

    void addUnicodeCallback(int id, decltype(unicodeCallbacks)::value_type::second_type callback);

    void removeUnicodeCallback(int id);

    void handleUnicodeEvent(int screenId, unsigned int codepoint);

    void addDropCallback(int id, decltype(dropCallbacks)::value_type::second_type callback);

    void removeDropCallback(int id);

    void handleDropEvent(int screenId, int count, const char **filenames);

    void addScrollCallback(int id, decltype(scrollCallbacks)::value_type::second_type callback);

    void removeScrollCallback(int id);

    void handleScrollEvent(int screenId, double x, double y);

    void addFramebufferSizeCallback(int id, decltype(framebufferSizeCallbacks)::value_type::second_type callback);

    void removeFramebufferSizeCallback(int id);

    void handleFramebufferSizeEvent(int screenId, double x, double y, double pixelRatio);
};

// skip the forward declarations for the docs
#ifndef DOXYGEN_SHOULD_SKIP_THIS

/* Forward declarations */
template <typename T> class ref;
class AdvancedGridLayout;
class BoxLayout;
class Button;
class CheckBox;
class ColorWheel;
class ColorPicker;
class ComboBox;
class GLFramebuffer;
class GLShader;
class GridLayout;
class GroupLayout;
class ImagePanel;
class ImageView;
class Label;
class Layout;
class MessageDialog;
class Object;
class Popup;
class PopupButton;
class ProgressBar;
class Screen;
class Serializer;
class Slider;
class StackedWidget;
class TabHeader;
class TabWidget;
class TextBox;
class Theme;
class ToolButton;
class VScrollPanel;
class Widget;
class Window;

#endif // DOXYGEN_SHOULD_SKIP_THIS

/**
 * Static initialization; should be called once before invoking **any** NanoGUI
 * functions **if** you are having NanoGUI manage OpenGL / GLFW.  This method
 * is effectively a wrapper call to ``glfwInit()``, so if you are managing
 * OpenGL / GLFW on your own *do not call this method*.
 *
 * \param WindowHandlerConstants
 *     Set constants related to the used window handler. E.g. SDL_BUTTON_LEFT in SDL2 or
 *     GLFW_MOUSE_BUTTON_1 in GLFW for the primary mouse button.
 *
 * \rst
 * Refer to :ref:`nanogui_example_3` for how you might go about managing OpenGL
 * and GLFW on your own, while still using NanoGUI's classes.
 * \endrst
 */
extern NANOGUI_EXPORT void init(WindowHandlerConstants* constants);

/// Static shutdown; should be called before the application terminates.
extern NANOGUI_EXPORT void shutdown();

// internal helper function to get window handler constants
extern WindowHandlerConstants* get_window_handler_constants();

/**
 * \brief Open a native file open/save dialog.
 *
 * \param filetypes
 *     Pairs of permissible formats with descriptions like
 *     ``("png", "Portable Network Graphics")``.
 *
 * \param save
 *     Set to ``true`` if you would like subsequent file dialogs to open
 *     at whatever folder they were in when they close this one.
 */
extern NANOGUI_EXPORT std::string
file_dialog(const std::vector<std::pair<std::string, std::string>> &filetypes,
            bool save);

#if defined(__APPLE__) || defined(DOXYGEN_DOCUMENTATION_BUILD)
/**
 * \brief Move to the application bundle's parent directory
 *
 * This is function is convenient when deploying .app bundles on OSX. It
 * adjusts the file path to the parent directory containing the bundle.
 */
extern NANOGUI_EXPORT void chdir_to_bundle_parent();
#endif

/**
 * \brief Convert a single UTF32 character code to UTF8.
 *
 * \rst
 * NanoGUI uses this to convert the icon character codes
 * defined in :ref:`file_include_nanogui_entypo.h`.
 * \endrst
 *
 * \param c
 *     The UTF32 character to be converted.
 */
extern NANOGUI_EXPORT std::array<char, 8> utf8(int c);

/// Load a directory of PNG images and upload them to the GPU (suitable for use with ImagePanel)
extern NANOGUI_EXPORT std::vector<std::pair<int, std::string>>
    loadImageDirectory(NVGcontext *ctx, const std::string &path);

/// Convenience function for instanting a PNG icon from the application's data segment (via bin2c)
#define nvgImageIcon(ctx, name) nanogui::__nanogui_get_image(ctx, #name, name##_png, name##_png_size)
/// Helper function used by nvgImageIcon
extern NANOGUI_EXPORT int __nanogui_get_image(NVGcontext *ctx, const std::string &name, uint8_t *data, uint32_t size);

NAMESPACE_END(nanogui)
