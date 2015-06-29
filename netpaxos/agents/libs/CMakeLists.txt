# Create a library called "mylist" which includes the source file "mylist.c".
# The extension is already found. Any number of sources could be listed here.
add_library (list list.c)
add_library (value value.c)

# Make sure the compiler can find include files for our mylist library
# when other libraries or executables link to mylist
target_include_directories (list PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories (value PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
