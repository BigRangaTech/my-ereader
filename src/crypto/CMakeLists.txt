add_library(crypto STATIC
  CryptoVault.cpp
  CryptoBackendNull.cpp
  CryptoBackendMonocypher.cpp
)

target_include_directories(crypto PUBLIC include)

target_link_libraries(crypto PUBLIC Qt6::Core)

option(ENABLE_MONOCYPHER "Enable vendored Monocypher backend" ON)
if (ENABLE_MONOCYPHER)
  target_sources(crypto PRIVATE ../../third_party/monocypher/monocypher.c)
  target_include_directories(crypto PRIVATE ../../third_party/monocypher)
  target_compile_definitions(crypto PUBLIC HAVE_MONOCYPHER=1)
endif()

option(ENABLE_LIBSODIUM "Enable libsodium crypto backend" ON)
if (ENABLE_LIBSODIUM)
  find_package(PkgConfig QUIET)
  if (PkgConfig_FOUND)
    pkg_check_modules(LIBSODIUM QUIET libsodium)
  endif()
  if (LIBSODIUM_FOUND)
    target_sources(crypto PRIVATE CryptoBackendSodium.cpp)
    target_compile_definitions(crypto PUBLIC HAVE_LIBSODIUM=1)
    target_include_directories(crypto PRIVATE ${LIBSODIUM_INCLUDE_DIRS})
    target_link_libraries(crypto PRIVATE ${LIBSODIUM_LIBRARIES})
  endif()
endif()
