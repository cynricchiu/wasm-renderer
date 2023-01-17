#include <iostream>
using namespace std;

/**
 * 创建缓冲区
 */

uint8_t *createUint8Buffer(int length) {
  uint8_t *buffer = (uint8_t *)malloc(length * sizeof(uint8_t));
  return buffer;
}

float *createFloatBuffer(int length) {
  float *buffer = (float *)malloc(length * sizeof(float));
  return buffer;
}

uint32_t *createUint32Buffer(int length) {
  uint32_t *buffer = (uint32_t *)malloc(length * sizeof(uint32_t));
  return buffer;
}