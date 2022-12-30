#include "emscripten/emscripten.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <array>
#include <cmath>
#include <iostream>
#include <string>
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  float *positions;
  float *colors;
  uint8_t *indices;
} Atrributes;

typedef struct {
  float *mvpMatrix;
  float *viewportMatrix;
} Uniforms;

typedef struct {
  int width;
  int height;
  float *positions;
  uint8_t *colorBuffer;
} Renderer;

// 全局变量
Renderer renderer;

// 初始化
EMSCRIPTEN_KEEPALIVE void init(int w, int h) {
  renderer.width = w;
  renderer.height = h;
  renderer.colorBuffer = NULL;
}

// 创建颜色缓冲区
EMSCRIPTEN_KEEPALIVE uint8_t *createColorBuffer(int w, int h) {
  if (renderer.colorBuffer) {
    free(renderer.colorBuffer);
  }
  renderer.colorBuffer = (uint8_t *)malloc(w * h * 4 * sizeof(uint8_t));
  return renderer.colorBuffer;
}

// 清空画布
EMSCRIPTEN_KEEPALIVE void clearRect() {
  if (renderer.colorBuffer) {
    int width = renderer.width;
    int height = renderer.height;
    for (int i = 0; i < width * height; i++) {
      renderer.colorBuffer[i * 4] = 0;
      renderer.colorBuffer[i * 4 + 1] = 0;
      renderer.colorBuffer[i * 4 + 2] = 0;
      renderer.colorBuffer[i * 4 + 3] = 255;
    }
  }
}

// 设置像素
void setPixel(int x, int y, int r, int g, int b, int a) {
  if (renderer.colorBuffer) {
    int width = renderer.width;
    int height = renderer.height;
    if (x >= 0 && x <= width && y >= 0 && y <= height) {
      // 需要限制x∈[0,w],y∈[0,h],否则会溢出
      x = x >> 0; // 列
      y = y >> 0; // 行
      int index = y * width + x;
      renderer.colorBuffer[index * 4] = r;
      renderer.colorBuffer[index * 4 + 1] = g;
      renderer.colorBuffer[index * 4 + 2] = b;
      renderer.colorBuffer[index * 4 + 3] = a;
    }
  }
};

// 绘制圆
EMSCRIPTEN_KEEPALIVE void drawCircle(int x0, int y0, int radius, int r, int g,
                                     int b, int a) {
  int width = renderer.width;
  int height = renderer.height;
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      int d = hypot(x - x0, y - y0);
      if (d <= radius) {
        setPixel(x, y, r, g, b, a);
      }
    }
  }
}

EMSCRIPTEN_KEEPALIVE string getString(string name) { return name; }

EMSCRIPTEN_KEEPALIVE int getArray(int *array, int index) {
  return array[index];
}

EMSCRIPTEN_KEEPALIVE int test() {
  glm::vec4 vec(1.0f, 0.0f, 0.0f, 1.0f);
  return vec.x + vec.y + vec.z + vec.w;
}

int main(int argc, char *argv[]) {
  std::cout << "wasm ready" << endl;
  std::cout << "input args:" << endl;
  for (int i = 0; i < argc; i++) {
    std::cout << argv[i] << endl;
  }
  glm::vec4 vec(1.0f, 0.0f, 0.0f, 1.0f);
  glm::mat4 mat(1.0f);
  mat[1][1] = 666;
  glm::value_ptr(mat);
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      cout << mat[i][j] << endl;
    }
  }
  return 0;
}
#ifdef __cplusplus
}
#endif