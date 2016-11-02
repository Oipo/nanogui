/*
    src/example1.cpp -- C++ version of an example application that shows
    how to use the various widget classes. For a Python implementation, see
    '../python/example1.py'.

    NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/opengl.h>
#include <nanogui/glutil.h>
#include <nanogui/screen.h>
#include <nanogui/window.h>
#include <nanogui/layout.h>
#include <nanogui/label.h>
#include <nanogui/checkbox.h>
#include <nanogui/button.h>
#include <nanogui/toolbutton.h>
#include <nanogui/popupbutton.h>
#include <nanogui/combobox.h>
#include <nanogui/progressbar.h>
#include <nanogui/entypo.h>
#include <nanogui/messagedialog.h>
#include <nanogui/textbox.h>
#include <nanogui/slider.h>
#include <nanogui/imagepanel.h>
#include <nanogui/imageview.h>
#include <nanogui/vscrollpanel.h>
#include <nanogui/colorwheel.h>
#include <nanogui/graph.h>
#include <nanogui/tabwidget.h>
#include <iostream>
#include <string>

// Includes for the GLTexture class.
#include <cstdint>
#include <memory>
#include <utility>
#include <thread>
#include <chrono>

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

#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#if defined(_WIN32)
#  pragma warning(push)
#  pragma warning(disable: 4457 4456 4005 4312)
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#if defined(_WIN32)
#  pragma warning(pop)
#endif
#if defined(_WIN32)
#  if defined(APIENTRY)
#    undef APIENTRY
#  endif
#  include <windows.h>
#endif

#define SCREEN_ID 0

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::pair;
using std::to_string;

class GLTexture {
public:
    using handleType = std::unique_ptr<uint8_t[], void(*)(void*)>;
    GLTexture() = default;
    GLTexture(const std::string& textureName)
        : mTextureName(textureName), mTextureId(0) {}

    GLTexture(const std::string& textureName, GLint textureId)
        : mTextureName(textureName), mTextureId(textureId) {}

    GLTexture(const GLTexture& other) = delete;
    GLTexture(GLTexture&& other) noexcept
        : mTextureName(std::move(other.mTextureName)),
        mTextureId(other.mTextureId) {
        other.mTextureId = 0;
    }
    GLTexture& operator=(const GLTexture& other) = delete;
    GLTexture& operator=(GLTexture&& other) noexcept {
        mTextureName = std::move(other.mTextureName);
        std::swap(mTextureId, other.mTextureId);
        return *this;
    }
    ~GLTexture() noexcept {
        if (mTextureId)
            glDeleteTextures(1, &mTextureId);
    }

    GLuint texture() const { return mTextureId; }
    const std::string& textureName() const { return mTextureName; }

    /**
    *  Load a file in memory and create an OpenGL texture.
    *  Returns a handle type (an std::unique_ptr) to the loaded pixels.
    */
    handleType load(const std::string& fileName) {
        if (mTextureId) {
            glDeleteTextures(1, &mTextureId);
            mTextureId = 0;
        }
        int force_channels = 0;
        int w, h, n;
        handleType textureData(stbi_load(fileName.c_str(), &w, &h, &n, force_channels), stbi_image_free);
        if (!textureData)
            throw std::invalid_argument("Could not load texture data from file " + fileName);
        glGenTextures(1, &mTextureId);
        glBindTexture(GL_TEXTURE_2D, mTextureId);
        GLint internalFormat;
        GLint format;
        switch (n) {
            case 1: internalFormat = GL_R8; format = GL_RED; break;
            case 2: internalFormat = GL_RG8; format = GL_RG; break;
            case 3: internalFormat = GL_RGB8; format = GL_RGB; break;
            case 4: internalFormat = GL_RGBA8; format = GL_RGBA; break;
            default: internalFormat = 0; format = 0; break;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, textureData.get());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        return textureData;
    }

private:
    std::string mTextureName;
    GLuint mTextureId;
};

