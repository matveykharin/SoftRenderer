// main.cpp
// ---------------------------------------------------------------------------
// Точка входа. Загружает модель, настраивает камеру и свет, прогоняет
// каждый треугольник через конвейер:
//     vertex shader -> растеризатор (+ z-буфер) -> fragment shader
// и сохраняет картинки в файлы (.tga как в лекции и .bmp для просмотра в Windows).
//
// Запуск:  SoftRenderer.exe [модель.obj] [текстура.tga]
// Без аргументов ищет assets/african_head.obj (а также african_head.obj рядом
// с проектом или obj/african_head/african_head.obj). Текстура ищется рядом с моделью.
// ---------------------------------------------------------------------------
#include <fstream>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "camera.h"
#include "geometry.h"
#include "model.h"
#include "rasterizer.h"
#include "shaders.h"
#include "tgaimage.h"

#ifdef _WIN32
// Только для Windows: чтобы русский текст в консоли не превращался в «кракозябры».
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX            // иначе windows.h сломает std::min / std::max
#include <windows.h>
#endif

const int WIDTH  = 800;
const int HEIGHT = 800;

static bool file_exists(const std::string& path) { return std::ifstream(path).good(); }

// Прогоняет все треугольники модели через шейдер и растеризатор.
static void render(const Model& model, IShader& shader, const Mat4& vp,
                   TGAImage& image, std::vector<float>& zbuffer) {
    for (int i = 0; i < model.nfaces(); i++) {
        Vec4f clip[3];
        for (int j = 0; j < 3; j++)
            clip[j] = shader.vertex(i, j);               // вершинный шейдер
        triangle(clip, shader, image, zbuffer, vp);      // растеризация + фрагментный шейдер
    }
}

// Сохраняет картинку в двух форматах.
static void save(const TGAImage& image, const std::string& name) {
    image.write_tga_file(name + ".tga");
    image.write_bmp_file(name + ".bmp");
    std::cout << "  сохранено: " << name << ".tga / .bmp\n";
}

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(GetACP());   // кодировка консоли = кодировка строк в программе
#endif
    // ---- 1. Загрузка модели -------------------------------------------------
    // Без аргументов ищем модель по очереди в нескольких местах.
    // Основное — папка assets/ внутри проекта.
    std::string model_path = "assets/african_head.obj";
    if (argc > 1) {
        model_path = argv[1];
    } else {
        const char* candidates[] = { "assets/african_head.obj",
                                     "african_head.obj",
                                     "obj/african_head/african_head.obj" };
        for (const char* c : candidates)
            if (file_exists(c)) { model_path = c; break; }
    }

    Model model;
    if (!model.load(model_path)) {
        std::cerr << "Положите african_head.obj в папку assets/ проекта или передайте путь аргументом.\n";
        return 1;
    }

    // Текстура (бонус): из аргумента или по имени модели: xxx.obj -> xxx_diffuse.tga
    std::string tex_path = argc > 2 ? argv[2]
                         : model_path.substr(0, model_path.size() - 4) + "_diffuse.tga";
    if (file_exists(tex_path)) model.load_diffuse_texture(tex_path);
    else std::cout << "Текстура не найдена (" << tex_path << ") — рендер без текстуры\n";

    // ---- 2. Камера и матрицы ------------------------------------------------
    // Камера стоит прямо перед лицом модели (как на картинке в задании).
    Camera camera(Vec3f(0.f, 0.f, 3.f),      // позиция камеры
                  Vec3f(0.f, 0.f, 0.f),      // смотрим в центр модели
                  Vec3f(0.f, 1.f, 0.f),      // «верх» — ось Y
                  45.f,                      // вертикальный угол обзора, градусы
                  float(WIDTH) / HEIGHT);

    Mat4 model_matrix = Mat4::identity();     // модель не двигаем и не вращаем
    Mat4 model_view   = camera.view() * model_matrix;
    Mat4 projection   = camera.projection();
    Mat4 vp           = viewport(WIDTH, HEIGHT);

    // Направление на источник света (в мировых координатах): спереди, чуть сверху и справа.
    // Переводим его в view space, где считается освещение.
    Vec3f light_dir_world = Vec3f(0.3f, 0.5f, 1.f).normalized();
    Vec3f light_dir_view  = (camera.view() * Vec4f(light_dir_world, 0.f)).xyz().normalized();

    // ---- 3. Шейдер Фонга ----------------------------------------------------
    PhongShader shader;
    shader.model          = &model;
    shader.model_view     = model_view;
    shader.projection     = projection;
    shader.light_dir_view = light_dir_view;
    shader.material       = Material::textured();
    shader.use_texture    = model.has_texture();   // нет текстуры — будет серая модель

    // ---- 4. Рендер: картинка + z-буфер --------------------------------------
    TGAImage image(WIDTH, HEIGHT);                 // изначально чёрная
    std::vector<float> zbuffer(WIDTH * HEIGHT, std::numeric_limits<float>::max());
    render(model, shader, vp, image, zbuffer);

    save(image, "output");
    std::cout << "Готово.\n";
    return 0;
}
