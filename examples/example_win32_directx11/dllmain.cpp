

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include <MinHook.h>
#include <chrono>
#include <algorithm>
#include <functional>
#include <vector>
#include <random>
#include <math.h>
#include <dwmapi.h>
#include <iostream>
#include <ctime>
#include <string>
#define GLEW_STATIC
#include <glew.h>
#include "Chams/Texture.h"
#include <shobjidl.h>
#include <tlhelp32.h>
#pragma comment(lib, "Dwmapi.lib")

#define IM_COLOR(r, g, b, a) ImVec4((r)/255.0f, (g)/255.0f, (b)/255.0f, (a)/255.0f)


GLfloat blendColor[4] = { 0, 255, 0, 1 };

struct Color {
    int r, g, b, a;
};

struct ShaderInfo {
    const char* vertexShaderPath;
    const char* fragmentShaderPath;
};

struct AdditionalEffectInfo {
    GLuint vertexArrayObject;  
    GLuint shaderProgram;      
};


void (WINAPI* oglDrawElements)(GLenum mode, GLsizei count, GLenum type, const void* indices);

GLuint gTexCyan;
GLuint gTexYellow;

std::vector<const char*> textureNames = {
    "hlslcc_mtx4x4unity_ObjectToWorld[0]",
    "hlslcc_mtx4x4unity_MatrixVP[0]",
    "_MainTex",
    "_MainTex_ST",
    "_AlphaTex"
    "_Color",
    "_WeatherDarkness",
    "glstate_lightmodel_ambient"
    "_Cutoff",
    "unity_LightmapST",
    "unity_Lightmap_HDR",
    "unity_Lightmap",
    "_CharaLightIntensity"
    "_Ambientlight",
    "_AlphaMask",
    "_StretchRatio"
    "_WorldSpaceCameraPos"
};

std::unordered_map<std::string, ChamsInfo> gChamsDescs{
    {"_CharaLightIntensity", {nullptr}}   
};

std::atomic_bool gbRunning = true;

ShaderInfo additionalEffectShaderInfo = { "particleVertexShader.glsl", "particleFragmentShader.glsl" };

AdditionalEffectInfo additionalEffectInfo;

bool CurrentShaderHasUniform(const std::string& uniform)
{
    GLint currProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);

    GLint id = glGetUniformLocation(currProgram, uniform.c_str());

    return id != -1;
}


void drawAdditionalEffect()
{

    glUseProgram(additionalEffectInfo.shaderProgram);

  
    glBindVertexArray(additionalEffectInfo.vertexArrayObject);
    glDrawArrays(GL_POINTS, 2, 100);  
    glBindVertexArray(0);


    glUseProgram(0);
}

#define _DRAW_RGB_ 29,0,28
#define GL_PI 3.14159


float width_head = 1.0f;
float height_head = 1.0f;

float rectangleX_head = 0.0f;
float rectangleY_head = 0.0f;
float rectangleZ_head = 1.3f;
float rotationX_head = 90.0f;
float rotationY_head = 90.0f;
void RotatePoint(float& x, float& y, float angleDegrees) {
    float angleRadians = angleDegrees * (3.14159265359f / 180.0f);
    float tempX = x * cos(angleRadians) - y * sin(angleRadians);
    float tempY = x * sin(angleRadians) + y * cos(angleRadians);
    x = tempX;
    y = tempY;
}

static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

ImVec2 menu_size = ImVec2(400, 350);

static bool dragging = false;
static POINT drag_start_mouse = { 0 };
static RECT drag_start_window = { 0 };

static bool show_main_window = true;

bool moco = false;
bool transparent = false;
bool bones = false;
bool box3d = false;
bool glowhack = false;
bool rgbchams = false;

static float thickness_bones = 1.5f;
static float thickness_box = 1.0f;
static float rgb_time = 1.0f;

static float frequencyM = 0.600;
static float lineWidthT = 1.0;

static float width = 0.923f;
static float height = 0.523f;

