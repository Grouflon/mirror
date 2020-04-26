
# This script is intended to be used by a parent CMakeList.txt located somewhere else
#
# Question: How to use mirror with cmake ?
#
# Answer:
#    1 - copy mirror folder in a subfolder of your project.
#    2 - add "find_package(MIRROR REQUIRED HINTS "./<path-to-mirror-relative-to-your-cmakelists-file>")" in your CMakeLists.txt
#    3 - add ${MIRROR_SOURCES} and  ${MIRROR_INCLUDE_DIRS} in your targets.

# Declare the source files
set(MIRROR_SOURCES
  "${CMAKE_CURRENT_LIST_DIR}/mirror.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/mirror.h"
  "${CMAKE_CURRENT_LIST_DIR}/mirror_std.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/mirror_std.h"
  "${CMAKE_CURRENT_LIST_DIR}/mirror_types.h"
  "${CMAKE_CURRENT_LIST_DIR}/tools/BinarySerializer.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/tools/BinarySerializer.h"
)

# Declare the include directories
set(MIRROR_INCLUDE_DIRS
  "${CMAKE_CURRENT_LIST_DIR}/"
)

# Message the user will see configuring his cmake project.
message("Mirror (mirror-config.cmake) script read, use MIRROR_SOURCES and MIRROR_INCLUDE_DIRS."  )
