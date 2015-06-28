# CMakeLists files in this project can
# refer to the root source directory of the project as ${NETPAXOS_SOURCE_DIR} and
# to the root binary directory of the project as ${NETPAXOS_BINARY_DIR}.
cmake_minimum_required (VERSION 2.8.11)
project (NetPaxos)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Recurse into the "libs" and "src" subdirectories. This does not actually
# cause another cmake executable to run. The same process will walk through
# the project's entire directory structure.
add_subdirectory (libs)
add_subdirectory (src)

