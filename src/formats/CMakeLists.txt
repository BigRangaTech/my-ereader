add_library(formats STATIC
  FormatRegistry.cpp
  TxtProvider.cpp
)

target_include_directories(formats PUBLIC include)

target_link_libraries(formats PUBLIC Qt6::Core)
