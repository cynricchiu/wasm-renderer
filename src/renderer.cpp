#include "buffer.hpp"
#include "emscripten/emscripten.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "shader.hpp"
#include <cmath>
#include <iostream>

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

#pragma region 全局变量
Renderer renderer;
uint32_t id = 0; // 物体序号
#pragma endregion

#pragma region 数据交互
// 初始化
EMSCRIPTEN_KEEPALIVE void init(int w, int h) {
  renderer.width = w;
  renderer.height = h;
  renderer.frameBuffer.colorBuffer = NULL;
}

// 添加物体
EMSCRIPTEN_KEEPALIVE int32_t addObject(uint8_t type) {
  Object item;
  item.type = type;
  item.id = id++;
  item.attributes.positions = NULL;
  item.attributes.colors = NULL;
  item.attributes.indices = NULL;
  item.uniforms.mvpMatrix = glm::mat4(1.0f);
  item.uniforms.viewportMatrix = glm::mat4(1.0f);
  item.frameBuffer.colorBuffer =
      createUint8Buffer(renderer.width * renderer.height * 4);
  renderer.ObjectList.emplace(item.id, item);
  return item.id; // 返回序号
}

// 输入物体信息
// attributes.positions
EMSCRIPTEN_KEEPALIVE float *setAttributePositions(uint32_t id, int length) {
  if (renderer.ObjectList[id].attributes.positions != NULL) {
    free(renderer.ObjectList[id].attributes.positions);
    renderer.ObjectList[id].attributes.positions = NULL;
  }
  float *positions = renderer.ObjectList[id].attributes.positions =
      createFloatBuffer(length);
  renderer.ObjectList[id].attributes.positionsLength = length;
  return positions;
}

// attributes.colors
EMSCRIPTEN_KEEPALIVE uint8_t *setAttributeColors(int32_t id, int length) {
  if (renderer.ObjectList[id].attributes.colors != NULL) {
    free(renderer.ObjectList[id].attributes.colors);
    renderer.ObjectList[id].attributes.colors = NULL;
  }
  uint8_t *colors = renderer.ObjectList[id].attributes.colors =
      createUint8Buffer(length);
  renderer.ObjectList[id].attributes.colorsLength = length;
  return colors;
}

// attributes.indices
EMSCRIPTEN_KEEPALIVE uint32_t *setAttributeIndices(int32_t id, int length) {
  if (renderer.ObjectList[id].attributes.indices != NULL) {
    free(renderer.ObjectList[id].attributes.indices);
    renderer.ObjectList[id].attributes.indices = NULL;
  }
  uint32_t *indices = renderer.ObjectList[id].attributes.indices =
      createUint32Buffer(length);
  renderer.ObjectList[id].attributes.indicesLength = length;
  return indices;
}

// 设置矩阵
void setMatrixValue(glm::mat4 mat, float a0, float b0, float c0, float d0,
                    float a1, float b1, float c1, float d1, float a2, float b2,
                    float c2, float d2, float a3, float b3, float c3,
                    float d3) {
  mat[0][0] = a0;
  mat[1][0] = b0;
  mat[2][0] = c0;
  mat[3][0] = d0;
  mat[0][1] = a1;
  mat[1][1] = b1;
  mat[2][1] = c1;
  mat[3][1] = d1;
  mat[0][2] = a2;
  mat[1][2] = b2;
  mat[2][2] = c2;
  mat[3][2] = d2;
  mat[0][3] = a3;
  mat[1][3] = b3;
  mat[2][3] = c3;
  mat[3][3] = d3;
  glm::value_ptr(mat); // 按列存储
}

// uniforms.mvpMatrix
EMSCRIPTEN_KEEPALIVE void setUniformMVPMatrix(int32_t id, float a0, float b0,
                                              float c0, float d0, float a1,
                                              float b1, float c1, float d1,
                                              float a2, float b2, float c2,
                                              float d2, float a3, float b3,
                                              float c3, float d3) {
  setMatrixValue(renderer.ObjectList[id].uniforms.mvpMatrix, a0, b0, c0, d0, a1,
                 b1, c1, d1, a2, b2, c2, d2, a3, b3, c3, d3);
}

