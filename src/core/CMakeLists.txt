add_library(core STATIC
  LibraryModel.cpp
  ReaderController.cpp
)

target_include_directories(core PUBLIC include)

target_link_libraries(core PUBLIC Qt6::Core Qt6::Sql formats)
