#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <cstring>
#include <stb_image.h>
#include <stb_image_write.h>
#include "dort/image.hpp"

namespace dort {
  Image<PixelRgb8> read_image(FILE* input) {
    int x_res, y_res;
    uint8_t* data = stbi_load_from_file(input, &x_res, &y_res, 0, 3);

    assert(data);
    assert(x_res >= 0 && y_res >= 0);
    Image<PixelRgb8> img(x_res, y_res);
    std::memcpy(img.storage.data(), data, 3 * img.x_res * img.y_res);

    stbi_image_free(data);
    return img;
  }

  void write_image_png(FILE* output, const Image<PixelRgb8>& img) {
    auto write = [](void* file, void* data, int size) {
      std::fwrite(data, size, 1, (FILE*)file);
    };
    int result = stbi_write_png_to_func(write, output, 
          img.x_res, img.y_res, 3, img.storage.data(), 0);
    if(result == 0) {
      throw std::runtime_error("Error writing PNG (stbi_write_png_to_func)");
    }
  }

  void write_image_ppm(FILE* output, const Image<PixelRgb8>& img) {
    std::fprintf(output, "P6 %u %u 255\n", img.x_res, img.y_res);
    std::fwrite(img.storage.data(), 3, img.x_res * img.y_res, output);
  }

  void write_image_rgbe(FILE* output, const Image<PixelRgbFloat>& img) {
    auto write = [](void* file, void* data, int size) {
      std::fwrite(data, size, 1, (FILE*)file);
    };
    int result = stbi_write_hdr_to_func(write, output, 
          img.x_res, img.y_res, 3, img.storage.data());
    if(result == 0) {
      throw std::runtime_error("Error writing RGBE (stbi_write_hdr_to_func)");
    }
  }
}