class ExampleApplication : public nanogui::Screen {
public:
    ExampleApplication(float pixel_ratio) : nanogui::Screen(SCREEN_ID, Eigen::Vector2i(1024, 768), pixel_ratio) {
        using namespace nanogui;

        Window *window = new Window(this, "Button demo");
        window->setPosition(Vector2i(15, 15));
        window->setLayout(new GroupLayout());

        /* No need to store a pointer, the data structure will be automatically
           freed when the parent window is deleted */
        new Label(window, "Push buttons", "sans-bold");

        Button *b = new Button(window, "Plain button");
        b->setCallback([] { cout << "pushed!" << endl; });
        b->setTooltip("short tooltip");

        /* Alternative construction notation using variadic template */
        b = window->add<Button>("Styled", ENTYPO_ICON_ROCKET);
        b->setBackgroundColor(Color(0, 0, 255, 25));
        b->setCallback([] { cout << "pushed!" << endl; });
        b->setTooltip("This button has a fairly long tooltip. It is so long, in "
                "fact, that the shown text will span several lines.");

        new Label(window, "Toggle buttons", "sans-bold");
        b = new Button(window, "Toggle me");
        b->setFlags(Button::ToggleButton);
        b->setChangeCallback([](bool state) { cout << "Toggle button state: " << state << endl; });

        new Label(window, "Radio buttons", "sans-bold");
        b = new Button(window, "Radio button 1");
        b->setFlags(Button::RadioButton);
        b = new Button(window, "Radio button 2");
        b->setFlags(Button::RadioButton);

        new Label(window, "A tool palette", "sans-bold");
        Widget *tools = new Widget(window);
        tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 6));

        b = new ToolButton(tools, ENTYPO_ICON_CLOUD);
        b = new ToolButton(tools, ENTYPO_ICON_FF);
        b = new ToolButton(tools, ENTYPO_ICON_COMPASS);
        b = new ToolButton(tools, ENTYPO_ICON_INSTALL);

        new Label(window, "Popup buttons", "sans-bold");
        PopupButton *popupBtn = new PopupButton(window, "Popup", ENTYPO_ICON_EXPORT);
        Popup *popup = popupBtn->popup();
        popup->setLayout(new GroupLayout());
        new Label(popup, "Arbitrary widgets can be placed here");
        new CheckBox(popup, "A check box");
        popupBtn = new PopupButton(popup, "Recursive popup", ENTYPO_ICON_FLASH);
        popup = popupBtn->popup();
        popup->setLayout(new GroupLayout());
        new CheckBox(popup, "Another check box");

        window = new Window(this, "Basic widgets");
        window->setPosition(Vector2i(200, 15));
        window->setLayout(new GroupLayout());

        new Label(window, "Message dialog", "sans-bold");
        tools = new Widget(window);
        tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 6));
        b = new Button(tools, "Info");
        b->setCallback([&] {
            auto dlg = new MessageDialog(this, MessageDialog::Type::Information, "Title", "This is an information message");
            dlg->setCallback([](int result) { cout << "Dialog result: " << result << endl; });
        });
        b = new Button(tools, "Warn");
        b->setCallback([&] {
            auto dlg = new MessageDialog(this, MessageDialog::Type::Warning, "Title", "This is a warning message");
            dlg->setCallback([](int result) { cout << "Dialog result: " << result << endl; });
        });
        b = new Button(tools, "Ask");
        b->setCallback([&] {
            auto dlg = new MessageDialog(this, MessageDialog::Type::Warning, "Title", "This is a question message", "Yes", "No", true);
            dlg->setCallback([](int result) { cout << "Dialog result: " << result << endl; });
        });

        vector<pair<int, string>>
            icons = loadImageDirectory(mNVGContext, "icons");
        #if defined(_WIN32)
            string resourcesFolderPath("../resources/");
        #else
            string resourcesFolderPath("./");
        #endif

        new Label(window, "Image panel & scroll panel", "sans-bold");
        PopupButton *imagePanelBtn = new PopupButton(window, "Image Panel");
        imagePanelBtn->setIcon(ENTYPO_ICON_FOLDER);
        popup = imagePanelBtn->popup();
        VScrollPanel *vscroll = new VScrollPanel(popup);
        ImagePanel *imgPanel = new ImagePanel(vscroll);
        imgPanel->setImages(icons);
        popup->setFixedSize(Vector2i(245, 150));

        auto imageWindow = new Window(this, "Selected image");
        imageWindow->setPosition(Vector2i(710, 15));
        imageWindow->setLayout(new GroupLayout());

        // Load all of the images by creating a GLTexture object and saving the pixel data.
        for (auto& icon : icons) {
            GLTexture texture(icon.second);
            auto data = texture.load(resourcesFolderPath + icon.second + ".png");
            mImagesData.emplace_back(std::move(texture), std::move(data));
        }

        // Set the first texture
        auto imageView = new ImageView(imageWindow, mImagesData[0].first.texture());
        mCurrentImage = 0;
        // Change the active textures.
        imgPanel->setCallback([this, imageView, imgPanel](int i) {
            imageView->bindImage(mImagesData[i].first.texture());
            mCurrentImage = i;
            cout << "Selected item " << i << '\n';
        });
        imageView->setGridThreshold(20);
        imageView->setPixelInfoThreshold(20);
        imageView->setPixelInfoCallback(
            [this, imageView](const Vector2i& index) -> pair<string, Color> {
            auto& imageData = mImagesData[mCurrentImage].second;
            auto& textureSize = imageView->imageSize();
            string stringData;
            uint16_t channelSum = 0;
            for (int i = 0; i != 4; ++i) {
                auto& channelData = imageData[4*index.y()*textureSize.x() + 4*index.x() + i];
                channelSum += channelData;
                stringData += (to_string(static_cast<int>(channelData)) + "\n");
            }
            float intensity = static_cast<float>(255 - (channelSum / 4)) / 255.0f;
            float colorScale = intensity > 0.5f ? (intensity + 1) / 2 : intensity / 2;
            Color textColor = Color(colorScale, 1.0f);
            return { stringData, textColor };
        });

        new Label(window, "File dialog", "sans-bold");
        tools = new Widget(window);
        tools->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 6));
        b = new Button(tools, "Open");
        b->setCallback([&] {
            cout << "File dialog result: " << file_dialog(
                    { {"png", "Portable Network Graphics"}, {"txt", "Text file"} }, false) << endl;
        });
        b = new Button(tools, "Save");
        b->setCallback([&] {
            cout << "File dialog result: " << file_dialog(
                    { {"png", "Portable Network Graphics"}, {"txt", "Text file"} }, true) << endl;
        });

        new Label(window, "Combo box", "sans-bold");
        new ComboBox(window, { "Combo box item 1", "Combo box item 2", "Combo box item 3"});
        new Label(window, "Check box", "sans-bold");
        CheckBox *cb = new CheckBox(window, "Flag 1",
            [](bool state) { cout << "Check box 1 state: " << state << endl; }
        );
        cb->setChecked(true);
        cb = new CheckBox(window, "Flag 2",
            [](bool state) { cout << "Check box 2 state: " << state << endl; }
        );
        new Label(window, "Progress bar", "sans-bold");
        mProgress = new ProgressBar(window);

        new Label(window, "Slider and text box", "sans-bold");

        Widget *panel = new Widget(window);
        panel->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 20));

        Slider *slider = new Slider(panel);
        slider->setValue(0.5f);
        slider->setFixedWidth(80);

        TextBox *textBox = new TextBox(panel);
        textBox->setFixedSize(Vector2i(60, 25));
        textBox->setValue("50");
        textBox->setUnits("%");
        slider->setCallback([textBox](float value) {
            textBox->setValue(std::to_string((int) (value * 100)));
        });
        slider->setFinalCallback([&](float value) {
            cout << "Final slider value: " << (int) (value * 100) << endl;
        });
        textBox->setFixedSize(Vector2i(60,25));
        textBox->setFontSize(20);
        textBox->setAlignment(TextBox::Alignment::Right);

        window = new Window(this, "Misc. widgets");
        window->setPosition(Vector2i(425,15));
        window->setLayout(new GroupLayout());

        TabWidget* tabWidget = window->add<TabWidget>();

        Widget* layer = tabWidget->createTab("Color Wheel");
        layer->setLayout(new GroupLayout());

        // Use overloaded variadic add to fill the tab widget with Different tabs.
        layer->add<Label>("Color wheel widget", "sans-bold");
        layer->add<ColorWheel>();

        layer = tabWidget->createTab("Function Graph");
        layer->setLayout(new GroupLayout());

        layer->add<Label>("Function graph widget", "sans-bold");

        Graph *graph = layer->add<Graph>("Some Function");

        graph->setHeader("E = 2.35e-3");
        graph->setFooter("Iteration 89");
        VectorXf &func = graph->values();
        func.resize(100);
        for (int i = 0; i < 100; ++i)
            func[i] = 0.5f * (0.5f * std::sin(i / 10.f) +
                              0.5f * std::cos(i / 23.f) + 1);

        // Dummy tab used to represent the last tab button.
        tabWidget->createTab("+");

        // A simple counter.
        int counter = 1;
        tabWidget->setCallback([tabWidget, this, counter] (int index) mutable {
            if (index == (tabWidget->tabCount()-1)) {
                // When the "+" tab has been clicked, simply add a new tab.
                string tabName = "Dynamic " + to_string(counter);
                Widget* layerDyn = tabWidget->createTab(index, tabName);
                layerDyn->setLayout(new GroupLayout());
                layerDyn->add<Label>("Function graph widget", "sans-bold");
                Graph *graphDyn = layerDyn->add<Graph>("Dynamic function");

                graphDyn->setHeader("E = 2.35e-3");
                graphDyn->setFooter("Iteration " + to_string(index*counter));
                VectorXf &funcDyn = graphDyn->values();
                funcDyn.resize(100);
                for (int i = 0; i < 100; ++i)
                    funcDyn[i] = 0.5f *
                        std::abs((0.5f * std::sin(i / 10.f + counter) +
                                  0.5f * std::cos(i / 23.f + 1 + counter)));
                ++counter;
                // We must invoke perform layout from the screen instance to keep everything in order.
                // This is essential when creating tabs dynamically.
                performLayout();
                // Ensure that the newly added header is visible on screen
                tabWidget->ensureTabVisible(index);

            }
        });
        tabWidget->setActiveTab(0);

        // A button to go back to the first tab and scroll the window.
        panel = window->add<Widget>();
        panel->add<Label>("Jump to tab: ");
        panel->setLayout(new BoxLayout(Orientation::Horizontal,
                                       Alignment::Middle, 0, 6));

        auto ib = panel->add<IntBox<int>>();
        ib->setEditable(true);

        b = panel->add<Button>("", ENTYPO_ICON_FORWARD);
        b->setFixedSize(Vector2i(22, 22));
        ib->setFixedHeight(22);
        b->setCallback([tabWidget, ib] {
            int value = ib->value();
            if (value >= 0 && value < tabWidget->tabCount()) {
                tabWidget->setActiveTab(value);
                tabWidget->ensureTabVisible(value);
            }
        });

        window = new Window(this, "Grid of small widgets");
        window->setPosition(Vector2i(425, 300));
        GridLayout *layout =
            new GridLayout(Orientation::Horizontal, 2,
                           Alignment::Middle, 15, 5);
        layout->setColAlignment(
            { Alignment::Maximum, Alignment::Fill });
        layout->setSpacing(0, 10);
        window->setLayout(layout);

        {
            new Label(window, "Floating point :", "sans-bold");
            textBox = new TextBox(window);
            textBox->setEditable(true);
            textBox->setFixedSize(Vector2i(100, 20));
            textBox->setValue("50");
            textBox->setUnits("GiB");
            textBox->setDefaultValue("0.0");
            textBox->setFontSize(16);
            textBox->setFormat("[-]?[0-9]*\\.?[0-9]+");
        }

        {
            new Label(window, "Positive integer :", "sans-bold");
            auto intBox = new IntBox<int>(window);
            intBox->setEditable(true);
            intBox->setFixedSize(Vector2i(100, 20));
            intBox->setValue(50);
            intBox->setUnits("Mhz");
            intBox->setDefaultValue("0");
            intBox->setFontSize(16);
            intBox->setFormat("[1-9][0-9]*");
            intBox->setSpinnable(true);
            intBox->setMinValue(1);
            intBox->setValueIncrement(2);
        }

        {
            new Label(window, "Checkbox :", "sans-bold");

            cb = new CheckBox(window, "Check me");
            cb->setFontSize(16);
            cb->setChecked(true);
        }

        new Label(window, "Combo box :", "sans-bold");
        ComboBox *cobo =
            new ComboBox(window, { "Item 1", "Item 2", "Item 3" });
        cobo->setFontSize(16);
        cobo->setFixedSize(Vector2i(100,20));

        new Label(window, "Color button :", "sans-bold");
        popupBtn = new PopupButton(window, "", 0);
        popupBtn->setBackgroundColor(Color(255, 120, 0, 255));
        popupBtn->setFontSize(16);
        popupBtn->setFixedSize(Vector2i(100, 20));
        popup = popupBtn->popup();
        popup->setLayout(new GroupLayout());

        ColorWheel *colorwheel = new ColorWheel(popup);
        colorwheel->setColor(popupBtn->backgroundColor());

        Button *colorBtn = new Button(popup, "Pick");
        colorBtn->setFixedSize(Vector2i(100, 25));
        Color c = colorwheel->color();
        colorBtn->setBackgroundColor(c);

        colorwheel->setCallback([colorBtn](const Color &value) {
            colorBtn->setBackgroundColor(value);
        });

        colorBtn->setChangeCallback([colorBtn, popupBtn](bool pushed) {
            if (pushed) {
                popupBtn->setBackgroundColor(colorBtn->backgroundColor());
                popupBtn->setPushed(false);
            }
        });

        performLayout();

        /* All NanoGUI widgets are initialized at this point. Now
           create an OpenGL shader to draw the main window contents.

           NanoGUI comes with a simple Eigen-based wrapper around OpenGL 3,
           which eliminates most of the tedious and error-prone shader and
           buffer object management.
        */

        mShader.init(
            /* An identifying name */
            "a_simple_shader",

            /* Vertex shader */
            "#version 330\n"
            "uniform mat4 modelViewProj;\n"
            "in vec3 position;\n"
            "void main() {\n"
            "    gl_Position = modelViewProj * vec4(position, 1.0);\n"
            "}",

            /* Fragment shader */
            "#version 330\n"
            "out vec4 color;\n"
            "uniform float intensity;\n"
            "void main() {\n"
            "    color = vec4(vec3(intensity), 1.0);\n"
            "}"
        );

        MatrixXu indices(3, 2); /* Draw 2 triangles */
        indices.col(0) << 0, 1, 2;
        indices.col(1) << 2, 3, 0;

        MatrixXf positions(3, 4);
        positions.col(0) << -1, -1, 0;
        positions.col(1) <<  1, -1, 0;
        positions.col(2) <<  1,  1, 0;
        positions.col(3) << -1,  1, 0;

        mShader.bind();
        mShader.uploadIndices(indices);
        mShader.uploadAttrib("position", positions);
        mShader.setUniform("intensity", 0.5f);
    }

    ~ExampleApplication() {
        mShader.free();
    }

    virtual bool keyboardEvent(int key, int scancode, int action, int modifiers) {
        if (Screen::keyboardEvent(key, scancode, action, modifiers))
            return true;
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            setVisible(false);
            return true;
        }
        return false;
    }

    virtual void draw(NVGcontext *ctx) {
        /* Animate the scrollbar */
        mProgress->setValue(std::fmod((float) glfwGetTime() / 10, 1.0f));

        /* Draw the user interface */
        Screen::draw(ctx);
    }

    virtual void drawContents() {
        using namespace nanogui;

        /* Draw the window contents using OpenGL */
        mShader.bind();

        Matrix4f mvp;
        mvp.setIdentity();
        mvp.topLeftCorner<3,3>() = Matrix3f(Eigen::AngleAxisf((float) glfwGetTime(),  Vector3f::UnitZ())) * 0.25f;

        mvp.row(0) *= (float) mSize.y() / (float) mSize.x();

        mShader.setUniform("modelViewProj", mvp);

        /* Draw 2 triangles starting at index 0 */
        mShader.drawIndexed(GL_TRIANGLES, 0, 2);
    }
private:
    nanogui::ProgressBar *mProgress;
    nanogui::GLShader mShader;

    using imagesDataType = vector<pair<GLTexture, GLTexture::handleType>>;
    imagesDataType mImagesData;
    int mCurrentImage;
};

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
    try {

        #if !defined(_WIN32)
           /* Avoid locale-related number parsing issues */
           setlocale(LC_NUMERIC, "C");
       #endif

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

        auto mGLFWWindow = glfwCreateWindow(1024, 768, "example1", nullptr, nullptr);

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

        nanogui::init(&constants);

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

        {
            nanogui::ref<ExampleApplication> app = new ExampleApplication(get_pixel_ratio(mGLFWWindow));
            app->drawAll();
            app->setVisible(true);

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

                app->drawAll();

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
    } catch (const std::runtime_error &e) {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        #if defined(_WIN32)
            MessageBoxA(nullptr, error_msg.c_str(), NULL, MB_ICONERROR | MB_OK);
        #else
            std::cerr << error_msg << endl;
        #endif
    }

    glfwTerminate();

    return 0;
}
