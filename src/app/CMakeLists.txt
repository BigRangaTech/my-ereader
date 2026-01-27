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

target_link_libraries(ereader PRIVATE
  Qt6::Core
  Qt6::Quick
  Qt6::Qml
  Qt6::Network
  Qt6::Sql
  core
  formats
  sync
  tts
)
