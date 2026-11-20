// camera.h
// ---------------------------------------------------------------------------
// Камера в пространстве. Отвечает за две матрицы конвейера:
//   * матрица вида (View)        — переводит мир в систему координат камеры;
//   * матрица проекции (Projection) — делает перспективу (дальше = меньше).
// Плюс отдельная функция viewport(): перевод из [-1..1] в пиксели картинки.
//
// Полный путь вершины:
//   world --View--> view (camera) space --Projection--> clip space
//         --деление на w--> NDC [-1..1]^3 --Viewport--> пиксели экрана
// ---------------------------------------------------------------------------
#pragma once
#include "geometry.h"

class Camera {
public:
    Vec3f position;           // где стоит камера
    Vec3f target;             // куда смотрит
    Vec3f up;                 // какое направление считать «верхом»
    float fov_y_degrees;      // вертикальный угол обзора
    float aspect;             // ширина / высота картинки
    float z_near, z_far;      // ближняя и дальняя плоскости отсечения

    Camera(Vec3f position_, Vec3f target_, Vec3f up_,
           float fov_y_degrees_, float aspect_,
           float z_near_ = 0.1f, float z_far_ = 100.f);

    Mat4 view() const;        // «LookAt»
    Mat4 projection() const;  // перспективная проекция в стиле OpenGL
};

// Матрица Viewport: NDC x,y из [-1..1] -> пиксели [0..width] x [0..height],
// глубина z из [-1..1] -> [0..1] (0 — ближняя плоскость, 1 — дальняя).
Mat4 viewport(int width, int height);
