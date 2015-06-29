# Add executable called "validate" that is built from the source files
# "validate.c". The extensions are automatically found.

find_package(Threads)
find_package(lrt)
add_executable (validate validate.c)
add_executable (receiver receiver.c)
add_executable (sender sender.c)
add_executable (server server.c)

# Link the executable to the library. Since the library has public
# include directories we will use those link directories when building
target_link_libraries (validate LINK_PUBLIC list)
target_link_libraries (receiver "${CMAKE_THREAD_LIBS_INIT}")
target_link_libraries (receiver LINK_PUBLIC value)
target_link_libraries (server "${CMAKE_THREAD_LIBS_INIT}")
target_link_libraries (server -lrt)
target_link_libraries (server LINK_PUBLIC value)
target_link_libraries (sender "${CMAKE_THREAD_LIBS_INIT}")
target_link_libraries (sender -lrt)
target_link_libraries (sender LINK_PUBLIC value)