float rectangleX = 0.0f;
float rectangleY = 0.3378f;
float rectangleZ = 0.73f;
float rotationX = 90.0f;
float rotationY = 90.0f;
float rotationX2 = 0.0f;
float rotationY2 = 90.0f;
float rectangleX2 = 0.3378f;
float rectangleY2 = 0.0f;
float rectangleZ2 = 0.73f;
float rotationX3 = 90.0f;
float rotationY3 = 90.0f;
float rectangleX3 = 0.0f;
float rectangleY3 = -0.3378f;
float rectangleZ3 = 0.73f;
float rotationX4 = 0.0f;
float rotationY4 = 90.0f;
float rectangleX4 = -0.3378f;
float rectangleY4 = 0.0f;
float rectangleZ4 = 0.73f;


HWND hwnd;
RECT rc;

double getCurrentTime() {
    auto now = std::chrono::high_resolution_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count() / 1000.0;
}

bool ChamsContextInitialize()
{

    {
        GLTexture2DBindRestore guard(0);
        ColorToTexture({ 0, 255, 0, 1 }, &gTexCyan);      
        ColorToTexture({ 0, 0, 255, 1 }, &gTexYellow);    

    }

    return true;
}

void DrawRectangle() {
    float halfWidth = width / 1.38f;
    float halfHeight = height / 1.5f;

    float x1 = -halfWidth;
    float y1 = -halfHeight;
    float z1 = 0.0f;

    float x2 = halfWidth;
    float y2 = -halfHeight;
    float z2 = 0.0f;

    float x3 = halfWidth;
    float y3 = halfHeight;
    float z3 = 0.0f;

    float x4 = -halfWidth;
    float y4 = halfHeight;
    float z4 = 0.0f;

    RotatePoint(y1, z1, rotationX);
    RotatePoint(y2, z2, rotationX);
    RotatePoint(y3, z3, rotationX);
    RotatePoint(y4, z4, rotationX);

    RotatePoint(x1, z1, rotationY);
    RotatePoint(x2, z2, rotationY);
    RotatePoint(x3, z3, rotationY);
    RotatePoint(x4, z4, rotationY);

    x1 += rectangleX;
    y1 += rectangleY;
    z1 += rectangleZ;

    x2 += rectangleX;
    y2 += rectangleY;
    z2 += rectangleZ;

    x3 += rectangleX;
    y3 += rectangleY;
    z3 += rectangleZ;

    x4 += rectangleX;
    y4 += rectangleY;
    z4 += rectangleZ;

    glLineWidth(thickness_box); 
    glBegin(GL_LINE_LOOP);
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y2, z2);
    glVertex3f(x3, y3, z3);
    glVertex3f(x4, y4, z4);
    glEnd();
    glLineWidth(1.0f); 
}

void DrawRectangle2() {
    float halfWidth = width / 1.38f;
    float halfHeight = height / 1.5f;

    float x1 = -halfWidth;
    float y1 = -halfHeight;
    float z1 = 0.0f;

    float x2 = halfWidth;
    float y2 = -halfHeight;
    float z2 = 0.0f;

    float x3 = halfWidth;
    float y3 = halfHeight;
    float z3 = 0.0f;

    float x4 = -halfWidth;
    float y4 = halfHeight;
    float z4 = 0.0f;

    RotatePoint(y1, z1, rotationX2);
    RotatePoint(y2, z2, rotationX2);
    RotatePoint(y3, z3, rotationX2);
    RotatePoint(y4, z4, rotationX2);

    RotatePoint(x1, z1, rotationY2);
    RotatePoint(x2, z2, rotationY2);
    RotatePoint(x3, z3, rotationY2);
    RotatePoint(x4, z4, rotationY2);

    x1 += rectangleX2;
    y1 += rectangleY2;
    z1 += rectangleZ2;

    x2 += rectangleX2;
    y2 += rectangleY2;
    z2 += rectangleZ2;

    x3 += rectangleX2;
    y3 += rectangleY2;
    z3 += rectangleZ2;

    x4 += rectangleX2;
    y4 += rectangleY2;
    z4 += rectangleZ2;

    glLineWidth(thickness_box);
    glBegin(GL_LINE_LOOP);
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y2, z2);
    glVertex3f(x3, y3, z3);
    glVertex3f(x4, y4, z4);
    glEnd();
    glLineWidth(1.0f); 
}

