#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "simplegl::simplegl" for configuration "Release"
set_property(TARGET simplegl::simplegl APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(simplegl::simplegl PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/simplegl.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/simplegl.dll"
  )

list(APPEND _cmake_import_check_targets simplegl::simplegl )
list(APPEND _cmake_import_check_files_for_simplegl::simplegl "${_IMPORT_PREFIX}/lib/simplegl.lib" "${_IMPORT_PREFIX}/bin/simplegl.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
