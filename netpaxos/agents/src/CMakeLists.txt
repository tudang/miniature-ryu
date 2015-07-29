# Add executable called "validate" that is built from the source files
# "validate.c". The extensions are automatically found.

find_package(Threads)
find_package(lrt)
add_executable (receiver receiver.c)
add_executable (sender sender.c)
add_executable (server server.c)
add_executable (storage storage.c)

# Link the executable to the library. Since the library has public
# include directories we will use those link directories when building
target_link_libraries (receiver LINK_PUBLIC value "${CMAKE_THREAD_LIBS_INIT}")
target_link_libraries (server LINK_PUBLIC value -lrt "${CMAKE_THREAD_LIBS_INIT}")
target_link_libraries (sender LINK_PUBLIC value -lrt "${CMAKE_THREAD_LIBS_INIT}")
target_link_libraries (storage LINK_PUBLIC value "${CMAKE_THREAD_LIBS_INIT}")