void DrawRectangle3() {
    float halfWidth = width / 1.38f;
    float halfHeight = height / 1.5f;

    float x1 = -halfWidth;
    float y1 = -halfHeight;
    float z1 = 0.0f;

    float x2 = halfWidth;
    float y2 = -halfHeight;
    float z2 = 0.0f;

    float x3 = halfWidth;
    float y3 = halfHeight;
    float z3 = 0.0f;

    float x4 = -halfWidth;
    float y4 = halfHeight;
    float z4 = 0.0f;

    RotatePoint(y1, z1, rotationX3);
    RotatePoint(y2, z2, rotationX3);
    RotatePoint(y3, z3, rotationX3);
    RotatePoint(y4, z4, rotationX3);

    RotatePoint(x1, z1, rotationY3);
    RotatePoint(x2, z2, rotationY3);
    RotatePoint(x3, z3, rotationY3);
    RotatePoint(x4, z4, rotationY3);

    x1 += rectangleX3;
    y1 += rectangleY3;
    z1 += rectangleZ3;

    x2 += rectangleX3;
    y2 += rectangleY3;
    z2 += rectangleZ3;

    x3 += rectangleX3;
    y3 += rectangleY3;
    z3 += rectangleZ3;

    x4 += rectangleX3;
    y4 += rectangleY3;
    z4 += rectangleZ3;

    glLineWidth(thickness_box);
    glBegin(GL_LINE_LOOP);
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y2, z2);
    glVertex3f(x3, y3, z3);
    glVertex3f(x4, y4, z4);
    glEnd();
    glLineWidth(1.0f);
}

void DrawRectangle4() {
    float halfWidth = width / 1.38f;
    float halfHeight = height / 1.5f;

    float x1 = -halfWidth;
    float y1 = -halfHeight;
    float z1 = 0.0f;

    float x2 = halfWidth;
    float y2 = -halfHeight;
    float z2 = 0.0f;

    float x3 = halfWidth;
    float y3 = halfHeight;
    float z3 = 0.0f;

    float x4 = -halfWidth;
    float y4 = halfHeight;
    float z4 = 0.0f;

    RotatePoint(y1, z1, rotationX4);
    RotatePoint(y2, z2, rotationX4);
    RotatePoint(y3, z3, rotationX4);
    RotatePoint(y4, z4, rotationX4);

    RotatePoint(x1, z1, rotationY4);
    RotatePoint(x2, z2, rotationY4);
    RotatePoint(x3, z3, rotationY4);
    RotatePoint(x4, z4, rotationY4);

    x1 += rectangleX4;
    y1 += rectangleY4;
    z1 += rectangleZ4;

    x2 += rectangleX4;
    y2 += rectangleY4;
    z2 += rectangleZ4;

    x3 += rectangleX4;
    y3 += rectangleY4;
    z3 += rectangleZ4;

    x4 += rectangleX4;
    y4 += rectangleY4;
    z4 += rectangleZ4;

    glLineWidth(thickness_box);
    glBegin(GL_LINE_LOOP);
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y2, z2);
    glVertex3f(x3, y3, z3);
    glVertex3f(x4, y4, z4);
    glEnd();
    glLineWidth(1.0f);  
}

void Moco()
{
    float radio = 0.2f;
    for (float i = 0.f; i <= 2.0f * 3.1416; i += 0.1f)
    {
        float x = radio * cos(i);
        float y = radio * sin(i);
        glLineWidth(1.f);
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 1.4f);
        glVertex3f(x, y, 1.8f);
        glEnd();

    }
}

void RotatePointAAA(float& x, float& y, float& z, float rotationX, float rotationY, float rotationZ) {
    float radianX = rotationX * 3.14159265f / 180.0f;
    float radianY = rotationY * 3.14159265f / 180.0f;
    float radianZ = rotationZ * 3.14159265f / 180.0f;

    float tempY = y;
    y = cos(radianX) * y - sin(radianX) * z;
    z = sin(radianX) * tempY + cos(radianX) * z;

    float tempX = x;
    x = cos(radianY) * x + sin(radianY) * z;
    z = -sin(radianY) * tempX + cos(radianY) * z;

    tempX = x;
    x = cos(radianZ) * x - sin(radianZ) * y;
    y = sin(radianZ) * tempX + cos(radianZ) * y;
}


