qt_add_executable(ereader
  MANUAL_FINALIZATION
  main.cpp
)

qt_add_qml_module(ereader
  URI Ereader
  VERSION 1.0
  QML_FILES
    qml/Main.qml
    qml/MainAndroid.qml
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

if (NOT ANDROID)
  target_link_libraries(ereader PRIVATE Qt6::Widgets)
endif()

if (POPPLER_QT6_AVAILABLE)
  target_link_libraries(ereader PRIVATE Poppler::Qt6)
endif()

if (ANDROID)
  set_target_properties(ereader PROPERTIES
    QT_ANDROID_PACKAGE_NAME "com.bigrangatech.myereader"
  )

  set(_android_abi "${ANDROID_ABI}")
  if (NOT _android_abi)
    set(_android_abi "${CMAKE_ANDROID_ARCH_ABI}")
  endif()

  if (_android_abi)
    set(_third_party_android_prefix "${CMAKE_SOURCE_DIR}/third_party/install/android/${_android_abi}")
    set(_android_extra_libs "")
    foreach(libpath
      "${_third_party_android_prefix}/libxml2/lib/libxml2.so"
      "${_third_party_android_prefix}/libarchive/lib/libarchive.so"
      "${_third_party_android_prefix}/sqlite/lib/libsqlite3.so"
    )
      if (EXISTS "${libpath}")
        list(APPEND _android_extra_libs "${libpath}")
      endif()
    endforeach()

    if (_android_extra_libs)
      set_target_properties(ereader PROPERTIES
        QT_ANDROID_EXTRA_LIBS "${_android_extra_libs}"
      )
    endif()
  endif()
endif()

qt_finalize_target(ereader)

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
