cmake_minimum_required (VERSION 2.6)
project (Netpaxos)
# The version number
set (Netpaxos_VERSION_MAJOR 1)
set (Netpaxos_VERSION_MINOR 0)

include_directories("${PROJECT_SOURCE_DIR}/netpaxos")
add_subdirectory(netpaxos)
set (EXTRA_LIBS ${EXTRA_LIBS} netpaxos)

# add the executable
add_executable(replica replica.c)
target_link_libraries (replica ${EXTRA_LIBS})
