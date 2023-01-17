
#include "glm/glm.hpp"
#include <iostream>
#include <unordered_map>
#include <vector>

using namespace std;

/**
 * 数据结构定义
 */

// 图元类型
enum GL_ENUM_MODE {
  GL_POINTS,
  GL_LINES,
  GL_LINE_STRIP,
  GL_LINE_LOOP,
  GL_TRIANGLES,
  GL_TRIANGLE_STRIP,
  GL_TRIANGLE_FAN,
};

// 输入attributes变量
typedef struct {
  float *positions; // 顶点数据
  uint32_t positionsLength;
  uint8_t *colors; // 颜色数据
  uint32_t colorsLength;
  uint32_t *indices; // 顶点顺序
  uint32_t indicesLength;
} Attributes;

// 输入uniforms变量
typedef struct {
  glm::mat4 mvpMatrix;      // mvp矩阵
  glm::mat4 viewportMatrix; // viewport矩阵
} Uniforms;

// 顶点
typedef struct {
  glm::vec4 gl_Position;
  glm::vec4 screenPostion;
  glm::vec4 color;
} Vertex;

// 片元

// 帧缓存
typedef struct {
  uint8_t *colorBuffer; // 颜色缓存
  float *depthBuffer;   // 深度缓存
} FrameBuffer;

// 物体信息
typedef struct {
  uint32_t id;                // 唯一标识
  uint8_t type;               // GL_ENUM_MODE
  Attributes attributes;      // 输入attributes
  Uniforms uniforms;          // 输入uniforms
  vector<Vertex> vertexCache; // 顶点计算结果缓存
  vector<Vertex> EBO;         // 图元缓冲对象
  FrameBuffer frameBuffer;    // 帧缓存
} Object;

// 渲染器
typedef struct {
  int width;
  int height;
  unordered_map<uint32_t, Object> ObjectList; // 物体列表
  FrameBuffer frameBuffer;                    // 融合后的帧缓存
} Renderer;
