#include "camera.hpp"
#include "glm/fwd.hpp"
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

template <typename T>
void expect_vec_eq(const T& val, const T& target) {}
template <>
void expect_vec_eq(const glm::vec4& val, const glm::vec4& target) {
  EXPECT_TRUE(close_enough(val.x, target.x));
  EXPECT_TRUE(close_enough(val.y, target.y));
  EXPECT_TRUE(close_enough(val.z, target.z));
  EXPECT_TRUE(close_enough(val.w, target.w));
}
template <>
void expect_vec_eq(const glm::vec3& val, const glm::vec3& target) {
  EXPECT_TRUE(close_enough(val.x, target.x));
  EXPECT_TRUE(close_enough(val.y, target.y));
  EXPECT_TRUE(close_enough(val.z, target.z));
}
template <>
void expect_vec_eq(const glm::vec2& val, const glm::vec2& target) {
  EXPECT_TRUE(close_enough(val.x, target.x));
  EXPECT_TRUE(close_enough(val.y, target.y));
}

struct ClipData {
  glm::vec3 world;
  glm::vec4 clip;
};

TEST(Cameras, WorldToClip) {
  woop::CameraConfig cfg = {
      .position = {5, 3, 8},
      .rotation = 225.0f,
      .near_plane = 0.1f,
      .far_plane = 100.0f,
      .fov = 60.0f,
  };
  woop::Camera camera = {window, cfg};
  // Collection of points and their images under the transformation
  std::vector<ClipData> data = {
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

TEST(Cameras, ClipToNDC) {
  woop::Camera camera = {window};

  // Visible point
  {
    glm::vec4 clip = {1.0f, 1.0f, 1.0f, 1.0f};
    std::optional<glm::vec3> result = camera.clip_to_ndc(clip);
    EXPECT_TRUE(result);
    expect_vec_eq(result.value(), {1.0f, 1.0f, 1.0f});
  }
  // Visible point
  {
    glm::vec4 clip = {0.5f, 0.5f, 1.0f, 2.0f};
    std::optional<glm::vec3> result = camera.clip_to_ndc(clip);
    EXPECT_TRUE(result);
    expect_vec_eq(result.value(), {0.25f, 0.25f, 0.5f});
  }
  // Not visible point (w=0)
  {
    glm::vec4 clip = {1.0f, 1.0f, 1.0f, 0.0f};
    std::optional<glm::vec3> result = camera.clip_to_ndc(clip);
    EXPECT_FALSE(result);
  }
  // Not visible point (should still transform okay)
  {
    glm::vec4 clip = {3.0f, 0.5f, 1.0f, 0.5f};
    std::optional<glm::vec3> result = camera.clip_to_ndc(clip);
    EXPECT_TRUE(result);
    expect_vec_eq(result.value(), {6.0f, 1.0f, 2.0f});
  }
}

TEST(Cameras, NDCToScreen) {
  woop::Camera camera = {window};

  // Visible
  {
    glm::vec3 ndc = {1.0f, 1.0f, 0.1f};
    glm::ivec2 screen = camera.ndc_to_screen(ndc);
    glm::ivec2 target = window.get_resolution();
    EXPECT_EQ(screen.x, target.x);
    EXPECT_EQ(screen.y, target.y);
  }
  // Visible
  {
    glm::vec3 ndc = {-0.25f, 0.5f, 1.0f};
    glm::ivec2 screen = camera.ndc_to_screen(ndc);
    glm::ivec2 target = {
        window.get_resolution().x * 0.375f,
        window.get_resolution().y * 0.75f,
    };
    EXPECT_EQ(screen.x, target.x);
    EXPECT_EQ(screen.y, target.y);
  }
  // Not visible (still transforms)
  {
    glm::vec3 ndc = {1.25f, -1.25f, 1.0f};
    glm::ivec2 screen = camera.ndc_to_screen(ndc);
    glm::ivec2 target = {
        window.get_resolution().x * 1.125f,
        window.get_resolution().y * -0.125f,
    };
    EXPECT_EQ(screen.x, target.x);
    EXPECT_EQ(screen.y, target.y);
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