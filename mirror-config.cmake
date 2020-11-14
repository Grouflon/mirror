
# This script is intended to be used by a parent CMakeList.txt located somewhere else

# Declare the source files
file(GLOB_RECURSE MIRROR_SOURCES
  "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)

# Declare the header files
file(GLOB_RECURSE MIRROR_HEADERS
  "${CMAKE_CURRENT_LIST_DIR}/*.h"
)

# Declare the include directories
set(MIRROR_INCLUDE_DIRS
  "${CMAKE_CURRENT_LIST_DIR}/"
)

# Message the user will see configuring his cmake project.
message("Mirror (mirror-config.cmake) script read, use MIRROR_SOURCES, MIRROR_HEADERS and MIRROR_INCLUDE_DIRS."  )
