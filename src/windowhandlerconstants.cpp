/*
    src/windowhandlerconstants->cpp -- class to be used as global for handling different windowing libraries

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/common.h>

NAMESPACE_BEGIN(nanogui)

/* mouse buttons */

int WindowHandlerConstants::primaryMouseButton() const {
    return mPrimaryMouseButton;
}

int WindowHandlerConstants::secondaryMouseButton() const {
    return mSecondaryMouseButton;
}

int WindowHandlerConstants::mousePress() const {
    return mMousePress;
}

int WindowHandlerConstants::mouseRelease() const {
    return mMouseRelease;
}

/* keyboard buttons */

int WindowHandlerConstants::leftKey() const {
    return mKeyLeft;
}

int WindowHandlerConstants::rightKey() const {
    return mKeyRight;
}

int WindowHandlerConstants::downKey() const {
    return mKeyDown;
}

int WindowHandlerConstants::upKey() const {
    return mKeyUp;
}

int WindowHandlerConstants::homeKey() const {
    return mKeyHome;
}

int WindowHandlerConstants::endKey() const {
    return mKeyEnd;
}

int WindowHandlerConstants::backspaceKey() const {
    return mKeyBackspace;
}

int WindowHandlerConstants::deleteKey() const {
    return mKeyDelete;
}

int WindowHandlerConstants::enterKey() const {
    return mKeyEnter;
}

int WindowHandlerConstants::aKey() const {
    return mKeyA;
}

int WindowHandlerConstants::xKey() const {
    return mKeyX;
}

int WindowHandlerConstants::cKey() const {
    return mKeyC;
}

int WindowHandlerConstants::vKey() const {
    return mKeyV;
}

int WindowHandlerConstants::shiftMod() const {
    return mModShift;
}

int WindowHandlerConstants::controlMod() const {
    return mModControl;
}

int WindowHandlerConstants::commandMod() const {
    return mModCommand;
}

/* common functions needed from window handling libraries */

double WindowHandlerConstants::getTime() const {
    if(!getTimeCallback) {
        throw std::runtime_error("getTimeCallback not initialized");
    }

    return getTimeCallback();
}

void WindowHandlerConstants::setGetTimeCallback(decltype(getTimeCallback) callback) {
    getTimeCallback = callback;
}

bool WindowHandlerConstants::getWindowVisible(int windowId) const {
    if(!getWindowVisibleCallback) {
        throw std::runtime_error("getWindowVisibleCallback not initialized");
    }

    return getWindowVisibleCallback(windowId);
}

void WindowHandlerConstants::setGetWindowVisibleCallback(decltype(getWindowVisibleCallback) callback) {
    getWindowVisibleCallback = callback;
}

void WindowHandlerConstants::setClipboard(int windowId, std::string text) {
    if(!setClipboardCallback) {
        throw std::runtime_error("setClipboardCallback not initialized");
    }

    setClipboardCallback(windowId, text);
}

void WindowHandlerConstants::setSetClipboardCallback(decltype(setClipboardCallback) callback) {
    setClipboardCallback = callback;
}

std::string WindowHandlerConstants::getClipboard(int windowId) const {
    if(!getClipboardCallback) {
        throw std::runtime_error("getClipboardCallback not initialized");
    }

    return getClipboardCallback(windowId);
}

void WindowHandlerConstants::setGetClipboardCallback(decltype(getClipboardCallback) callback) {
    getClipboardCallback = callback;
}


/* callbacks */

void WindowHandlerConstants::addCursorPosCallback(int id, decltype(cursorPosCallbacks)::value_type::second_type callback) {
    cursorPosCallbacks.push_back(std::make_pair(id, callback));
}

void WindowHandlerConstants::removeCursorPosCallback(int id) {
    auto result = std::find_if(std::begin(cursorPosCallbacks), std::end(cursorPosCallbacks),
        [id](decltype(*std::begin(cursorPosCallbacks)) element) { return element.first == id; } );

    if(result != std::end(cursorPosCallbacks)) {
        cursorPosCallbacks.erase(result);
    }
}

void WindowHandlerConstants::handleCursorPosEvent(int screenId, double x, double y) {
    for(auto& callback : cursorPosCallbacks) {
        if(callback.first == screenId) {
            callback.second(x, y);
        }
    }
}

void WindowHandlerConstants::addMouseButtonCallback(int id, decltype(mouseButtonCallbacks)::value_type::second_type callback) {
    mouseButtonCallbacks.push_back(std::make_pair(id, callback));
}

