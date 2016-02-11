#include <cstdio>
#include <cstdint>
#include <random>
#include <chrono>

struct Vec4 {
  float xs[4];

  float operator[](uint32_t i) const { return this->xs[i]; }
  float& operator[](uint32_t i) { return this->xs[i]; }
};

Vec4 operator+(const Vec4& v1, const Vec4& v2) {
  return Vec4 {{ v1[0] + v2[0], v1[1] + v2[1], v1[2] + v2[2], v1[3] + v2[3] }};
}

Vec4 operator*(float a, const Vec4& v) {
  return Vec4 {{ a * v[0], a * v[1], a * v[2], a * v[3] }};
}

float dot(const Vec4& v1, const Vec4& v2) {
  return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2] + v1[3] * v2[3];
}

struct MatRows {
  Vec4 rows[4];
};

Vec4 operator*(const MatRows& m, const Vec4& v) {
  return Vec4 {{
      dot(m.rows[0], v),
      dot(m.rows[1], v),
      dot(m.rows[2], v),
      dot(m.rows[3], v)
    }};
}

struct MatCols {
  Vec4 cols[4];
};

Vec4 operator*(const MatCols& m, const Vec4& v) {
  return v[0] * m.cols[0] + v[1] * m.cols[1] + v[2] * m.cols[2] + v[3] * m.cols[3];
}

int main() {
  std::mt19937 rng(42);
  std::uniform_real_distribution<float> dis(-10.f, 10.f);

  MatRows mat_rows;
  MatCols mat_cols;
  for(uint32_t row = 0; row < 4; ++row) {
    for(uint32_t col = 0; col < 4; ++col) {
      mat_rows.rows[row][col] = mat_cols.cols[col][row] = dis(rng);
    }
  }

  std::vector<Vec4> vecs;
  for(uint32_t i = 0; i < 8; ++i) {
    vecs.push_back(Vec4 {{ dis(rng), dis(rng), dis(rng), dis(rng) }});
  }

  uint32_t count = 1000000;

  auto blackhole = [](const Vec4& v) { 
    volatile float x; x = v[0]; x = v[1]; x = v[2]; x = v[3];
    (void)x;
  };
  using clock = std::chrono::high_resolution_clock;

  auto t1 = clock::now();
  for(uint32_t i = 0; i < count; ++i) {
    for(auto& vec: vecs) {
      blackhole(mat_rows * vec);
    }
  }

  auto t2 = clock::now();
  for(uint32_t i = 0; i < count; ++i) {
    for(auto& vec: vecs) {
      blackhole(mat_cols * vec);
    }
  }

  auto t3 = clock::now();

  auto rows_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
  auto cols_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t3 - t2).count();
  std::printf("rows %lu ns\ncols %lu ns\n", rows_ns, cols_ns);
  return 0;
}
