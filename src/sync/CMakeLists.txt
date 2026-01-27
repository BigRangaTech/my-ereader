add_library(sync STATIC
  SyncManager.cpp
)

target_include_directories(sync PUBLIC include)

target_link_libraries(sync PUBLIC Qt6::Core Qt6::Network)
