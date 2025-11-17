#include "camera.hpp"
#include "gtest/gtest.h"
#include "window.hpp"

// Window is required to create a camera (for rendering purposes)
woop::Window window{woop::WindowConfig{
    .resolution = {16, 9},
    .decorated = false,
}};

bool close_enough(float val, float target) {
  constexpr float epsilon = 0.01f;
  return abs(val - target) < epsilon;
}

void expect_vec_eq(const glm::vec4& val, const glm::vec4& target) {
  EXPECT_TRUE(close_enough(val.x, target.x));
  EXPECT_TRUE(close_enough(val.y, target.y));
  EXPECT_TRUE(close_enough(val.z, target.z));
  EXPECT_TRUE(close_enough(val.w, target.w));
}

struct TransformData {
  glm::vec3 world;
  glm::vec4 clip;
};

TEST(Cameras, Transformations) {
  woop::CameraConfig cfg = {
      .position = {5, 3, 8},
      .rotation = 225.0f,
      .near_plane = 0.1f,
      .far_plane = 100.0f,
      .fov = 60.0f,
  };
  woop::Camera camera = {window, cfg};
  // Collection of points and their images under the transformation
  std::vector<TransformData> data = {
      {
          .world = {0, 0, 0},
          .clip = {-2.0668, -5.1962, -9.4110, -9.1924},
      },
      {
          .world = {2, 1, 2},
          .clip = {-2.0668, -3.4641, -6.5769, -6.3640},
      },
      {
          .world = {-3, 2, -3},
          .clip = {-2.0668, -1.7321, -13.6621, -13.4350},
      },
      {
          .world = {-2, 3, 1},
          .clip = {0.000000, 0.000000, -10.119514, -9.899495},
      },
  };

  for (const auto& set : data) {
    glm::vec4 clip = camera.world_to_clip(set.world);
    expect_vec_eq(clip, set.clip);
  }
}

TEST(Cameras, Visibility) {
  woop::Camera camera = {window};
  EXPECT_FALSE(camera.is_visible(glm::vec4{-1.1f, 0.0f, 0.0f, 1.0f}));
  EXPECT_FALSE(camera.is_visible(glm::vec4{1.1f, 0.0f, 0.0f, 1.0f}));
  EXPECT_FALSE(camera.is_visible(glm::vec4{0.0f, -1.1f, 0.0f, 1.0f}));
  EXPECT_FALSE(camera.is_visible(glm::vec4{0.0f, 1.1f, 0.0f, 1.0f}));
  EXPECT_FALSE(camera.is_visible(glm::vec4{0.0f, 0.0f, -1.1f, 1.0f}));
  EXPECT_FALSE(camera.is_visible(glm::vec4{0.0f, 0.0f, 1.1f, 1.0f}));
  EXPECT_TRUE(camera.is_visible(glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}));
}