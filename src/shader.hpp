#include "interface.hpp"
/**
 * 光栅化
 */
#pragma region 光栅化
// 设置像素
void _setPixel(Object item, int width, int height, int x, int y, int r, int g,
               int b, int a) {
  if (item.frameBuffer.colorBuffer != NULL) {
    if (x >= 0 && x <= width && y >= 0 && y <= height) {
      // 需要限制x∈[0,w],y∈[0,h],否则会溢出
      int index = y * width + x;
      item.frameBuffer.colorBuffer[index * 4] = r;
      item.frameBuffer.colorBuffer[index * 4 + 1] = g;
      item.frameBuffer.colorBuffer[index * 4 + 2] = b;
      item.frameBuffer.colorBuffer[index * 4 + 3] = a;
    }
  }
};

// 点
void _rasterPoints(Object item, int width, int height) {
  for (int i = 0; i < item.EBO.size(); i++) {
    Vertex v = item.EBO[i];
    int x = v.screenPostion.x;
    int y = v.screenPostion.y;
    int r = v.color.r;
    int g = v.color.g;
    int b = v.color.b;
    int a = v.color.a;
    // 点的大小可调，点关于中心对称
    int gl_PointSize = 10;
    for (int j = 0; j < gl_PointSize; j++) {
      for (int k = 0; k < gl_PointSize; k++) {
        _setPixel(item, width, height, x + j - gl_PointSize / 2,
                  y + k - gl_PointSize / 2, r, g, b, a);
      }
    }
  }
}
#pragma endregion

/**
 * 着色器
 */
// step1: 依次对每个顶点坐标变换，将顶点转换到裁剪空间
Object _toClipSpace(Object item) {
  item.vertexCache.clear();
  for (int i = 0; i < item.attributes.positionsLength; i += 3) {
    float x = item.attributes.positions[i * 3];
    float y = item.attributes.positions[i * 3 + 1];
    float z = item.attributes.positions[i * 3 + 2];
    glm::vec4 a_position = {x, y, z, 1};
    Vertex vertex;
    vertex.gl_Position = item.uniforms.mvpMatrix * a_position;
    vertex.color.r = item.attributes.colors[i * 4];
    vertex.color.g = item.attributes.colors[i * 4 + 1];
    vertex.color.b = item.attributes.colors[i * 4 + 2];
    vertex.color.a = item.attributes.colors[i * 4 + 3];
    item.vertexCache.emplace_back(vertex);
  }
  return item;
}

