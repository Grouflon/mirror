
# This script is intended to be used by a parent CMakeList.txt located somewhere else

# Declare the source files
set(MIRROR_SOURCES
  "${CMAKE_CURRENT_LIST_DIR}/mirror_base.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/mirror_types.cpp"
  "${CMAKE_CURRENT_LIST_DIR}/tools/BinarySerializer.cpp"
)

# Declare the header files
set(MIRROR_HEADERS
  "${CMAKE_CURRENT_LIST_DIR}/mirror.h"
  "${CMAKE_CURRENT_LIST_DIR}/mirror_base.h"
  "${CMAKE_CURRENT_LIST_DIR}/mirror_std.h"
  "${CMAKE_CURRENT_LIST_DIR}/mirror_macros.h"
  "${CMAKE_CURRENT_LIST_DIR}/mirror_types.h"
  "${CMAKE_CURRENT_LIST_DIR}/tools/BinarySerializer.h"
)

# Declare the include directories
set(MIRROR_INCLUDE_DIRS
  "${CMAKE_CURRENT_LIST_DIR}/"
)

# Message the user will see configuring his cmake project.
message("Mirror (mirror-config.cmake) script read, use MIRROR_SOURCES and MIRROR_INCLUDE_DIRS."  )
