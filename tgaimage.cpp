// tgaimage.cpp — реализация чтения/записи TGA и записи BMP.
#include "tgaimage.h"
#include <fstream>
#include <iostream>
#include <algorithm>

TGAImage::TGAImage(int w, int h) : w_(w), h_(h), data_(static_cast<size_t>(w) * h) {}

void TGAImage::set(int x, int y, const TGAColor& c) {
    if (x < 0 || y < 0 || x >= w_ || y >= h_) return;
    data_[static_cast<size_t>(x) + static_cast<size_t>(y) * w_] = c;
}

TGAColor TGAImage::get(int x, int y) const {
    if (x < 0 || y < 0 || x >= w_ || y >= h_) return TGAColor();
    return data_[static_cast<size_t>(x) + static_cast<size_t>(y) * w_];
}

void TGAImage::flip_vertically() {
    for (int y = 0; y < h_ / 2; y++)
        for (int x = 0; x < w_; x++)
            std::swap(data_[x + y * w_], data_[x + (h_ - 1 - y) * w_]);
}

// ---------------------------------------------------------------------------
// Формат TGA: 18 байт заголовка, затем пиксели.
//   байт 2      — тип: 2 = цветное несжатое, 3 = серое несжатое,
//                       10 = цветное RLE, 11 = серое RLE
//   байты 12-15 — ширина и высота (little-endian)
//   байт 16     — бит на пиксель (8, 24 или 32)
//   байт 17     — флаги; бит 0x20 = строки идут сверху вниз
// ---------------------------------------------------------------------------
static TGAColor color_from_bytes(const std::uint8_t* p, int bpp) {
    if (bpp == 1) return TGAColor(p[0], p[0], p[0], 255);           // серый
    if (bpp == 3) return TGAColor(p[2], p[1], p[0], 255);           // B G R
    return TGAColor(p[2], p[1], p[0], p[3]);                        // B G R A
}

bool TGAImage::read_tga_file(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in) { std::cerr << "Не удалось открыть " << filename << "\n"; return false; }

    std::uint8_t header[18];
    in.read(reinterpret_cast<char*>(header), 18);
    if (!in) { std::cerr << "Повреждён заголовок " << filename << "\n"; return false; }

    const int id_length  = header[0];
    const int image_type = header[2];
    w_ = header[12] | (header[13] << 8);
    h_ = header[14] | (header[15] << 8);
    const int bpp = header[16] / 8;                 // байт на пиксель
    const bool top_to_bottom = (header[17] & 0x20) != 0;

    if (w_ <= 0 || h_ <= 0 || (bpp != 1 && bpp != 3 && bpp != 4)) {
        std::cerr << "Неподдерживаемый TGA: " << filename << "\n"; return false;
    }
    in.ignore(id_length);                           // пропускаем необязательный ID
    data_.assign(static_cast<size_t>(w_) * h_, TGAColor());

    const size_t total = static_cast<size_t>(w_) * h_;
    std::uint8_t px[4];

    if (image_type == 2 || image_type == 3) {       // несжатый
        for (size_t i = 0; i < total; i++) {
            in.read(reinterpret_cast<char*>(px), bpp);
            data_[i] = color_from_bytes(px, bpp);
        }
    } else if (image_type == 10 || image_type == 11) { // RLE-сжатый
        // RLE: байт-заголовок пакета. Если старший бит = 1, следующий пиксель
        // повторяется (n+1) раз; иначе дальше идут (n+1) разных пикселей.
        size_t i = 0;
        while (i < total && in) {
            int chunk = in.get();
            if (chunk < 128) {
                for (int k = 0; k <= chunk && i < total; k++) {
                    in.read(reinterpret_cast<char*>(px), bpp);
                    data_[i++] = color_from_bytes(px, bpp);
                }
            } else {
                in.read(reinterpret_cast<char*>(px), bpp);
                TGAColor c = color_from_bytes(px, bpp);
                for (int k = 0; k <= chunk - 128 && i < total; k++) data_[i++] = c;
            }
        }
    } else {
        std::cerr << "Тип TGA " << image_type << " не поддерживается\n"; return false;
    }
    if (!in) { std::cerr << "Файл обрезан: " << filename << "\n"; return false; }

    // У нас строка 0 — нижняя. Если в файле строки сверху вниз — переворачиваем.
    if (top_to_bottom) flip_vertically();
    return true;
}

bool TGAImage::write_tga_file(const std::string& filename) const {
    std::ofstream out(filename, std::ios::binary);
    if (!out) { std::cerr << "Не удалось создать " << filename << "\n"; return false; }

    std::uint8_t header[18] = {0};
    header[2]  = 2;                                  // цветное несжатое
    header[12] = w_ & 0xFF; header[13] = (w_ >> 8) & 0xFF;
    header[14] = h_ & 0xFF; header[15] = (h_ >> 8) & 0xFF;
    header[16] = 32;                                 // BGRA
    header[17] = 8;                                  // 8 бит альфы, строки снизу вверх
    out.write(reinterpret_cast<const char*>(header), 18);
    for (const TGAColor& c : data_) {
        std::uint8_t px[4] = {c.b, c.g, c.r, c.a};
        out.write(reinterpret_cast<const char*>(px), 4);
    }
    return static_cast<bool>(out);
}

// BMP (24 бита): заголовок 14 байт + 40 байт, строки снизу вверх,
// каждая строка дополняется нулями до кратности 4 байтам.
static void put32(std::ofstream& out, std::uint32_t v) {
    std::uint8_t b[4] = {std::uint8_t(v), std::uint8_t(v >> 8), std::uint8_t(v >> 16), std::uint8_t(v >> 24)};
    out.write(reinterpret_cast<const char*>(b), 4);
}
static void put16(std::ofstream& out, std::uint16_t v) {
    std::uint8_t b[2] = {std::uint8_t(v), std::uint8_t(v >> 8)};
    out.write(reinterpret_cast<const char*>(b), 2);
}

bool TGAImage::write_bmp_file(const std::string& filename) const {
    std::ofstream out(filename, std::ios::binary);
    if (!out) { std::cerr << "Не удалось создать " << filename << "\n"; return false; }

    const int row_size  = (w_ * 3 + 3) & ~3;
    const std::uint32_t data_size = static_cast<std::uint32_t>(row_size) * h_;

    out.put('B'); out.put('M');
    put32(out, 54 + data_size); put32(out, 0); put32(out, 54);    // файловый заголовок
    put32(out, 40); put32(out, w_); put32(out, h_);                // заголовок DIB
    put16(out, 1); put16(out, 24);
    put32(out, 0); put32(out, data_size);
    put32(out, 2835); put32(out, 2835); put32(out, 0); put32(out, 0);

    std::vector<std::uint8_t> row(row_size, 0);
    for (int y = 0; y < h_; y++) {
        for (int x = 0; x < w_; x++) {
            const TGAColor& c = data_[x + y * w_];
            row[x * 3 + 0] = c.b; row[x * 3 + 1] = c.g; row[x * 3 + 2] = c.r;
        }
        out.write(reinterpret_cast<const char*>(row.data()), row_size);
    }
    return static_cast<bool>(out);
}