// uniforms.viewportMatrix
EMSCRIPTEN_KEEPALIVE void
setUniformViewportMatrix(int32_t id, float a0, float b0, float c0, float d0,
                         float a1, float b1, float c1, float d1, float a2,
                         float b2, float c2, float d2, float a3, float b3,
                         float c3, float d3) {
  setMatrixValue(renderer.ObjectList[id].uniforms.viewportMatrix, a0, b0, c0,
                 d0, a1, b1, c1, d1, a2, b2, c2, d2, a3, b3, c3, d3);
}

// 删除物体
EMSCRIPTEN_KEEPALIVE void removeObject(int id) {
  if (renderer.ObjectList.find(id) != renderer.ObjectList.end()) {
    free(renderer.ObjectList[id].attributes.positions);
    renderer.ObjectList[id].attributes.positions = NULL;
    free(renderer.ObjectList[id].attributes.colors);
    renderer.ObjectList[id].attributes.colors = NULL;
    free(renderer.ObjectList[id].attributes.indices);
    renderer.ObjectList[id].attributes.indices = NULL;
    free(renderer.ObjectList[id].frameBuffer.colorBuffer);
    renderer.ObjectList[id].frameBuffer.colorBuffer = NULL;
  }
  renderer.ObjectList.erase(id);
}
#pragma endregion

#pragma region 渲染相关

// 创建颜色缓冲区
EMSCRIPTEN_KEEPALIVE uint8_t *createColorBuffer(int w, int h) {
  if (renderer.frameBuffer.colorBuffer != NULL) {
    free(renderer.frameBuffer.colorBuffer);
    renderer.frameBuffer.colorBuffer = NULL;
  }
  renderer.frameBuffer.colorBuffer = createUint8Buffer(w * h * 4);
  return renderer.frameBuffer.colorBuffer;
}

// 清空画布
EMSCRIPTEN_KEEPALIVE void clearRect(int r, int g, int b, int a) {
  if (renderer.frameBuffer.colorBuffer != NULL) {
    int width = renderer.width;
    int height = renderer.height;
    for (int i = 0; i < width * height; i++) {
      renderer.frameBuffer.colorBuffer[i * 4] = r;
      renderer.frameBuffer.colorBuffer[i * 4 + 1] = g;
      renderer.frameBuffer.colorBuffer[i * 4 + 2] = b;
      renderer.frameBuffer.colorBuffer[i * 4 + 3] = a;
    }
  }
}
#pragma endregion

#pragma region 依次对每个物体进行着色器处理
EMSCRIPTEN_KEEPALIVE void execute() {
  int width = renderer.width;
  int height = renderer.height;
  for (auto i = renderer.ObjectList.begin(); i != renderer.ObjectList.end();
       i++) {
    uint32_t id = i->first;
    Object item = renderer.ObjectList[id];
    item = _toClipSpace(item);
    item = _processPrimitive(item);
    item = _toViewPortSpace(item);
    item = _rasterization(item, width, height);
    renderer.ObjectList[id] = item;
    // 融合颜色缓存
    uint8_t *colorBuffer = item.frameBuffer.colorBuffer;
    for (int i = 0; i < width * height; i++) {
      renderer.frameBuffer.colorBuffer[i * 4] = colorBuffer[i * 4];
      renderer.frameBuffer.colorBuffer[i * 4 + 1] = colorBuffer[i * 4 + 1];
      renderer.frameBuffer.colorBuffer[i * 4 + 2] = colorBuffer[i * 4 + 2];
      renderer.frameBuffer.colorBuffer[i * 4 + 3] = colorBuffer[i * 4 + 3];
    }
  }
}
#pragma endregion

int main(int argc, char *argv[]) {
  std::cout << "wasm ready" << endl;
  std::cout << "input args:" << endl;
  for (int i = 0; i < argc; i++) {
    std::cout << argv[i] << endl;
  }
  return 0;
}
#ifdef __cplusplus
}
#endif