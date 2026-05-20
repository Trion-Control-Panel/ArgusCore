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

    # --- Derive OpenSSL major version from the installed headers ---
    # OPENSSL_INCLUDE_DIR is a cache variable set by FindOpenSSL and is always
    # a single plain path, so it is safe to read here even though this function
    # definition is processed before find_package(OpenSSL) runs — the function
    # body is only evaluated at call time, which is after dep/ is processed.
    set(_ossl_major "3")  # safe fallback
    if(DEFINED OPENSSL_INCLUDE_DIR AND EXISTS "${OPENSSL_INCLUDE_DIR}/openssl/opensslv.h")
        file(STRINGS "${OPENSSL_INCLUDE_DIR}/openssl/opensslv.h" _ossl_ver_line
            REGEX "^#[ \t]*define[ \t]+OPENSSL_VERSION_STR[ \t]+\"[0-9]")
        if(_ossl_ver_line)
            string(REGEX REPLACE ".*\"([0-9]+)\\.[0-9]+.*" "\\1" _ossl_major "${_ossl_ver_line}")
        endif()
    endif()

    # --- Build the list of directories to search for OpenSSL DLLs ---
    set(_ssl_dirs "")

    if(DEFINED OPENSSL_ROOT_DIR AND NOT OPENSSL_ROOT_DIR STREQUAL "")
        file(TO_CMAKE_PATH "${OPENSSL_ROOT_DIR}" _ossl_root)
        list(APPEND _ssl_dirs "${_ossl_root}/bin" "${_ossl_root}")
    endif()

    if(DEFINED OPENSSL_INCLUDE_DIR AND NOT OPENSSL_INCLUDE_DIR STREQUAL "")
        get_filename_component(_ossl_root_from_include "${OPENSSL_INCLUDE_DIR}" DIRECTORY)
        list(APPEND _ssl_dirs "${_ossl_root_from_include}/bin" "${_ossl_root_from_include}")
    endif()

    if(DEFINED OPENSSL_SSL_LIBRARY AND NOT OPENSSL_SSL_LIBRARY STREQUAL "")
        foreach(_ossl_lib_entry IN LISTS OPENSSL_SSL_LIBRARY)
            if(_ossl_lib_entry STREQUAL "optimized" OR
               _ossl_lib_entry STREQUAL "debug"     OR
               _ossl_lib_entry STREQUAL "general")
                continue()
            endif()
            get_filename_component(_ossl_lib_dir    "${_ossl_lib_entry}" DIRECTORY)
            get_filename_component(_ossl_root_from_lib "${_ossl_lib_dir}" DIRECTORY)
            list(APPEND _ssl_dirs "${_ossl_root_from_lib}/bin" "${_ossl_lib_dir}")
        endforeach()
    endif()

    # --- Copy libssl and libcrypto using the actual major version suffix ---
    foreach(_dll IN ITEMS
            "libssl-${_ossl_major}-x64.dll"
            "libcrypto-${_ossl_major}-x64.dll")
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

    # --- Copy legacy.dll (required for RC4/ARC4 packet encryption) ---
    # The slproweb installer puts it in <root>/bin/ossl-modules/legacy.dll.
    # OpenSSLCrypto::threadsSetup() sets the provider search path to the
    # directory that contains worldserver.exe, so legacy.dll must land there
    # (not in a subdirectory).
    set(_legacy_found FALSE)
    foreach(_dir IN LISTS _ssl_dirs)
        set(_legacy_path "${_dir}/ossl-modules/legacy.dll")
        if(EXISTS "${_legacy_path}")
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${_legacy_path}"
                    "$<TARGET_FILE_DIR:${target}>/legacy.dll"
                COMMENT "Copying legacy.dll (OpenSSL legacy provider for RC4)"
                VERBATIM)
            set(_legacy_found TRUE)
            break()
        endif()
    endforeach()
    if(NOT _legacy_found)
        message(WARNING
            "copy_runtime_dependencies: cannot find ossl-modules/legacy.dll for target '${target}'. "
            "ARC4 packet encryption will crash at runtime. "
            "Ensure the OpenSSL legacy provider is installed alongside your OpenSSL distribution.")
    endif()

    # --- MySQL / MariaDB ---
    set(_mysql_dirs "")

    if(DEFINED MYSQL_ROOT_DIR AND NOT MYSQL_ROOT_DIR STREQUAL "")
        file(TO_CMAKE_PATH "${MYSQL_ROOT_DIR}" _mysql_root)
        list(APPEND _mysql_dirs "${_mysql_root}/lib" "${_mysql_root}/bin" "${_mysql_root}")
    endif()

    if(DEFINED MYSQL_LIBRARY AND NOT MYSQL_LIBRARY STREQUAL "")
        get_filename_component(_mysql_lib_dir    "${MYSQL_LIBRARY}" DIRECTORY)
        get_filename_component(_mysql_root_from_lib "${_mysql_lib_dir}" DIRECTORY)
        list(APPEND _mysql_dirs "${_mysql_lib_dir}" "${_mysql_root_from_lib}/bin")
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
