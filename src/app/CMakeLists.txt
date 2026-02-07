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
      "${_third_party_android_prefix}/zlib/lib/libz.so"
      "${_third_party_android_prefix}/freetype/lib/libfreetype.so"
      "${_third_party_android_prefix}/libjpeg-turbo/lib/libjpeg.so"
      "${_third_party_android_prefix}/libpng/lib/libpng16.so"
      "${_third_party_android_prefix}/poppler/lib/libpoppler.so"
      "${_third_party_android_prefix}/poppler/lib/libpoppler-qt6.so"
      "${_third_party_android_prefix}/djvulibre/lib/libdjvulibre.so"
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

    set(_djvu_bin_dir "${_third_party_android_prefix}/djvulibre/bin")
    if (EXISTS "${_djvu_bin_dir}/ddjvu" AND EXISTS "${_djvu_bin_dir}/djvused" AND EXISTS "${_djvu_bin_dir}/djvutxt")
      set(_djvu_ddjvu "${_djvu_bin_dir}/ddjvu")
      set(_djvu_djvused "${_djvu_bin_dir}/djvused")
      set(_djvu_djvutxt "${_djvu_bin_dir}/djvutxt")
      set_source_files_properties("${_djvu_ddjvu}" PROPERTIES QT_RESOURCE_ALIAS "ddjvu")
      set_source_files_properties("${_djvu_djvused}" PROPERTIES QT_RESOURCE_ALIAS "djvused")
      set_source_files_properties("${_djvu_djvutxt}" PROPERTIES QT_RESOURCE_ALIAS "djvutxt")
      qt_add_resources(ereader "djvutools"
        PREFIX "/djvulibre/bin"
        FILES "${_djvu_ddjvu}" "${_djvu_djvused}" "${_djvu_djvutxt}"
      )
    endif()
  endif()
endif()

qt_finalize_target(ereader)

if (ANDROID)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)
  set(_deploy_json "${CMAKE_CURRENT_BINARY_DIR}/android-ereader-deployment-settings.json")
  add_custom_command(TARGET ereader POST_BUILD
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/scripts/android/patch_deployment_json.py" "${_deploy_json}"
    COMMENT "Patching Android deployment settings JSON"
  )
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
