// model.h
// ---------------------------------------------------------------------------
// Загрузка полигональной сетки (меша) из файла Wavefront .obj.
// Поддерживаются строки:
//   v  x y z        — позиция вершины
//   vt u v          — текстурная координата
//   vn x y z        — нормаль
//   f  a b c ...    — грань; каждый элемент: v, v/vt, v//vn или v/vt/vn
// Грани с 4+ вершинами разбиваются на треугольники «веером».
// ---------------------------------------------------------------------------
#pragma once
#include <array>
#include <string>
#include <vector>
#include "geometry.h"
#include "tgaimage.h"

class Model {
public:
    bool load(const std::string& filename);

    int nverts() const { return static_cast<int>(positions_.size()); }
    int nfaces() const { return static_cast<int>(faces_.size()); }

    // Данные n-й вершины (0, 1 или 2) треугольника номер iface.
    Vec3f vert  (int iface, int nthvert) const;
    Vec3f normal(int iface, int nthvert) const;
    Vec2f uv    (int iface, int nthvert) const;

    // Бонус: диффузная текстура (цвет поверхности).
    bool  load_diffuse_texture(const std::string& filename);
    bool  has_texture() const { return !diffuse_.empty(); }
    Vec3f diffuse(const Vec2f& uv) const;       // цвет текстуры, компоненты 0..1

private:
    // Одна вершина грани: индексы в массивы позиций, uv и нормалей (-1 = нет).
    struct FaceVertex { int v = -1, vt = -1, vn = -1; };

    std::vector<Vec3f> positions_;
    std::vector<Vec2f> uvs_;
    std::vector<Vec3f> normals_;
    std::vector<std::array<FaceVertex, 3>> faces_;   // только треугольники
    TGAImage diffuse_;

    void compute_normals_if_missing();
};
