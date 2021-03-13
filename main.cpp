#include <GL/glew.h>
#include <GL/freeglut.h>
#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl3.h"
#include "Application.h"

//Remove console (only works in Visual Studio)
#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")

#define TIME_PER_FRAME 1000.f/60.f // Approx. 60 fps

static int prevTime;
static Application app; // This object represents our whole app

// If a key is pressed this callback is called

static void keyboardDownCallback(unsigned char key, int x, int y)
{
    ImGui_ImplGLUT_KeyboardFunc(key, x, y);
    if (ImGui::GetIO().WantCaptureKeyboard)
        return;

    Application::instance().keyPressed(key);
}

// If a key is released this callback is called

static void keyboardUpCallback(unsigned char key, int x, int y)
{
    ImGui_ImplGLUT_KeyboardUpFunc(key, x, y);
    if (ImGui::GetIO().WantCaptureKeyboard)
        return;

    Application::instance().keyReleased(key);
}

// If a special key is pressed this callback is called

static void specialDownCallback(int key, int x, int y)
{
    ImGui_ImplGLUT_SpecialFunc(key, x, y);
    if (ImGui::GetIO().WantCaptureKeyboard)
        return;

    Application::instance().specialKeyPressed(key);
}

// If a special key is released this callback is called

static void specialUpCallback(int key, int x, int y)
{
    ImGui_ImplGLUT_SpecialFunc(key, x, y);
    if (ImGui::GetIO().WantCaptureKeyboard)
        return;

    Application::instance().specialKeyReleased(key);
}

// Same for changes in mouse cursor position

static void motionCallback(int x, int y)
{
    ImGui_ImplGLUT_MotionFunc(x, y);
    if (ImGui::GetIO().WantCaptureMouse)
        return;

    Application::instance().mouseMove(x, y);
}

// Same for mouse button presses or releases

static void mouseCallback(int button, int state, int x, int y)
{
    ImGui_ImplGLUT_MouseFunc(button, state, x, y);
    if (ImGui::GetIO().WantCaptureMouse)
        return;

    int buttonId;

    // FIXME: This is causing a bug when scrolling the mouse wheel: add default case to the switch
    switch (button)
    {
    case GLUT_LEFT_BUTTON:
        buttonId = 0;
        break;
    case GLUT_RIGHT_BUTTON:
        buttonId = 1;
        break;
    case GLUT_MIDDLE_BUTTON:
        buttonId = 2;
        break;
    }

    if (state == GLUT_DOWN)
        Application::instance().mousePress(buttonId);
    else if (state == GLUT_UP)
        Application::instance().mouseRelease(buttonId);
}

// TODO: Should application handle these too?
static void mouseWheelCallback(int button, int dir, int x, int y)
{
    ImGui_ImplGLUT_MouseWheelFunc(button, dir, x, y);
}

// Resizing the window calls this function

static void resizeCallback(int width, int height)
{
    ImGui_ImplGLUT_ReshapeFunc(width, height);
    Application::instance().resize(width, height);
}

static void drawCallback()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGLUT_NewFrame();

    // Render the application frame (including calls to Dear ImGui)
    Application::instance().render();

    // Render the Dear ImGui frame (with the calls to Dear ImGui that the applcation has made)
    ImGui::Render();
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glutSwapBuffers();
}

static void idleCallback()
{
    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    int deltaTime = currentTime - prevTime;

    if (deltaTime > TIME_PER_FRAME)
    {
        // Every time we enter here is equivalent to a game loop execution
        if (!Application::instance().update(deltaTime))
            exit(0);
        prevTime = currentTime;
        glutPostRedisplay();
    }
}

int main(int argc, char **argv)
{
    // GLUT initialization
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(640, 480);

    glutCreateWindow(argv[0]);
    glutReshapeFunc(resizeCallback);
    glutDisplayFunc(drawCallback);
    glutIdleFunc(idleCallback);
    glutKeyboardFunc(keyboardDownCallback);
    glutKeyboardUpFunc(keyboardUpCallback);
    glutSpecialFunc(specialDownCallback);
    glutSpecialUpFunc(specialUpCallback);
    glutMouseFunc(mouseCallback);
    glutMouseWheelFunc(mouseWheelCallback);
    glutMotionFunc(motionCallback);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGLUT_Init();
    ImGui_ImplOpenGL3_Init();

    // GLEW will take care of OpenGL extension functions
    glewExperimental = GL_TRUE;
    glewInit();

    // Application instance initialization
    Application::instance().init();
    if (argc > 1)
        Application::instance().loadMesh(argv[1]);
    prevTime = glutGet(GLUT_ELAPSED_TIME);
    // GLUT gains control of the application
    glutMainLoop();

    // Dear ImGui cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGLUT_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
