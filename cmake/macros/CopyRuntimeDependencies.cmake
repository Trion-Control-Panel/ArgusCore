# This file is part of the ArgusCore Project. See AUTHORS file for Copyright information
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

# Prevent this file from being processed more than once per configure run.
include_guard(GLOBAL)

# copy_runtime_dependencies(target)
#
# Adds a POST_BUILD step to the given target that copies the required Windows
# runtime DLLs (OpenSSL and MySQL/MariaDB) next to the built executable.
#
# Why POST_BUILD and not install(): install() only runs when you do
# cmake --install. POST_BUILD runs every time you hit Build in Visual Studio,
# so the server is always runnable straight from the output folder without
# any manual DLL copying.
#
# Does nothing on Linux/macOS — shared libraries are found via rpath there.

function(copy_runtime_dependencies target)
    if(NOT WIN32)
        return()
    endif()

    # --- OpenSSL ---
    # Three common layouts depending on how OpenSSL was installed:
    #   Standard Win64 installer : <root>/bin/libssl-3-x64.dll
    #   AppVeyor / some repacks  : <root>/libssl-3-x64.dll (DLLs at root level)
    # We build a candidate list from all available OpenSSL hints and try each
    # candidate directory until we find the DLL.

    set(_ssl_dirs "")

    # Hint 1: OPENSSL_ROOT_DIR — the most explicit, set by the developer or CI.
    if(DEFINED OPENSSL_ROOT_DIR AND NOT OPENSSL_ROOT_DIR STREQUAL "")
        file(TO_CMAKE_PATH "${OPENSSL_ROOT_DIR}" _ossl_root)
        list(APPEND _ssl_dirs "${_ossl_root}/bin" "${_ossl_root}")
    endif()

    # Hint 2: OPENSSL_INCLUDE_DIR — always a plain path like <root>/include.
    # This is the most reliable hint because FindOpenSSL.cmake always sets it
    # to a single directory (never a generator expression or multi-config list).
    # Go up one level from include/ to reach the install root, then check bin/.
    if(DEFINED OPENSSL_INCLUDE_DIR AND NOT OPENSSL_INCLUDE_DIR STREQUAL "")
        get_filename_component(_ossl_root_from_include "${OPENSSL_INCLUDE_DIR}" DIRECTORY)
        list(APPEND _ssl_dirs "${_ossl_root_from_include}/bin" "${_ossl_root_from_include}")
    endif()

    # Hint 3: OPENSSL_SSL_LIBRARY — may be a multi-config list like
    # "optimized;C:/path/libssl.lib;debug;C:/path/libssld.lib" when both
    # Debug and Release libs are present. We iterate the list and skip the
    # CMake keywords ("optimized", "debug", "general") to find the real paths.
    if(DEFINED OPENSSL_SSL_LIBRARY AND NOT OPENSSL_SSL_LIBRARY STREQUAL "")
        foreach(_ossl_lib_entry IN LISTS OPENSSL_SSL_LIBRARY)
            # Skip the CMake link-type keywords that appear in multi-config lists.
            if(_ossl_lib_entry STREQUAL "optimized" OR
               _ossl_lib_entry STREQUAL "debug"     OR
               _ossl_lib_entry STREQUAL "general")
                continue()
            endif()
            # This entry is an actual file path — go up one level to the lib/
            # folder, then one more to the install root, and check bin/.
            get_filename_component(_ossl_lib_dir "${_ossl_lib_entry}" DIRECTORY)
            get_filename_component(_ossl_root_derived "${_ossl_lib_dir}" DIRECTORY)
            list(APPEND _ssl_dirs "${_ossl_root_derived}/bin" "${_ossl_lib_dir}")
        endforeach()
    endif()

    foreach(_dll IN ITEMS libssl-3-x64.dll libcrypto-3-x64.dll)
        set(_found FALSE)
        foreach(_dir IN LISTS _ssl_dirs)
            if(EXISTS "${_dir}/${_dll}")
                add_custom_command(TARGET ${target} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${_dir}/${_dll}"
                        "$<TARGET_FILE_DIR:${target}>/${_dll}"
                    COMMENT "Copying ${_dll}"
                    VERBATIM)
                set(_found TRUE)
                break()
            endif()
        endforeach()
        if(NOT _found)
            message(WARNING
                "copy_runtime_dependencies: cannot find ${_dll} for target '${target}'. "
                "The server may fail to start. "
                "Set OPENSSL_ROOT_DIR to your OpenSSL installation path and re-run CMake.")
        endif()
    endforeach()

    # --- MySQL / MariaDB ---
    # MySQL Server puts libmysql.dll in its lib/ directory alongside the import lib.
    # MariaDB puts libmariadb.dll in the same place.
    # We try libmysql.dll first, then libmariadb.dll, and stop at the first one found
    # because only one database client is needed at runtime.

    set(_mysql_dirs "")

    if(DEFINED MYSQL_ROOT_DIR AND NOT MYSQL_ROOT_DIR STREQUAL "")
        file(TO_CMAKE_PATH "${MYSQL_ROOT_DIR}" _mysql_root)
        list(APPEND _mysql_dirs "${_mysql_root}/lib" "${_mysql_root}/bin" "${_mysql_root}")
    endif()

    if(DEFINED MYSQL_LIBRARY AND NOT MYSQL_LIBRARY STREQUAL "")
        # The .lib and .dll usually live in the same lib/ directory.
        get_filename_component(_mysql_lib_dir "${MYSQL_LIBRARY}" DIRECTORY)
        get_filename_component(_mysql_root_derived "${_mysql_lib_dir}" DIRECTORY)
        list(APPEND _mysql_dirs "${_mysql_lib_dir}" "${_mysql_root_derived}/bin")
    endif()

    set(_mysql_found FALSE)
    foreach(_dll IN ITEMS libmysql.dll libmariadb.dll)
        if(_mysql_found)
            break()
        endif()
        foreach(_dir IN LISTS _mysql_dirs)
            if(EXISTS "${_dir}/${_dll}")
                add_custom_command(TARGET ${target} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${_dir}/${_dll}"
                        "$<TARGET_FILE_DIR:${target}>/${_dll}"
                    COMMENT "Copying ${_dll}"
                    VERBATIM)
                set(_mysql_found TRUE)
                break()
            endif()
        endforeach()
    endforeach()

    if(NOT _mysql_found)
        message(WARNING
            "copy_runtime_dependencies: cannot find libmysql.dll or libmariadb.dll for target '${target}'. "
            "The server may fail to start. "
            "Set MYSQL_ROOT_DIR to your MySQL/MariaDB installation path and re-run CMake.")
    endif()

endfunction()