void DrawBOX1() {
    float halfWidth = width_head / 9.0;
    float halfHeight = height_head / 9.0f;

    float x1 = -halfWidth;
    float y1 = -halfHeight;
    float z1 = 0.0f;

    float x2 = halfWidth;
    float y2 = -halfHeight;
    float z2 = 0.0f;

    float x3 = halfWidth;
    float y3 = halfHeight;
    float z3 = 0.0f;

    float x4 = -halfWidth;
    float y4 = halfHeight;
    float z4 = 0.0f;

    RotatePoint(y1, z1, rotationX_head);
    RotatePoint(y2, z2, rotationX_head);
    RotatePoint(y3, z3, rotationX_head);
    RotatePoint(y4, z4, rotationX_head);

    RotatePoint(x1, z1, rotationY_head);
    RotatePoint(x2, z2, rotationY_head);
    RotatePoint(x3, z3, rotationY_head);
    RotatePoint(x4, z4, rotationY_head);

    x1 += rectangleX_head;
    y1 += rectangleY_head;
    z1 += rectangleZ_head;

    x2 += rectangleX_head;
    y2 += rectangleY_head;
    z2 += rectangleZ_head;

    x3 += rectangleX_head;
    y3 += rectangleY_head;
    z3 += rectangleZ_head;

    x4 += rectangleX_head;
    y4 += rectangleY_head;
    z4 += rectangleZ_head;

    glLineWidth(thickness_bones);
    glBegin(GL_LINE_LOOP);
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y2, z2);
    glVertex3f(x3, y3, z3);
    glVertex3f(x4, y4, z4);
    glEnd();
    glLineWidth(1.0f);
}

void DrawLine() {
    float width_line = 0.6f;      
    float height_line = 0.6f;          

    float rectangleX_line = 0.0f;          
    float rectangleY_line = 0.0f;          
    float rectangleZ_line = 0.9f;          

    float rotationX_line = 180.0f;           
    float rotationY_line = 90.0f;           
    float rotationZ_line = 270.0f;           

    float halfWidth_line = width_line / 2.0f;

    float x1_line = -halfWidth_line;
    float y1_line = 0.0f;
    float z1_line = 0.0f;

    float x2_line = halfWidth_line;
    float y2_line = 0.0f;
    float z2_line = 0.0f;

    RotatePoint(y1_line, z1_line, rotationX_line);
    RotatePoint(y2_line, z2_line, rotationX_line);
    RotatePoint(x1_line, z1_line, rotationY_line);
    RotatePoint(x2_line, z2_line, rotationY_line);

    x1_line += rectangleX_line;
    y1_line += rectangleY_line;
    z1_line += rectangleZ_line;

    x2_line += rectangleX_line;
    y2_line += rectangleY_line;
    z2_line += rectangleZ_line;

    glLineWidth(thickness_bones);

    glBegin(GL_LINES);
    glVertex3f(x1_line, y1_line, z1_line);
    glVertex3f(x2_line, y2_line, z2_line);
    glEnd();

    glLineWidth(1.0f);
}

void DrawSlashLine() {
    float x1 = -0.5f;        
    float y1 = -0.5f;        
    float z1 = 0.0f;         

    float x2 = 0.5f;         
    float y2 = 0.5f;         
    float z2 = 0.0f;         

    float posX = 0.1f;        
    float posY = 0.0f;        
    float posZ = 0.4f;        

    float scaleX = 0.4f;      
    float scaleY = 0.4f;      
    float scaleZ = 0.4f;      

    float rotationX = 90.0f;           
    float rotationY = 110.0f;           
    float rotationZ = 0.0f;          

    x1 *= scaleX;
    y1 *= scaleY;
    z1 *= scaleZ;
    x2 *= scaleX;
    y2 *= scaleY;
    z2 *= scaleZ;

    RotatePointAAA(x1, y1, z1, rotationX, rotationY, rotationZ);
    RotatePointAAA(x2, y2, z2, rotationX, rotationY, rotationZ);

    x1 += posX;
    y1 += posY;
    z1 += posZ;
    x2 += posX;
    y2 += posY;
    z2 += posZ;

    glLineWidth(thickness_bones);

    glBegin(GL_LINES);
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y2, z2);
    glEnd();

    glLineWidth(1.0f);
}

