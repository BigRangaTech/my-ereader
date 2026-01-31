add_library(sync STATIC
  SyncManager.cpp
  include/SyncManager.h
)

target_include_directories(sync PUBLIC include ../core/include)

target_link_libraries(sync PUBLIC Qt6::Core Qt6::Network)
