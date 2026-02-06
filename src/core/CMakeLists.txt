add_library(core STATIC
  AsyncUtil.cpp
  DbWorker.cpp
  AnnotationModel.cpp
  KeychainStore.cpp
  LibraryModel.cpp
  Logger.cpp
  LicenseManager.cpp
  ReaderController.cpp
  SettingsManager.cpp
  UpdateManager.cpp
  VaultController.cpp
  include/AsyncUtil.h
  include/DbWorker.h
  include/AnnotationModel.h
  include/KeychainStore.h
  include/LibraryModel.h
  include/Logger.h
  include/LicenseManager.h
  include/ReaderController.h
  include/SettingsManager.h
  include/UpdateManager.h
  include/VaultController.h
)

target_include_directories(core PUBLIC include)

target_link_libraries(core PUBLIC Qt6::Core Qt6::Sql formats crypto SQLite::SQLite3)

if (TARGET Qt6::DBus)
  target_link_libraries(core PUBLIC Qt6::DBus)
endif()

option(ENABLE_LIBSECRET "Enable libsecret keychain storage" ON)
if (ENABLE_LIBSECRET)
  find_package(PkgConfig QUIET)
  if (PkgConfig_FOUND)
    pkg_check_modules(LIBSECRET QUIET libsecret-1)
  endif()
  if (LIBSECRET_FOUND)
    target_compile_definitions(core PUBLIC HAVE_LIBSECRET=1)
    target_include_directories(core PRIVATE ${LIBSECRET_INCLUDE_DIRS})
    target_link_libraries(core PRIVATE ${LIBSECRET_LIBRARIES})
  endif()
endif()
