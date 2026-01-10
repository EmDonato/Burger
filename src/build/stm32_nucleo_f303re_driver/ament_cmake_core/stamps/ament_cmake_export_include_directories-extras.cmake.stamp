# generated from ament_cmake_export_include_directories/cmake/ament_cmake_export_include_directories-extras.cmake.in

set(_exported_include_dirs "${stm32_nucleo_f303re_driver_DIR}/../../../include/stm32_nucleo_f303re_driver")

# append include directories to stm32_nucleo_f303re_driver_INCLUDE_DIRS
# warn about not existing paths
if(NOT _exported_include_dirs STREQUAL "")
  find_package(ament_cmake_core QUIET REQUIRED)
  foreach(_exported_include_dir ${_exported_include_dirs})
    if(NOT IS_DIRECTORY "${_exported_include_dir}")
      message(WARNING "Package 'stm32_nucleo_f303re_driver' exports the include directory '${_exported_include_dir}' which doesn't exist")
    endif()
    normalize_path(_exported_include_dir "${_exported_include_dir}")
    list(APPEND stm32_nucleo_f303re_driver_INCLUDE_DIRS "${_exported_include_dir}")
  endforeach()
endif()
