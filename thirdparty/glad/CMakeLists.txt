add_library(glad STATIC
  src/glad.c
)

target_include_directories(glad PRIVATE 
  include
)
# Suppress compiler warnings for targets that link to this library
target_include_directories(glad SYSTEM INTERFACE 
  include
)