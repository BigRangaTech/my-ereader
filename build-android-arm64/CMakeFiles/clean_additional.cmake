# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "src/app/CMakeFiles/ereader_autogen.dir/AutogenUsed.txt"
  "src/app/CMakeFiles/ereader_autogen.dir/ParseCache.txt"
  "src/app/ereader_autogen"
  "src/core/CMakeFiles/core_autogen.dir/AutogenUsed.txt"
  "src/core/CMakeFiles/core_autogen.dir/ParseCache.txt"
  "src/core/core_autogen"
  "src/crypto/CMakeFiles/crypto_autogen.dir/AutogenUsed.txt"
  "src/crypto/CMakeFiles/crypto_autogen.dir/ParseCache.txt"
  "src/crypto/crypto_autogen"
  "src/formats/CMakeFiles/formats_autogen.dir/AutogenUsed.txt"
  "src/formats/CMakeFiles/formats_autogen.dir/ParseCache.txt"
  "src/formats/formats_autogen"
  "src/sync/CMakeFiles/sync_autogen.dir/AutogenUsed.txt"
  "src/sync/CMakeFiles/sync_autogen.dir/ParseCache.txt"
  "src/sync/sync_autogen"
  "src/tts/CMakeFiles/tts_autogen.dir/AutogenUsed.txt"
  "src/tts/CMakeFiles/tts_autogen.dir/ParseCache.txt"
  "src/tts/tts_autogen"
  )
endif()
