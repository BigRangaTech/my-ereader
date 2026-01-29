add_library(formats STATIC
  FormatRegistry.cpp
  EpubProvider.cpp
  MobiProvider.cpp
  Fb2Provider.cpp
  CbzProvider.cpp
  PdfProvider.cpp
  DjvuProvider.cpp
  TxtProvider.cpp
  EpubProvider.h
  MobiProvider.h
  Fb2Provider.h
  CbzProvider.h
  PdfProvider.h
  DjvuProvider.h
)

target_include_directories(formats PUBLIC include)

target_link_libraries(formats PUBLIC Qt6::Core Qt6::Gui Qt6::Xml)

find_package(ZLIB REQUIRED)

target_sources(formats PRIVATE
  ../../third_party/miniz/miniz.c
  ../../third_party/miniz/miniz_tinfl.c
  ../../third_party/miniz/miniz_tdef.c
  ../../third_party/miniz/miniz_zip.c
)
target_include_directories(formats PRIVATE ../../third_party/miniz)

set(LIBMOBI_SOURCES
  ../../third_party/libmobi/src/buffer.c
  ../../third_party/libmobi/src/compression.c
  ../../third_party/libmobi/src/debug.c
  ../../third_party/libmobi/src/encryption.c
  ../../third_party/libmobi/src/index.c
  ../../third_party/libmobi/src/memory.c
  ../../third_party/libmobi/src/meta.c
  ../../third_party/libmobi/src/opf.c
  ../../third_party/libmobi/src/parse_rawml.c
  ../../third_party/libmobi/src/randombytes.c
  ../../third_party/libmobi/src/read.c
  ../../third_party/libmobi/src/sha1.c
  ../../third_party/libmobi/src/structure.c
  ../../third_party/libmobi/src/util.c
  ../../third_party/libmobi/src/write.c
  ../../third_party/libmobi/src/xmlwriter.c
)

target_sources(formats PRIVATE ${LIBMOBI_SOURCES})
target_include_directories(formats PRIVATE ../../third_party/libmobi/src)
target_link_libraries(formats PUBLIC ZLIB::ZLIB)

if (POPPLER_QT6_AVAILABLE)
  target_link_libraries(formats PUBLIC Poppler::Qt6)
endif()

if (LIBARCHIVE_AVAILABLE)
  target_link_libraries(formats PUBLIC Archive::Archive)
endif()
