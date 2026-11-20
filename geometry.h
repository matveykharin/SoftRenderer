// geometry.h
// ---------------------------------------------------------------------------
// Минимальная линейная алгебра для рендерера: векторы 2/3/4 компонент и
// матрица 4x4. Всё, что делает видеокарта с вершинами (повороты, проекция,
// перевод в пиксели), сводится к умножению матрицы 4x4 на вектор из 4 чисел.
// ---------------------------------------------------------------------------
#pragma once
#include <cmath>

const float PI = 3.14159265358979f;

// ----------------------------- Vec2f --------------------------------------
// Двумерный вектор: используется для экранных координат и текстурных (u, v).
struct Vec2f {
    float x = 0, y = 0;
    Vec2f() {}
    Vec2f(float x_, float y_) : x(x_), y(y_) {}
    Vec2f operator+(const Vec2f& o) const { return Vec2f(x + o.x, y + o.y); }
    Vec2f operator-(const Vec2f& o) const { return Vec2f(x - o.x, y - o.y); }
    Vec2f operator*(float k) const { return Vec2f(x * k, y * k); }
};

// ----------------------------- Vec3f --------------------------------------
// Трёхмерный вектор: позиции вершин, нормали, цвета, направления на свет.
struct Vec3f {
    float x = 0, y = 0, z = 0;
    Vec3f() {}
    Vec3f(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    Vec3f operator+(const Vec3f& o) const { return Vec3f(x + o.x, y + o.y, z + o.z); }
    Vec3f operator-(const Vec3f& o) const { return Vec3f(x - o.x, y - o.y, z - o.z); }
    Vec3f operator-() const { return Vec3f(-x, -y, -z); }
    Vec3f operator*(float k) const { return Vec3f(x * k, y * k, z * k); }
    Vec3f operator/(float k) const { return Vec3f(x / k, y / k, z / k); }

    // Покомпонентное произведение (нужно, например, чтобы умножить
    // цвет света на цвет материала: (r1*r2, g1*g2, b1*b2)).
    Vec3f mul(const Vec3f& o) const { return Vec3f(x * o.x, y * o.y, z * o.z); }

    float  length() const { return std::sqrt(x * x + y * y + z * z); }
    Vec3f  normalized() const { float l = length(); return l > 0 ? (*this) / l : *this; }

    float& operator[](int i)       { return i == 0 ? x : (i == 1 ? y : z); }
    float  operator[](int i) const { return i == 0 ? x : (i == 1 ? y : z); }
};

// Скалярное произведение: |a|*|b|*cos(угла между ними).
// Для единичных векторов это просто косинус угла — основа всего освещения.
inline float dot(const Vec3f& a, const Vec3f& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

// Векторное произведение: вектор, перпендикулярный и a, и b.
// Нужно для нормали треугольника и для построения базиса камеры.
inline Vec3f cross(const Vec3f& a, const Vec3f& b) {
    return Vec3f(a.y * b.z - a.z * b.y,
                 a.z * b.x - a.x * b.z,
                 a.x * b.y - a.y * b.x);
}

// ----------------------------- Vec4f --------------------------------------
// Вектор в однородных координатах (x, y, z, w).
// Точка в пространстве записывается как (x, y, z, 1), направление — (x, y, z, 0).
// Четвёртая координата нужна, чтобы перенос (сдвиг) и перспективу можно было
// записать умножением на матрицу.
struct Vec4f {
    float x = 0, y = 0, z = 0, w = 0;
    Vec4f() {}
    Vec4f(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
    Vec4f(const Vec3f& v, float w_) : x(v.x), y(v.y), z(v.z), w(w_) {}

    Vec3f xyz() const { return Vec3f(x, y, z); }

    float& operator[](int i)       { return i == 0 ? x : (i == 1 ? y : (i == 2 ? z : w)); }
    float  operator[](int i) const { return i == 0 ? x : (i == 1 ? y : (i == 2 ? z : w)); }
};

// ----------------------------- Mat4 ---------------------------------------
// Матрица 4x4, хранится по строкам: m[строка][столбец].
struct Mat4 {
    float m[4][4];

    Mat4() { for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) m[i][j] = 0.f; }

    static Mat4 identity() {
        Mat4 r;
        for (int i = 0; i < 4; i++) r.m[i][i] = 1.f;
        return r;
    }

    // Произведение матриц: применить сначала o, потом *this.
    Mat4 operator*(const Mat4& o) const {
        Mat4 r;
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                for (int k = 0; k < 4; k++)
                    r.m[i][j] += m[i][k] * o.m[k][j];
        return r;
    }

    // Матрица на вектор.
    Vec4f operator*(const Vec4f& v) const {
        Vec4f r;
        for (int i = 0; i < 4; i++)
            r[i] = m[i][0] * v.x + m[i][1] * v.y + m[i][2] * v.z + m[i][3] * v.w;
        return r;
    }
};