void DrawSlashLine2() {
    float x1 = -0.5f;        
    float y1 = -0.5f;        
    float z1 = 0.0f;         

    float x2 = 0.5f;         
    float y2 = 0.5f;         
    float z2 = 0.0f;         

    float posX = -0.1f;        
    float posY = 0.0f;        
    float posZ = 0.4f;        

    float scaleX = 0.4f;      
    float scaleY = 0.4f;      
    float scaleZ = 0.4f;      

    float rotationX = -90.0f;           
    float rotationY = 70.0f;           
    float rotationZ = 0.0f;          

    x1 *= scaleX;
    y1 *= scaleY;
    z1 *= scaleZ;
    x2 *= scaleX;
    y2 *= scaleY;
    z2 *= scaleZ;

    RotatePointAAA(x1, y1, z1, rotationX, rotationY, rotationZ);
    RotatePointAAA(x2, y2, z2, rotationX, rotationY, rotationZ);

    x1 += posX;
    y1 += posY;
    z1 += posZ;
    x2 += posX;
    y2 += posY;
    z2 += posZ;

    glLineWidth(thickness_bones);

    glBegin(GL_LINES);
    glVertex3f(x1, y1, z1);
    glVertex3f(x2, y2, z2);
    glEnd();

    glLineWidth(1.0f);
}


void DrawVisible(GLenum mode, GLsizei count, GLenum type, const void* indices, const ChamsInfo& transparentDesc)
{
    if (transparent == true)
    {
        glBlendColor(0, 0, 0, 1);
        glColorMask(1, 1, 1, 1);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_CONSTANT_COLOR);
        glDisable(GL_DEPTH_TEST);
        oglDrawElements(GL_TRIANGLES, count, type, indices);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
        glBlendColor(0, 0, 0, 1);
    }
    else
    {

    }

    if (moco == true)
    {
        glDepthRange(1, 0.5);
        glEnable(GL_BLEND);
        glColorMask(1, 1, 1, 1);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glBlendFuncSeparate(GL_CONSTANT_COLOR, GL_CONSTANT_ALPHA, GL_ONE, GL_ZERO);

        glBlendColor(blendColor[0], blendColor[1], blendColor[2], blendColor[3]);

        glPushMatrix();

        Moco();

        glPopMatrix();
        glDepthRange(0.5, 1);
        glDisable(GL_BLEND);
    }
    else
    {
        oglDrawElements(mode, count, type, indices);
    }

    if (bones == true)
    {
        glDepthRange(1, 0.5);
        glEnable(GL_BLEND);
        glColorMask(1, 1, 1, 1);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glBlendFuncSeparate(GL_CONSTANT_COLOR, GL_CONSTANT_ALPHA, GL_ONE, GL_ZERO);
        glBlendColor(blendColor[0], blendColor[1], blendColor[2], blendColor[3]);

        glPushMatrix();

        DrawBOX1();
        DrawLine();
        DrawSlashLine();
        DrawSlashLine2();

        glPopMatrix();
        glDepthRange(0.5, 1);
        glDisable(GL_BLEND);
    }
    else
    {
        oglDrawElements(mode, count, type, indices);
    }

    if (glowhack == true)
    {
        glDisable(GL_DEPTH_TEST);


        oglDrawElements(mode, count, type, indices);

        glEnable(GL_DEPTH_TEST);
    }
    else
    {

    }

    if (box3d == true)
    {
        glDepthRange(1, 0.5);
        glEnable(GL_BLEND);
        glColorMask(1, 1, 1, 1);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glBlendFuncSeparate(GL_CONSTANT_COLOR, GL_CONSTANT_ALPHA, GL_ONE, GL_ZERO);

        glBlendColor(blendColor[0], blendColor[1], blendColor[2], blendColor[3]);

        glPushMatrix();

        DrawRectangle();
        DrawRectangle2();
        DrawRectangle3();
        DrawRectangle4();

        glPopMatrix();
        glDepthRange(0.5, 1);
        glDisable(GL_BLEND);
    }
    else
    {

    }

    if (rgbchams == true)
    {
        glDisable(GL_DEPTH_TEST);


        glEnable(GL_DEPTH_TEST);
    }
    else
    {

    }

}

