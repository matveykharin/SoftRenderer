// camera.cpp — построение матриц вида, проекции и viewport.
#include "camera.h"
#include <cmath>

Camera::Camera(Vec3f position_, Vec3f target_, Vec3f up_,
               float fov_y_degrees_, float aspect_, float z_near_, float z_far_)
    : position(position_), target(target_), up(up_),
      fov_y_degrees(fov_y_degrees_), aspect(aspect_), z_near(z_near_), z_far(z_far_) {}

// ---------------------------------------------------------------------------
// LookAt. Строим ортонормированный базис камеры:
//   z — от цели к камере (камера смотрит вдоль -z, как в OpenGL),
//   x — «вправо» = up × z,
//   y — «вверх»  = z × x.
// Матрица вида = (поворот в этот базис) * (сдвиг на -position).
// После неё камера стоит в начале координат и смотрит вдоль -z.
// ---------------------------------------------------------------------------
Mat4 Camera::view() const {
    Vec3f z = (position - target).normalized();
    Vec3f x = cross(up, z).normalized();
    Vec3f y = cross(z, x);

    Mat4 rotation = Mat4::identity();
    for (int i = 0; i < 3; i++) {
        rotation.m[0][i] = x[i];   // строки матрицы — оси камеры
        rotation.m[1][i] = y[i];
        rotation.m[2][i] = z[i];
    }
    Mat4 translation = Mat4::identity();
    for (int i = 0; i < 3; i++) translation.m[i][3] = -position[i];

    return rotation * translation;
}

// ---------------------------------------------------------------------------
// Перспективная проекция (как gluPerspective в OpenGL).
//   f = 1 / tan(fov/2) — чем меньше угол обзора, тем сильнее «зум».
// Ключевая строка — последняя: w' = -z. После деления на w координаты x и y
// делятся на расстояние до камеры, поэтому далёкие объекты становятся меньше.
// Третья строка переводит z из [near..far] в [-1..1] (для z-буфера).
// ---------------------------------------------------------------------------
Mat4 Camera::projection() const {
    float f = 1.f / std::tan(fov_y_degrees * PI / 180.f / 2.f);
    Mat4 p;
    p.m[0][0] = f / aspect;
    p.m[1][1] = f;
    p.m[2][2] = (z_far + z_near) / (z_near - z_far);
    p.m[2][3] = 2.f * z_far * z_near / (z_near - z_far);
    p.m[3][2] = -1.f;
    return p;
}

Mat4 viewport(int width, int height) {
    Mat4 m = Mat4::identity();
    m.m[0][0] = width  / 2.f;  m.m[0][3] = width  / 2.f;
    m.m[1][1] = height / 2.f;  m.m[1][3] = height / 2.f;
    m.m[2][2] = 0.5f;          m.m[2][3] = 0.5f;
    return m;
}
