#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "mapfmt::mapc" for configuration "Release"
set_property(TARGET mapfmt::mapc APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(mapfmt::mapc PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/bin/mapc.exe"
  )

list(APPEND _cmake_import_check_targets mapfmt::mapc )
list(APPEND _cmake_import_check_files_for_mapfmt::mapc "${_IMPORT_PREFIX}/bin/mapc.exe" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