void DrawAlwaysTop(GLenum mode, GLsizei count, GLenum type, const void* indices, const ChamsInfo& transparentDesc)
{
    if (transparent == true)
    {
        glDisable(GL_DEPTH_TEST);


        oglDrawElements(mode, count, type, indices);

        glEnable(GL_DEPTH_TEST);
    }
    else
    {

    }

    if (moco == true)
    {
        glDisable(GL_DEPTH_TEST);


        oglDrawElements(mode, count, type, indices);

        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        oglDrawElements(mode, count, type, indices);
    }

    if (glowhack == true)
    {
        double time = getCurrentTime();
        float lineWidthM = lineWidthT + 8.0f * std::abs(std::sin(2.0 * 3.14159265358979323846 * frequencyM * time));
        GLfloat originalDepthRange[2];
        glGetFloatv(GL_DEPTH_RANGE, originalDepthRange);
        glDepthRange(1, 0.5);


        oglDrawElements(mode, count, type, indices);

        glDepthRange(0.5, 1);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(lineWidthM);
        glEnable(GL_BLEND);
        glBlendColor(blendColor[0], blendColor[1], blendColor[2], blendColor[3]);
        glBlendFunc(GL_ONE, GL_CONSTANT_COLOR);
        glDepthRange(1, 0.8);
        oglDrawElements(mode, count, type, indices);
        glDepthRange(originalDepthRange[0], originalDepthRange[1]);
        glDisable(GL_BLEND);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    else
    {

    }

    if (box3d == true)
    {
        glDisable(GL_DEPTH_TEST);


        oglDrawElements(mode, count, type, indices);

        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        oglDrawElements(mode, count, type, indices);
    }
    
    if (rgbchams == true)
    {
        double time = getCurrentTime();
        float lineWidthM = 4.0f + 8.0f * std::abs(std::sin(2.0 * 3.14159265358979323846 * rgb_time * time));

        glDepthRange(1, 0.5);
        glLineWidth(lineWidthM);
        glEnable(GL_BLEND);
        glColorMask(1, 1, 1, 1);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glBlendFuncSeparate(GL_CONSTANT_COLOR, GL_CONSTANT_ALPHA, GL_ONE, GL_ZERO);
        glBlendColor(0, 0, 0, 1);
        glDrawElements(GL_TRIANGLES_ADJACENCY, count, type, indices);

        float red = 0.5f * (1.0f + std::sin(2.0 * 3.14159265358979323846 * rgb_time * time));
        float green = 0.5f * (1.0f + std::sin(2.0 * 3.14159265358979323846 * rgb_time * time + 2.0 * 3.14159265358979323846 / 3.0));
        float blue = 0.5f * (1.0f + std::sin(2.0 * 3.14159265358979323846 * rgb_time * time + 4.0 * 3.14159265358979323846 / 3.0));

        glBlendColor(red, green, blue, 1);
        glDepthRangef(1, 0.5);
        glDrawElements(GL_LINES, count, type, indices);

        oglDrawElements(mode, count, type, indices);

        glDepthRange(0.5, 1);
        glDrawElements(GL_LINE_LOOP, count, type, indices);
        glDisable(GL_BLEND);
    }
    else
    {

    }
}


void move_window()
{
    static POINT click_offset = { 0 };
    static bool dragging = false; 

    if (!ImGui::IsAnyItemHovered() && GetAsyncKeyState(VK_LBUTTON)) {
        ImVec2 mouse_pos = ImGui::GetIO().MousePos;
        ImVec2 win_pos = ImGui::GetMainViewport()->Pos;

        if (!dragging && ImGui::IsMouseClicked(0)) {
            POINT p;
            GetCursorPos(&p);
            RECT win;
            GetWindowRect(hwnd, &win);
            click_offset.x = p.x - win.left;
            click_offset.y = p.y - win.top;
            dragging = true;
        }

        if (dragging) {
            POINT p;
            GetCursorPos(&p);
            int new_x = p.x - click_offset.x;
            int new_y = p.y - click_offset.y;

            SetWindowPos(hwnd, NULL, new_x, new_y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
        }
    }
    else {

        dragging = false;
    }
}

void WINAPI hglDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices) {

    static bool bGlewInitialized = false;

    if (!bGlewInitialized)
    {
        HGLRC ctx = wglGetCurrentContext();
        HDC dc = wglGetCurrentDC();

        if (ctx && dc)
        {
            GLenum err = glewInit();
            if (err != GLEW_OK)
            {
                printf("GLEW Init failed: %s\n", glewGetErrorString(err));
                MH_DisableHook(glDrawElements);
                gbRunning = false;
                return;
            }

            bGlewInitialized = true;
            printf("GLEW initialized successfully.\n");
        }
        else
        {
            return oglDrawElements(mode, count, type, indices);
        }
    }

    if (GetAsyncKeyState(VK_F20) & 1)
    {
        oglDrawElements(mode, count, type, indices);

        MH_DisableHook(glDrawElements);
        gbRunning = false;

        return;

    }

    if (mode != GL_TRIANGLES || count < 900)
    {
        return oglDrawElements(mode, count, type, indices);
    }

    const std::pair<const std::string, ChamsInfo>* pChamDescKv = nullptr;

    for (const auto& chamDescKv : gChamsDescs)
    {
        if (!CurrentShaderHasUniform(chamDescKv.first)) continue;

        pChamDescKv = &chamDescKv;
        break;
    }

    if (pChamDescKv == nullptr)
    {

        return oglDrawElements(mode, count, type, indices);
    }
    static bool gbChamsCtxInitialized = false;

    if (!gbChamsCtxInitialized &&
        !(gbChamsCtxInitialized = ChamsContextInitialize()))
    {
        oglDrawElements(mode, count, type, indices);

        MH_DisableHook(glDrawElements);
        gbRunning = false;

        return;
    }

    bool didDraw = false;

    //Add or edit bools from here.
    bool useChamsEffects = transparent || glowhack || rgbchams || moco || box3d || bones;

    if (useChamsEffects)
    {
        DrawAlwaysTop(mode, count, type, indices, pChamDescKv->second);
        DrawVisible(mode, count, type, indices, pChamDescKv->second);
        didDraw = true;
    }

    if (!didDraw)
    {
        oglDrawElements(mode, count, type, indices);
    }

}

void Main()
{
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, L"Free Chams Menu | Legit Hax", L"Free Chams Menu | Legit Hax", nullptr };
    ::RegisterClassExW(&wc);

    hwnd = CreateWindowEx(WS_EX_LAYERED,
        "Free Chams Menu | Legit Hax", "Free Chams Menu | Legit Hax",
        WS_POPUP,
        100, 100,
        (int)menu_size.x, (int)menu_size.y,
        NULL, NULL, wc.hInstance, NULL);


    SetWindowLongA(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);

    MARGINS margins = { -1 };
    DwmExtendFrameIntoClientArea(hwnd, &margins);


    POINT mouse;
    rc = { 0 };
    GetWindowRect(hwnd, &rc);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;        
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;         

    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    ImGuiStyle& style = ImGui::GetStyle();

    style.WindowRounding = 0.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 3.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 4.0f;

    style.FramePadding = ImVec2(6, 2);
    style.ItemSpacing = ImVec2(8, 10);
    style.WindowPadding = ImVec2(8, 8);

    style.Colors[ImGuiCol_WindowBg] = IM_COLOR(26, 26, 26, 255);
    style.Colors[ImGuiCol_ChildBg] = IM_COLOR(20, 20, 20, 255);
    style.Colors[ImGuiCol_FrameBgHovered] = IM_COLOR(19, 19, 19, 255);
    style.Colors[ImGuiCol_FrameBg] = IM_COLOR(19, 19, 19, 255);
    style.Colors[ImGuiCol_FrameBgActive] = IM_COLOR(19, 19, 19, 255);
    style.Colors[ImGuiCol_TitleBg] = IM_COLOR(15, 15, 15, 255);
    style.Colors[ImGuiCol_TitleBgActive] = IM_COLOR(15, 15, 15, 255);
    style.Colors[ImGuiCol_TitleBgCollapsed] = IM_COLOR(15, 15, 15, 255);
    style.Colors[ImGuiCol_Header] = IM_COLOR(45, 45, 45, 255);
    style.Colors[ImGuiCol_HeaderHovered] = IM_COLOR(65, 65, 65, 255);
    style.Colors[ImGuiCol_HeaderActive] = IM_COLOR(75, 75, 75, 255);
    style.Colors[ImGuiCol_Button] = IM_COLOR(36, 36, 36, 255);
    style.Colors[ImGuiCol_ButtonHovered] = IM_COLOR(56, 56, 56, 255);
    style.Colors[ImGuiCol_ButtonActive] = IM_COLOR(76, 76, 76, 255);
    style.Colors[ImGuiCol_SliderGrab] = IM_COLOR(56, 56, 56, 255);    
    style.Colors[ImGuiCol_SliderGrabActive] = IM_COLOR(56, 56, 56, 255);     
    style.Colors[ImGuiCol_CheckMark] = IM_COLOR(255, 255, 255, 255);  
    style.Colors[ImGuiCol_Border] = IM_COLOR(50, 50, 50, 255);
    style.Colors[ImGuiCol_BorderShadow] = IM_COLOR(0, 0, 0, 0);


    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
        g_SwapChainOccluded = false;

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0,0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(menu_size, ImGuiCond_FirstUseEver);

        {
            ImGui::Begin("Free Chams Menu | Legit Hax", &show_main_window, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

            move_window();

            if (ImGui::CollapsingHeader("Visuals"))
            {
                ImGui::Checkbox("Transparent", &transparent); 
                ImGui::Checkbox("Moco", &moco);
                ImGui::Checkbox("Bones", &bones);
                ImGui::Checkbox("3D Box", &box3d); 
                ImGui::Checkbox("Glow Hack", &glowhack); 
                ImGui::Checkbox("RGB", &rgbchams); 
            }

            if (ImGui::CollapsingHeader("Config"))
            {
                ImGui::PushItemWidth(155);
                ImGui::SliderFloat("Thickness Bones", &thickness_bones, 1.5f, 6.5f);
                ImGui::SliderFloat("Thickness Box", &thickness_box, 1.0f, 6.5f);
                ImGui::SliderFloat("RGB Time", &rgb_time, 0.1f, 4.5f);
                ImGui::PopItemWidth();
                
                ImGui::ColorEdit4("Color Edit", reinterpret_cast<float*>(blendColor));
            }

            ImGui::End();
            if (!show_main_window)
                done = true;
        }

        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        HRESULT hr = g_pSwapChain->Present(1, 0);      
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return;
}

bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = (UINT)menu_size.x;
    sd.BufferDesc.Height = (UINT)menu_size.y;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)           
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam);   
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)     
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

bool Run()
{
    if (MH_Initialize() != MH_OK)
        return false;

    MH_CreateHook((LPVOID)glDrawElements, hglDrawElements, (LPVOID*)&oglDrawElements);
    MH_EnableHook(glDrawElements);

    while (gbRunning)
    {
        Sleep(1000);
    }

    MH_Uninitialize();

    return true;
}

void WINAPI Start(HMODULE hMod)
{
    int result = Run() ? 0 : 1;
    FreeLibraryAndExitThread(hMod, result);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        //DisableThreadLibraryCalls(hModule);

        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Start, hModule, NULL, NULL);

        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Main, hModule, NULL, NULL);
    }
    return TRUE;
}
