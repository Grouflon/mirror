
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
  "${CMAKE_CURRENT_LIST_DIR}/tools/BinarySerializer.cpp"
)

# Declare the include directories
set(MIRROR_INCLUDE_DIRS
  "${CMAKE_CURRENT_LIST_DIR}/"
  "${CMAKE_CURRENT_LIST_DIR}/tools/"
)

# Message the user will see configuring his cmake project.
message("Mirror (mirror-config.cmake) script read, use MIRROR_SOURCES and MIRROR_INCLUDE_DIRS."  )
