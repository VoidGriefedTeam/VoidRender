#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "VoidCL::VoidCLcompute" for configuration "Release"
set_property(TARGET VoidCL::VoidCLcompute APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(VoidCL::VoidCLcompute PROPERTIES
  IMPORTED_IMPLIB_RELEASE "${_IMPORT_PREFIX}/lib/VoidCLcompute.lib"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/VoidCLcompute.dll"
  )

list(APPEND _cmake_import_check_targets VoidCL::VoidCLcompute )
list(APPEND _cmake_import_check_files_for_VoidCL::VoidCLcompute "${_IMPORT_PREFIX}/lib/VoidCLcompute.lib" "${_IMPORT_PREFIX}/bin/VoidCLcompute.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
