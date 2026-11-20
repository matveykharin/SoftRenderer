// rasterizer.h
// ---------------------------------------------------------------------------
// Растеризатор на барицентрических координатах + z-буфер.
// Это та часть, которую в настоящем OpenGL/DirectX выполняет «железо»
// между вершинным и фрагментным шейдером.
// ---------------------------------------------------------------------------
#pragma once
#include <vector>
#include "geometry.h"
#include "shader.h"
#include "tgaimage.h"

// Барицентрические координаты точки P относительно треугольника ABC (на экране).
// Возвращает (alpha, beta, gamma): P = alpha*A + beta*B + gamma*C,
// alpha+beta+gamma = 1. Если хоть одна координата < 0 — точка снаружи.
Vec3f barycentric(const Vec2f& A, const Vec2f& B, const Vec2f& C, const Vec2f& P);

// Нарисовать один треугольник.
//   clip     — координаты трёх вершин в clip space (результат vertex-шейдера)
//   shader   — фрагментный шейдер, который посчитает цвет пикселя
//   image    — куда рисуем
//   zbuffer  — буфер глубины размером width*height (изначально +бесконечность)
//   vp       — матрица Viewport
void triangle(const Vec4f clip[3], IShader& shader, TGAImage& image,
              std::vector<float>& zbuffer, const Mat4& vp);
