cmake_minimum_required (VERSION 3.2.2)
project (cpplinq)

set (CPPLINQ_VERSION "20150908")

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/CppLinq DESTINATION include
        FILES_MATCHING PATTERN "*.hpp")


include(CMakePackageConfigHelpers)
set(INCLUDE_INSTALL_DIR include/)
configure_package_config_file(
  ${cpplinq_SOURCE_DIR}/cmake/modules/${PROJECT_NAME}-config.cmake.in
  ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
  INSTALL_DESTINATION lib/cmake/${PROJECT_NAME}-${PRE_VERSION}
  PATH_VARS INCLUDE_INSTALL_DIR)

write_basic_package_version_file(
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
    VERSION ${CPPLINQ_VERSION}
    COMPATIBILITY AnyNewerVersion)

install(FILES
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
    DESTINATION lib/cmake/${PROJECT_NAME}-${CPPLINQ_VERSION})
