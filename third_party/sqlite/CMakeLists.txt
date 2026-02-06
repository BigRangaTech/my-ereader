cmake_minimum_required(VERSION 3.16)

project(sqlite3 C)

option(SQLITE_OMIT_LOAD_EXTENSION "Disable loadable extension support" ON)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/sqlite3.c")
  message(FATAL_ERROR "sqlite3.c not found. Place the SQLite amalgamation in third_party/sqlite.")
endif()

add_library(sqlite3 sqlite3.c)
target_include_directories(sqlite3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(sqlite3 PRIVATE SQLITE_THREADSAFE=1)
if(SQLITE_OMIT_LOAD_EXTENSION)
  target_compile_definitions(sqlite3 PRIVATE SQLITE_OMIT_LOAD_EXTENSION)
endif()

set_target_properties(sqlite3 PROPERTIES OUTPUT_NAME sqlite3)

install(TARGETS sqlite3
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(FILES sqlite3.h sqlite3ext.h DESTINATION include OPTIONAL)
