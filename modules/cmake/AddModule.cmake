# This file is part of the ArgusCore Project. See AUTHORS file for Copyright information
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

# add_argus_module(NAME <name> [SOURCES file1 file2 ...])
#
# Creates a static library for a module and registers it with the build system.
#
# NAME     — PascalCase identifier. The module must expose a C++ function
#            void Add<NAME>Scripts(). Example: NAME "BossEncounters" expects
#            void AddBossEncountersScripts().
#
# SOURCES  — Optional explicit source list. If omitted, all *.cpp and *.h
#            files under src/ are globbed automatically.
#
# After calling this function the module library:
#   - Compiles with core warning/feature flags (trinity-core-interface)
#   - Has access to all game public headers (game-interface)
#   - Is automatically linked into worldserver
#   - Has its script entry point called during server startup
function(add_argus_module)
    cmake_parse_arguments(ARG "" "NAME" "SOURCES" ${ARGN})

    if(NOT ARG_NAME)
        message(FATAL_ERROR "add_argus_module: NAME is required")
    endif()

    if(NOT ARG_SOURCES)
        file(GLOB_RECURSE ARG_SOURCES CONFIGURE_DEPENDS
            "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
    endif()

    if(NOT ARG_SOURCES)
        message(WARNING "add_argus_module(${ARG_NAME}): no source files found under src/")
    endif()

    string(TOLOWER "mod-${ARG_NAME}" _target)

    add_library(${_target} STATIC ${ARG_SOURCES})

    target_link_libraries(${_target}
        PRIVATE
            trinity-core-interface
            game-interface)

    # Register for worldserver linking and script loader generation.
    set_property(GLOBAL APPEND PROPERTY ARGUS_MODULE_LIBRARIES ${_target})
    set_property(GLOBAL APPEND PROPERTY ARGUS_MODULE_NAMES "${ARG_NAME}")

    # Copy conf/*.conf.dist files to configs/modules/ next to the server binary.
    # Gated on COPY_CONF (the same option that controls worldserver.conf.dist copying).
    if(COPY_CONF)
        file(GLOB _conf_files "${CMAKE_CURRENT_SOURCE_DIR}/conf/*.conf.dist")
        if(_conf_files)
            # Post-build: drop files into the build output folder so the server
            # can be started from the build tree without running cmake --install.
            add_custom_command(TARGET worldserver POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                    "$<TARGET_FILE_DIR:worldserver>/configs/modules"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${_conf_files}
                    "$<TARGET_FILE_DIR:worldserver>/configs/modules"
                COMMENT "Copying ${ARG_NAME} config files to build output"
            )

            # Install: land alongside the server's own conf files.
            if(UNIX)
                install(FILES ${_conf_files} DESTINATION "${CONF_DIR}/modules")
            else()
                install(FILES ${_conf_files} DESTINATION "${CMAKE_INSTALL_PREFIX}/configs/modules")
            endif()
        endif()
    endif()
endfunction()
