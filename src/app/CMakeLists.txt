qt_add_executable(ereader
  main.cpp
)

qt_add_qml_module(ereader
  URI Ereader
  VERSION 1.0
  QML_FILES
    qml/Main.qml
    qml/Theme.qml
)

set_source_files_properties(qml/Theme.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

target_link_libraries(ereader PRIVATE
  Qt6::Core
  Qt6::Quick
  Qt6::Qml
  Qt6::QuickControls2
  Qt6::Network
  Qt6::Sql
  core
  formats
  sync
  tts
)

if (POPPLER_QT6_AVAILABLE)
  target_link_libraries(ereader PRIVATE Poppler::Qt6)
endif()

if (UNIX AND NOT APPLE)
  set(_rpath "$ORIGIN")
  if (USE_BUNDLED_POPPLER)
    set(_poppler_lib_dir "${CMAKE_SOURCE_DIR}/third_party/install/poppler/lib")
    if (EXISTS "${_poppler_lib_dir}")
      list(APPEND _rpath "${_poppler_lib_dir}")
    endif()
  endif()
  set_target_properties(ereader PROPERTIES
    BUILD_RPATH "${_rpath}"
    INSTALL_RPATH "$ORIGIN"
  )
endif()
