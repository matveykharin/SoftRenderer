// model.cpp — парсер .obj и доступ к данным модели.
#include "model.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

// Разбор одного элемента грани: "7", "7/3", "7//5" или "7/3/5".
// В .obj индексы начинаются с 1, а отрицательные считаются от конца списка.
static int fix_index(int idx, int count) {
    if (idx > 0) return idx - 1;
    if (idx < 0) return count + idx;
    return -1;
}

bool Model::load(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) { std::cerr << "Не удалось открыть модель " << filename << "\n"; return false; }

    std::string line;
    while (std::getline(in, line)) {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") {
            Vec3f p; iss >> p.x >> p.y >> p.z;
            positions_.push_back(p);
        } else if (type == "vt") {
            Vec2f t; iss >> t.x >> t.y;
            uvs_.push_back(t);
        } else if (type == "vn") {
            Vec3f n; iss >> n.x >> n.y >> n.z;
            normals_.push_back(n.normalized());   // в .obj нормали могут быть не единичными
        } else if (type == "f") {
            std::vector<FaceVertex> poly;
            std::string token;
            while (iss >> token) {
                FaceVertex fv;
                int a = 0, b = 0, c = 0;
                // Заменяем '/' на пробелы и аккуратно разбираем три поля,
                // учитывая, что среднее может быть пустым ("7//5").
                size_t s1 = token.find('/');
                if (s1 == std::string::npos) {
                    a = std::stoi(token);
                } else {
                    a = std::stoi(token.substr(0, s1));
                    size_t s2 = token.find('/', s1 + 1);
                    std::string mid = token.substr(s1 + 1, s2 == std::string::npos ? std::string::npos : s2 - s1 - 1);
                    if (!mid.empty()) b = std::stoi(mid);
                    if (s2 != std::string::npos && s2 + 1 < token.size()) c = std::stoi(token.substr(s2 + 1));
                }
                fv.v  = fix_index(a, static_cast<int>(positions_.size()));
                fv.vt = fix_index(b, static_cast<int>(uvs_.size()));
                fv.vn = fix_index(c, static_cast<int>(normals_.size()));
                poly.push_back(fv);
            }
            // Триангуляция «веером»: (0,1,2), (0,2,3), (0,3,4) ...
            for (size_t i = 1; i + 1 < poly.size(); i++)
                faces_.push_back({ poly[0], poly[i], poly[i + 1] });
        }
        // Прочие строки (комментарии #, o, g, s, usemtl...) пропускаем.
    }

    compute_normals_if_missing();
    std::cout << "Модель " << filename << ": вершин " << positions_.size()
              << ", треугольников " << faces_.size()
              << ", uv " << uvs_.size() << ", нормалей " << normals_.size() << "\n";
    return !faces_.empty();
}

// Если в файле нет нормалей (строк vn), считаем их сами: для каждой вершины
// усредняем нормали всех треугольников, в которые она входит. Получается
// «гладкая» модель, а не гранёная.
void Model::compute_normals_if_missing() {
    bool missing = false;
    for (auto& f : faces_) for (auto& fv : f) if (fv.vn < 0) missing = true;
    if (!missing) return;

    normals_.assign(positions_.size(), Vec3f(0, 0, 0));
    for (auto& f : faces_) {
        Vec3f a = positions_[f[0].v], b = positions_[f[1].v], c = positions_[f[2].v];
        Vec3f n = cross(b - a, c - a);              // нормаль грани (длина ~ площади)
        for (auto& fv : f) normals_[fv.v] = normals_[fv.v] + n;
    }
    for (auto& n : normals_) n = n.normalized();
    for (auto& f : faces_) for (auto& fv : f) fv.vn = fv.v;
}

Vec3f Model::vert(int iface, int nthvert) const   { return positions_[faces_[iface][nthvert].v]; }

Vec3f Model::normal(int iface, int nthvert) const {
    int i = faces_[iface][nthvert].vn;
    return i >= 0 ? normals_[i] : Vec3f(0, 0, 1);
}

Vec2f Model::uv(int iface, int nthvert) const {
    int i = faces_[iface][nthvert].vt;
    return i >= 0 ? uvs_[i] : Vec2f(0, 0);
}

bool Model::load_diffuse_texture(const std::string& filename) {
    if (!diffuse_.read_tga_file(filename)) { diffuse_ = TGAImage(); return false; }
    std::cout << "Текстура " << filename << ": " << diffuse_.width() << "x" << diffuse_.height() << "\n";
    return true;
}

// Выборка из текстуры: (u, v) в [0..1] умножаем на размер картинки и берём
// ближайший пиксель. v = 0 — низ текстуры (как и наша строка 0).
Vec3f Model::diffuse(const Vec2f& uv) const {
    float u = uv.x - std::floor(uv.x);              // «заворачиваем» координаты в [0..1)
    float v = uv.y - std::floor(uv.y);
    TGAColor c = diffuse_.get(static_cast<int>(u * diffuse_.width()),
                              static_cast<int>(v * diffuse_.height()));
    return Vec3f(c.r / 255.f, c.g / 255.f, c.b / 255.f);
}
