# This file is part of the ArgusCore Project. See AUTHORS file for Copyright information
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

########### bnetserver ###############

CollectSourceFiles(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PRIVATE_SOURCES
  # Exclude
  ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders)

if (WIN32)
  if (MSVC)
    list(APPEND PRIVATE_SOURCES bnetserver.rc)
  endif()
endif()

if (USE_COREPCH)
  set(PRIVATE_PCH_HEADER PrecompiledHeaders/bnetPCH.h)
endif()

GroupSources(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(bnetserver
  ${PRIVATE_SOURCES}
)

# The config file is always looked up as a relative path — configs/ sits
# next to the executable on both Windows and Linux. On Linux the working
# directory when starting the server should be the installation prefix
# (or the build output folder during development). The -c flag on the
# command line overrides this at runtime if an absolute path is needed.
target_compile_definitions(bnetserver PRIVATE
  _TRINITY_BNET_CONFIG="configs/bnetserver.conf"
  _TRINITY_BNET_CONFIG_DIR="configs/bnetserver.conf.d"
)

target_link_libraries(bnetserver
  PRIVATE
    trinity-core-interface
  PUBLIC
    shared)

CollectIncludeDirectories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC_INCLUDES
  # Exclude
  ${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders)

target_include_directories(bnetserver
  PUBLIC
    ${PUBLIC_INCLUDES}
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})

set_target_properties(bnetserver
  PROPERTIES
    COMPILE_WARNING_AS_ERROR ${WITH_WARNINGS_AS_ERRORS}
    FOLDER "server")

add_custom_command(TARGET bnetserver
  POST_BUILD
  # Create the certs/ subfolder first — cmake -E copy does not create
  # parent directories, so we have to make it ourselves.
  COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:bnetserver>/certs"
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/bnetserver.cert.pem
    "$<TARGET_FILE_DIR:bnetserver>/certs/bnetserver.cert.pem"
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/bnetserver.key.pem
    "$<TARGET_FILE_DIR:bnetserver>/certs/bnetserver.key.pem"
)

if (UNIX)
  install(TARGETS bnetserver DESTINATION bin)
  if (COPY_CONF)
    add_custom_command(TARGET bnetserver
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:bnetserver>/configs"
      COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/bnetserver.conf.dist
        "$<TARGET_FILE_DIR:bnetserver>/configs/bnetserver.conf.dist"
    )
    install(FILES bnetserver.conf.dist DESTINATION ${CONF_DIR})
  endif()
  install(FILES bnetserver.cert.pem bnetserver.key.pem DESTINATION certs)
elseif (WIN32)
  install(TARGETS bnetserver DESTINATION "${CMAKE_INSTALL_PREFIX}")
  if (COPY_CONF)
    add_custom_command(TARGET bnetserver
      POST_BUILD
      # Create the configs/ subfolder first — cmake -E copy does not create
      # parent directories, so we have to make it ourselves.
      COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:bnetserver>/configs"
      COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/bnetserver.conf.dist
        "$<TARGET_FILE_DIR:bnetserver>/configs/bnetserver.conf.dist"
    )
    install(FILES bnetserver.conf.dist DESTINATION "${CMAKE_INSTALL_PREFIX}/configs")
  endif()
  install(FILES bnetserver.cert.pem bnetserver.key.pem DESTINATION "${CMAKE_INSTALL_PREFIX}/certs")
endif()

# Copy required runtime DLLs (OpenSSL, MySQL/MariaDB) next to the executable
# so the server can be launched directly from the Visual Studio build output
# folder without having to run cmake --install first.
# The function is a no-op on Linux/macOS where rpath handles this.
include(CopyRuntimeDependencies)
copy_runtime_dependencies(bnetserver NEEDS_MYSQL)

# Generate precompiled header
if (USE_COREPCH)
  add_cxx_pch(bnetserver ${PRIVATE_PCH_HEADER})
endif()
