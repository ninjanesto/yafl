project(Yafl)

add_library(${PROJECT_NAME} INTERFACE)
add_library(Yafl::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)

# Installation
set(INSTALL_INCLUDE_DIR include)
set(INSTALL_LIB_DIR lib)
set(INSTALL_CMAKE_DIR lib/cmake/${PROJECT_NAME})

# Install the library targets
install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}-targets
        ARCHIVE DESTINATION ${INSTALL_LIB_DIR}   # Static library
        LIBRARY DESTINATION ${INSTALL_LIB_DIR})  # Shared library

# Install header files
install(DIRECTORY .
        DESTINATION ${INSTALL_INCLUDE_DIR}
        FILES_MATCHING PATTERN "*.h")

# Install the CMake package configuration files
install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}-targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${INSTALL_CMAKE_DIR})

# Generate package configuration and version files
include(CMakePackageConfigHelpers)
configure_package_config_file(
        "${CMAKE_SOURCE_DIR}/cmake/${PROJECT_NAME}.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION ${INSTALL_CMAKE_DIR})

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/YaflConfigVersion.cmake"
        VERSION ${YetAnotherFunctionalLib_VERSION}
        COMPATIBILITY SameMajorVersion
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${INSTALL_CMAKE_DIR})
