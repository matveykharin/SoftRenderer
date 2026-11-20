// tgaimage.h
// ---------------------------------------------------------------------------
// Картинка в памяти + чтение/запись файлов.
//  * TGA — формат из лекции (в нём хранятся текстуры модели african_head);
//    умеем читать несжатые и RLE-сжатые файлы, записываем несжатые.
//  * BMP — только запись, чтобы результат можно было открыть стандартным
//    просмотрщиком Windows (он не понимает TGA).
//
// Соглашение: пиксель (0, 0) — ЛЕВЫЙ НИЖНИЙ угол картинки, ось y идёт вверх,
// как в OpenGL. Поэтому никакой flip_vertically в main не нужен.
// ---------------------------------------------------------------------------
#pragma once
#include <cstdint>
#include <string>
#include <vector>

// Цвет пикселя. Храним в порядке B, G, R, A — именно так байты лежат в TGA и BMP.
struct TGAColor {
    std::uint8_t b = 0, g = 0, r = 0, a = 255;
    TGAColor() {}
    TGAColor(std::uint8_t R, std::uint8_t G, std::uint8_t B, std::uint8_t A = 255)
        : b(B), g(G), r(R), a(A) {}
};

class TGAImage {
public:
    TGAImage() {}
    TGAImage(int w, int h);

    bool read_tga_file(const std::string& filename);
    bool write_tga_file(const std::string& filename) const;
    bool write_bmp_file(const std::string& filename) const;

    // Записать/прочитать пиксель. Выход за границы безопасно игнорируется.
    void     set(int x, int y, const TGAColor& c);
    TGAColor get(int x, int y) const;

    int  width()  const { return w_; }
    int  height() const { return h_; }
    bool empty()  const { return w_ == 0 || h_ == 0; }

    void flip_vertically();

private:
    int w_ = 0, h_ = 0;
    std::vector<TGAColor> data_;   // w*h пикселей, строка 0 — нижняя
};
