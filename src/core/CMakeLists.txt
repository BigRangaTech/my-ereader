add_library(core STATIC
  AnnotationModel.cpp
  LibraryModel.cpp
  Logger.cpp
  LicenseManager.cpp
  ReaderController.cpp
  VaultController.cpp
  include/AnnotationModel.h
  include/LibraryModel.h
  include/Logger.h
  include/LicenseManager.h
  include/ReaderController.h
  include/VaultController.h
)

target_include_directories(core PUBLIC include)

target_link_libraries(core PUBLIC Qt6::Core Qt6::Sql formats crypto SQLite::SQLite3)
