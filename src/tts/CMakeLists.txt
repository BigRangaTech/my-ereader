add_library(tts STATIC
  TtsController.cpp
)

target_include_directories(tts PUBLIC include)

target_link_libraries(tts PUBLIC Qt6::Core)

if (Qt6TextToSpeech_FOUND)
  target_link_libraries(tts PUBLIC Qt6::TextToSpeech)
  target_compile_definitions(tts PUBLIC HAVE_QT_TTS=1)
endif()
