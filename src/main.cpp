#include <Windows.h>
#include <iostream>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "core/D3DContext.h"
#include "core/CoreCommon.h"

LRESULT CALLBACK MainWndProc(HWND hwnd, uint32_t msg, WPARAM wParam, LPARAM lParam);

HWND g_hwnd = nullptr;
uint32_t g_currentBuffer = 0;

void loadScene()
{
    Assimp::Importer importer;

    const std::string path = "../../assets/LP1.obj";
    const auto scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_MakeLeftHanded);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        return;
    }

    for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
    {
        auto mesh = scene->mMeshes[i];

        std::cout << mesh->mNumVertices << std::endl;
    }
}

class App
{
private:
    static App *instance;

private:
    HINSTANCE m_HInstance = nullptr;
    const wchar_t *m_WinName = L"D3DWindow";

public:
    App(HINSTANCE hInstance) : m_HInstance(hInstance)
    {
    }

    LRESULT MsgProc(HWND hwnd, uint32_t msg, WPARAM wParam, LPARAM lParam);

    bool InitWindow();

    int Run(D3DContext *);

    static App *GetApp();

    static void SetApp(App *);
};

App *App::instance = nullptr;

App *App::GetApp() { return instance; }

void App::SetApp(App *pApp) { App::instance = pApp; }

LRESULT App::MsgProc(HWND hwnd, uint32_t msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_ACTIVATE:
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

bool App::InitWindow()
{
    WNDCLASS wc{};
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWndProc;
    wc.hInstance = m_HInstance;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
    wc.lpszClassName = m_WinName;

    if (!RegisterClass(&wc))
    {
        MessageBox(0, L"Register Class failed!", 0, 0);
        return false;
    }

    RECT rect = {0, 0, g_displayWidth, g_displayHeight};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
    uint32_t width = rect.right - rect.left;
    uint32_t height = rect.bottom - rect.top;

    g_hwnd = CreateWindow(m_WinName, L"some data", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height,
                          0, 0, m_HInstance, 0);

    if (!g_hwnd)
    {
        MessageBox(0, L"Create Window is not OK!", 0, 0);
        return false;
    }

    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);

    return true;
}

int App::Run(D3DContext *context)
{

    MSG msg = {0};

    while (true)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
            {
                break;
            }
        }
        else
        {
            context->Update();
            context->Render();
        }
    }

    return (int)msg.wParam;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, uint32_t msg, WPARAM wParam, LPARAM lParam)
{
    return App::GetApp()->MsgProc(hwnd, msg, wParam, lParam);
}

int CALLBACK WinMain(
    HINSTANCE hinstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdArg)
{
    App::SetApp(new App(hinstance));
    D3DContext::setContext(new D3DContext());

    if (!App::GetApp()->InitWindow())
    {
        return EXIT_FAILURE;
    }

    // loadScene();

    D3DContext::getContext()->Init();

    return App::GetApp()->Run(D3DContext::getContext());
}