void WindowHandlerConstants::removeMouseButtonCallback(int id) {
    auto result = std::find_if(std::begin(mouseButtonCallbacks), std::end(mouseButtonCallbacks),
        [id](decltype(*std::begin(mouseButtonCallbacks)) element) { return element.first == id; } );

    if(result != std::end(mouseButtonCallbacks)) {
        mouseButtonCallbacks.erase(result);
    }
}

void WindowHandlerConstants::handleMouseButtonEvent(int screenId, int button, int action, int modifiers) {
    for(auto& callback : mouseButtonCallbacks) {
        if(callback.first == screenId) {
            callback.second(button, action, modifiers);
        }
    }
}

void WindowHandlerConstants::addKeyCallback(int id, decltype(keyCallbacks)::value_type::second_type callback) {
    keyCallbacks.push_back(std::make_pair(id, callback));
}

void WindowHandlerConstants::removeKeyCallback(int id) {
    auto result = std::find_if(std::begin(keyCallbacks), std::end(keyCallbacks),
        [id](decltype(*std::begin(keyCallbacks)) element) { return element.first == id; } );

    if(result != std::end(keyCallbacks)) {
        keyCallbacks.erase(result);
    }
}

void WindowHandlerConstants::handleKeyEvent(int screenId, int key, int scancode, int action, int mods) {
    for(auto& callback : keyCallbacks) {
        if(callback.first == screenId) {
            callback.second(key, scancode, action, mods);
        }
    }
}

void WindowHandlerConstants::addUnicodeCallback(int id, decltype(unicodeCallbacks)::value_type::second_type callback) {
    unicodeCallbacks.push_back(std::make_pair(id, callback));
}

void WindowHandlerConstants::removeUnicodeCallback(int id) {
    auto result = std::find_if(std::begin(unicodeCallbacks), std::end(unicodeCallbacks),
        [id](decltype(*std::begin(unicodeCallbacks)) element) { return element.first == id; } );

    if(result != std::end(unicodeCallbacks)) {
        unicodeCallbacks.erase(result);
    }
}

void WindowHandlerConstants::handleUnicodeEvent(int screenId, unsigned int codepoint) {
    for(auto& callback : unicodeCallbacks) {
        if(callback.first == screenId) {
            callback.second(codepoint);
        }
    }
}

void WindowHandlerConstants::addDropCallback(int id, decltype(dropCallbacks)::value_type::second_type callback) {
    dropCallbacks.push_back(std::make_pair(id, callback));
}

void WindowHandlerConstants::removeDropCallback(int id) {
    auto result = std::find_if(std::begin(dropCallbacks), std::end(dropCallbacks),
        [id](decltype(*std::begin(dropCallbacks)) element) { return element.first == id; } );

    if(result != std::end(dropCallbacks)) {
        dropCallbacks.erase(result);
    }
}

void WindowHandlerConstants::handleDropEvent(int screenId, int count, const char **filenames) {
    for(auto& callback : dropCallbacks) {
        if(callback.first == screenId) {
            callback.second(count, filenames);
        }
    }
}

void WindowHandlerConstants::addScrollCallback(int id, decltype(scrollCallbacks)::value_type::second_type callback) {
    scrollCallbacks.push_back(std::make_pair(id, callback));
}

void WindowHandlerConstants::removeScrollCallback(int id) {
    auto result = std::find_if(std::begin(scrollCallbacks), std::end(scrollCallbacks),
        [id](decltype(*std::begin(scrollCallbacks)) element) { return element.first == id; } );

    if(result != std::end(scrollCallbacks)) {
        scrollCallbacks.erase(result);
    }
}

void WindowHandlerConstants::handleScrollEvent(int screenId, double x, double y) {
    for(auto& callback : scrollCallbacks) {
        if(callback.first == screenId) {
            callback.second(x, y);
        }
    }
}

void WindowHandlerConstants::addFramebufferSizeCallback(int id, decltype(framebufferSizeCallbacks)::value_type::second_type callback) {
    framebufferSizeCallbacks.push_back(std::make_pair(id, callback));
}

void WindowHandlerConstants::removeFramebufferSizeCallback(int id) {
    auto result = std::find_if(std::begin(framebufferSizeCallbacks), std::end(framebufferSizeCallbacks),
        [id](decltype(*std::begin(framebufferSizeCallbacks)) element) { return element.first == id; } );

    if(result != std::end(framebufferSizeCallbacks)) {
        framebufferSizeCallbacks.erase(result);
    }
}

void WindowHandlerConstants::handleFramebufferSizeEvent(int screenId, double x, double y, double pixelRatio) {
    for(auto& callback : framebufferSizeCallbacks) {
        if(callback.first == screenId) {
            callback.second(x, y, pixelRatio);
        }
    }
}

NAMESPACE_END(nanogui)
