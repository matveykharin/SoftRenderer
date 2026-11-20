// shaders.h
// ---------------------------------------------------------------------------
// PhongShader — шейдер освещения по модели Фонга (ambient + diffuse + specular).
// Освещение считается в КАЖДОМ пикселе по интерполированной нормали;
// при наличии текстуры цвет поверхности берётся из неё (бонус).
//
// Все расчёты освещения ведём в пространстве камеры (view space):
// там камера находится в точке (0,0,0), и вектор «к наблюдателю» — это просто
// -позиция точки.
// ---------------------------------------------------------------------------
#pragma once
#include <algorithm>
#include <cmath>
#include "geometry.h"
#include "model.h"
#include "shader.h"

// Параметры материала для модели Фонга
struct Material {
    Vec3f ka;          // коэффициент фонового (ambient) отражения, по R,G,B
    Vec3f kd;          // коэффициент диффузного отражения
    Vec3f ks;          // коэффициент зеркального (specular) отражения
    float shininess;   // α — «блеск»: чем больше, тем меньше и резче блик

    // Материал для текстурированной модели: цвет поверхности берётся
 // из текстуры, поэтому kd = 1; слабый фон и неяркий широкий блик.
    static Material textured() { return { Vec3f(0.15f, 0.15f, 0.15f), Vec3f(1.f, 1.f, 1.f), Vec3f(0.3f, 0.3f, 0.3f), 10.f }; }
};

// Перевод цвета из [0..1] в байты с ограничением сверху (иначе 1.2 -> переполнение).
inline TGAColor to_color(const Vec3f& c) {
    auto ch = [](float v) { return static_cast<std::uint8_t>(std::min(1.f, std::max(0.f, v)) * 255.f); };
    return TGAColor(ch(c.x), ch(c.y), ch(c.z));
}

// Интерполяция трёх значений барицентрическими весами.
inline Vec3f interpolate(const Vec3f v[3], const Vec3f& bar) { return v[0] * bar.x + v[1] * bar.y + v[2] * bar.z; }
inline Vec2f interpolate(const Vec2f v[3], const Vec3f& bar) { return v[0] * bar.x + v[1] * bar.y + v[2] * bar.z; }

// ===========================================================================
// Шейдер Фонга
// ===========================================================================
struct PhongShader : public IShader {
    // uniform
    const Model* model = nullptr;
    Mat4     model_view;
    Mat4     projection;
    Vec3f    light_dir_view;           // направление на свет в view space (единичное)
    Vec3f    light_color = Vec3f(1, 1, 1);
    Vec3f    ambient_light = Vec3f(1, 1, 1);
    Material material = Material::textured();
    bool     use_texture = false;      // бонус: брать цвет из текстуры

    // varying
    Vec3f varying_pos[3];              // позиция вершины в view space
    Vec3f varying_normal[3];           // нормаль в view space
    Vec2f varying_uv[3];               // текстурные координаты

    Vec4f vertex(int iface, int nthvert) override {
        Vec4f p = model_view * Vec4f(model->vert(iface, nthvert), 1.f);
        varying_pos[nthvert]    = p.xyz();
        varying_normal[nthvert] = (model_view * Vec4f(model->normal(iface, nthvert), 0.f)).xyz();
        varying_uv[nthvert]     = model->uv(iface, nthvert);
        return projection * p;
    }

    bool fragment(const Vec3f& bar, TGAColor& color) override {
        // После интерполяции нормаль уже не единичная — нормализуем заново.
        Vec3f n = interpolate(varying_normal, bar).normalized();
        Vec3f p = interpolate(varying_pos, bar);

        Vec3f l = light_dir_view;                  // к источнику света
        Vec3f v = (-p).normalized();               // к наблюдателю (камера в (0,0,0))
        float n_dot_l = dot(n, l);

        // Зеркальное направление: r = 2(n·l)n - l  (угол падения = углу отражения).
        Vec3f r = (n * (2.f * n_dot_l) - l).normalized();

        // Коэффициенты материала. Если есть текстура — цвет поверхности берём из неё.
        Vec3f kd = material.kd, ka = material.ka;
        if (use_texture && model->has_texture()) {
            Vec3f tex = model->diffuse(interpolate(varying_uv, bar));
            kd = kd.mul(tex);
            ka = ka.mul(tex);
        }

        // Три компоненты модели Фонга:
        Vec3f ambient  = ka.mul(ambient_light);                                       // константа
        Vec3f diffuse  = kd.mul(light_color) * std::max(0.f, n_dot_l);                // Ламберт: cos θ
        Vec3f specular(0, 0, 0);
        if (n_dot_l > 0.f)                                                            // блик только на освещённой стороне
            specular = material.ks.mul(light_color) * std::pow(std::max(0.f, dot(r, v)), material.shininess); // cos^α φ

        color = to_color(ambient + diffuse + specular);
        return false;
    }
};
