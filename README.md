# SoftRenderer — «OpenGL at Home»

Программный рендерер на C++ без сторонних библиотек: растеризует меш из `.obj`
в картинку, повторяя основные этапы конвейера OpenGL.

## Что реализовано

| Требование задания | Где в коде |
|---|---|
| Рендер меша в текстуру (картинку) | `main.cpp`, `tgaimage.*` |
| Шейдеры (вершинный + фрагментный) | интерфейс `IShader` в `shader.h` |
| Растеризатор на барицентрических координатах + z-буфер | `rasterizer.*` |
| Шейдер освещения Фонга (ambient + diffuse + specular) | `PhongShader` в `shaders.h` |
| Класс камеры, матрицы вида и проекции | `camera.*` |
| Загрузка `.obj` (свой парсер) | `model.*` |
| **Бонус:** текстурирование | `Model::diffuse`, `PhongShader::fragment` |

Дополнительно: перспективно-корректная интерполяция.

## Конвейер

```
вершины .obj ──► vertex shader ──► clip space ──► деление на w ──► Viewport
   (мир)       (View, Projection)                    (NDC)          (пиксели)
                                                                       │
картинка ◄── fragment shader (Фонг) ◄── z-тест ◄── растеризация (барицентрические координаты)
```

## Сборка и запуск

**Visual Studio:** открыть `SoftRenderer.sln` (или создать пустой проект C++ и
добавить все `.h`/`.cpp`), выбрать конфигурацию *Release x64*, запустить (Ctrl+F5).

**CMake:**
```
cmake -B build && cmake --build build --config Release
```

Положите `african_head.obj` и `african_head_diffuse.tga` в папку `assets/`
проекта или передайте пути аргументами:
```
SoftRenderer.exe путь/к/model.obj [путь/к/texture.tga]
```

Программа сохраняет одну картинку 800×800 — `output.tga` и её копию `output.bmp`
(для просмотра в Windows): модель с текстурой, освещённая по Фонгу.

## Результаты

<!-- Сохраните output.bmp как PNG в папку images/ под именем result.png -->
![Результат: Фонг + текстура](images/result.png)

## Источники
- Лекция «OpenGL at Home»; цикл статей haqreu на Хабре: https://habr.com/ru/articles/248153/
