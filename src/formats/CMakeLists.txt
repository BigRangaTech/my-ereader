add_library(formats STATIC
  FormatRegistry.cpp
  EpubProvider.cpp
  PdfProvider.cpp
  TxtProvider.cpp
  EpubProvider.h
  PdfProvider.h
)

target_include_directories(formats PUBLIC include)

target_link_libraries(formats PUBLIC Qt6::Core Qt6::Xml)

target_sources(formats PRIVATE
  ../../third_party/miniz/miniz.c
  ../../third_party/miniz/miniz_tinfl.c
  ../../third_party/miniz/miniz_tdef.c
  ../../third_party/miniz/miniz_zip.c
)
target_include_directories(formats PRIVATE ../../third_party/miniz)

if (PopplerQt6_FOUND)
  target_link_libraries(formats PUBLIC Poppler::Qt6)
endif()
