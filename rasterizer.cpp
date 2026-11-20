// rasterizer.cpp — растеризация треугольника.
#include "rasterizer.h"
#include <algorithm>
#include <cmath>

// ---------------------------------------------------------------------------
// Барицентрические координаты (слайды 43-46).
// Ищем u, v такие, что  P = A + u*AB + v*AC, то есть
//     u*AB + v*AC + PA = 0.
// Расписав по x и по y, получаем: вектор (u, v, 1) перпендикулярен
// и (ABx, ACx, PAx), и (ABy, ACy, PAy). Значит, он параллелен их
// векторному произведению. Делим результат на третью компоненту — получаем (u, v, 1).
// ---------------------------------------------------------------------------
Vec3f barycentric(const Vec2f& A, const Vec2f& B, const Vec2f& C, const Vec2f& P) {
    Vec3f s = cross(Vec3f(C.x - A.x, B.x - A.x, A.x - P.x),
                    Vec3f(C.y - A.y, B.y - A.y, A.y - P.y));
    // s.z — удвоенная площадь треугольника. Если она ~0, треугольник вырожден.
    if (std::abs(s.z) < 1e-6f) return Vec3f(-1, 1, 1);
    // s.x соответствует AC (вес вершины C), s.y — AB (вес вершины B).
    return Vec3f(1.f - (s.x + s.y) / s.z, s.y / s.z, s.x / s.z);
}

void triangle(const Vec4f clip[3], IShader& shader, TGAImage& image,
              std::vector<float>& zbuffer, const Mat4& vp) {
    // Если вершина оказалась позади камеры (w <= 0), деление на w даст мусор.
    // Настоящий OpenGL в таком случае режет треугольник (clipping); мы для
    // простоты такой треугольник пропускаем.
    for (int i = 0; i < 3; i++) if (clip[i].w <= 1e-5f) return;

    // 1. Clip space -> экран. Сначала Viewport, потом деление на w
    //    (перспективное деление). Порядок неважен: Viewport — аффинная матрица.
    Vec2f screen[3];     // x, y в пикселях
    float depth[3];      // глубина в [0..1]
    for (int i = 0; i < 3; i++) {
        Vec4f s = vp * clip[i];
        screen[i] = Vec2f(s.x / s.w, s.y / s.w);
        depth[i]  = s.z / s.w;
    }

    // 2. Описывающий прямоугольник (bounding box), обрезанный краями картинки.
    const int W = image.width(), H = image.height();
    float minx = std::min(screen[0].x, std::min(screen[1].x, screen[2].x));
    float maxx = std::max(screen[0].x, std::max(screen[1].x, screen[2].x));
    float miny = std::min(screen[0].y, std::min(screen[1].y, screen[2].y));
    float maxy = std::max(screen[0].y, std::max(screen[1].y, screen[2].y));
    int x0 = std::max(0,     static_cast<int>(std::floor(minx)));
    int x1 = std::min(W - 1, static_cast<int>(std::ceil (maxx)));
    int y0 = std::max(0,     static_cast<int>(std::floor(miny)));
    int y1 = std::min(H - 1, static_cast<int>(std::ceil (maxy)));

    // 3. Перебираем все пиксели прямоугольника.
    TGAColor color;
    for (int x = x0; x <= x1; x++) {
        for (int y = y0; y <= y1; y++) {
            // Берём центр пикселя.
            Vec3f bc_screen = barycentric(screen[0], screen[1], screen[2],
                                          Vec2f(x + 0.5f, y + 0.5f));
            // Пиксель вне треугольника — пропускаем.
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;

            // 4. Глубина пикселя. После перспективного деления z меняется
            //    по экрану линейно, поэтому её интерполируем экранными весами.
            float z = bc_screen.x * depth[0] + bc_screen.y * depth[1] + bc_screen.z * depth[2];
            if (z < 0.f || z > 1.f) continue;               // за near/far плоскостями

            // 5. Z-тест: рисуем, только если этот пиксель ближе уже нарисованного.
            float& zb = zbuffer[static_cast<size_t>(x) + static_cast<size_t>(y) * W];
            if (z >= zb) continue;

            // 6. Перспективно-корректная интерполяция (слайды 56-58).
            //    Экранные барицентрические веса искажены перспективой. Делим
            //    каждый вес на w своей вершины и нормируем сумму на 1 —
            //    получаем «правильные» веса в clip space. Именно их получает
            //    фрагментный шейдер, поэтому текстура не «плывёт».
            Vec3f bc_clip(bc_screen.x / clip[0].w,
                          bc_screen.y / clip[1].w,
                          bc_screen.z / clip[2].w);
            bc_clip = bc_clip / (bc_clip.x + bc_clip.y + bc_clip.z);

            // 7. Фрагментный шейдер считает цвет.
            bool discard = shader.fragment(bc_clip, color);
            if (discard) continue;

            zb = z;
            image.set(x, y, color);
        }
    }
}