// step2: 图元装配 + 图元裁剪
Object _processPrimitive(Object item) {
  int indicesLength = item.attributes.indicesLength;
  uint32_t *indices = item.attributes.indices;
  item.EBO.clear();
  switch (item.type) {
  case GL_POINTS: {
    for (int i = 0; i < indicesLength; i++) {
      uint32_t index = indices[i];
      Vertex v = item.vertexCache[index];
      //   // const newPoints = _clip(this.type, point);
      // points.push(...newPoints);
      item.EBO.emplace_back(v);
    }
    break;
  }
  case GL_LINES: {
    for (int i = 0; i < indicesLength; i += 2) {
      Vertex v1 = item.vertexCache[indices[i]];
      Vertex v2 = item.vertexCache[indices[i + 1]];
      // const newPoints = _clip(this.type, point);
      // points.push(...newPoints);
      item.EBO.emplace_back(v1);
      item.EBO.emplace_back(v2);
    }
    break;
  }
  case GL_LINE_STRIP: {
    for (int i = 0; i < indicesLength - 1; i++) {
      Vertex v1 = item.vertexCache[indices[i]];
      Vertex v2 = item.vertexCache[indices[i + 1]];
      // const newPoints = _clip(this.type, point);
      // points.push(...newPoints);
      item.EBO.emplace_back(v1);
      item.EBO.emplace_back(v2);
    }
    break;
  }
  case GL_LINE_LOOP: {
    for (int i = 0; i < indicesLength; i++) {
      Vertex v1, v2;
      if (i == indicesLength - 1) {
        v1 = item.vertexCache[indices[i]];
        v2 = item.vertexCache[indices[0]];
      } else {
        v1 = item.vertexCache[indices[i]];
        v2 = item.vertexCache[indices[i + 1]];
      }
      item.EBO.emplace_back(v1);
      item.EBO.emplace_back(v2);
    }
    break;
  }
  case GL_TRIANGLES: {
    for (int i = 0; i < indicesLength; i += 3) {
      Vertex v1 = item.vertexCache[indices[i]];
      Vertex v2 = item.vertexCache[indices[i + 1]];
      Vertex v3 = item.vertexCache[indices[i + 2]];
      // const newPoints = _clip(this.type, point);
      // points.push(...newPoints);
      item.EBO.emplace_back(v1);
      item.EBO.emplace_back(v2);
      item.EBO.emplace_back(v3);
    }
    break;
  }
  case GL_TRIANGLE_STRIP: {
    for (int i = 2; i < indicesLength; i++) {
      Vertex v1, v2, v3;
      if (i % 2 == 0) {
        // 顶点数为奇数
        v1 = item.vertexCache[indices[i - 2]];
        v2 = item.vertexCache[indices[i - 1]];
        v3 = item.vertexCache[indices[i]];

      } else {
        // 偶数
        v1 = item.vertexCache[indices[i - 1]];
        v2 = item.vertexCache[indices[i - 2]];
        v3 = item.vertexCache[indices[i]];
      }
      item.EBO.emplace_back(v1);
      item.EBO.emplace_back(v2);
      item.EBO.emplace_back(v3);
    }
    break;
  }
  case GL_TRIANGLE_FAN: {
    Vertex v0 = item.vertexCache[indices[0]];
    for (int i = 1; i < indicesLength - 1; i++) {
      Vertex v1 = item.vertexCache[indices[i]];
      Vertex v2 = item.vertexCache[indices[i + 1]];
      item.EBO.emplace_back(v0);
      item.EBO.emplace_back(v1);
      item.EBO.emplace_back(v2);
    }
    break;
  }
  default: {
    break;
  }
  }
  return item;
}

// step3: 裁剪空间-NDC空间-视口空间
Object _toViewPortSpace(Object item) {
  switch (item.type) {
  case GL_POINTS: {
    for (int i = 0; i < item.EBO.size(); i++) {
      float x = item.EBO[i].gl_Position.x;
      float y = item.EBO[i].gl_Position.y;
      float z = item.EBO[i].gl_Position.z;
      float w = item.EBO[i].gl_Position.w;
      // 裁剪空间-NDC空间：透视除法
      glm::vec4 position = {x / w, y / w, z / w, 1.0};
      // NDC空间-视口空间：得到屏幕坐标
      glm::vec4 screenPosition = item.uniforms.viewportMatrix * position;
      // 取整
      item.EBO[i].screenPostion.x = floor(screenPosition.x);
      item.EBO[i].screenPostion.y = floor(screenPosition.y);
      item.EBO[i].screenPostion.z = floor(screenPosition.z);
      item.EBO[i].screenPostion.w = 1;
    }
    break;
  }
  case GL_LINES:
  case GL_LINE_STRIP:
  case GL_LINE_LOOP: {
    break;
  }
  case GL_TRIANGLES:
  case GL_TRIANGLE_FAN:
  case GL_TRIANGLE_STRIP: {
    break;
  }
  default:
    break;
  }
  return item;
}

// step4: 光栅化
Object _rasterization(Object item, int width, int height) {
  // 清空画布
  for (int i = 0; i < width * height; i++) {
    item.frameBuffer.colorBuffer[i * 4] = 0;
    item.frameBuffer.colorBuffer[i * 4 + 1] = 0;
    item.frameBuffer.colorBuffer[i * 4 + 2] = 0;
    item.frameBuffer.colorBuffer[i * 4 + 3] = 0;
  }
  switch (item.type) {
  case GL_POINTS: {
    _rasterPoints(item, width, height);
    break;
  }
  case GL_LINES:
  case GL_LINE_STRIP:
  case GL_LINE_LOOP: {
    break;
  }
  case GL_TRIANGLES:
  case GL_TRIANGLE_FAN:
  case GL_TRIANGLE_STRIP: {
    break;
  }
  default:
    break;
  }
  return item;
}
