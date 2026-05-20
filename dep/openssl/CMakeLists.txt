# This file is part of the ArgusCore Project. See AUTHORS file for Copyright information
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

# -----------------------------------------------------------------------
# Windows: auto-select the highest installed OpenSSL version.
# Only runs when the caller has not already pinned a path via
# -DOPENSSL_ROOT_DIR or the OPENSSL_ROOT_DIR environment variable.
# Once detected the result is cached, so CMake will not re-scan on
# every reconfigure unless the cache is cleared.
# -----------------------------------------------------------------------
if(WIN32 AND NOT DEFINED OPENSSL_ROOT_DIR AND "$ENV{OPENSSL_ROOT_DIR}" STREQUAL "")
    file(TO_CMAKE_PATH "$ENV{ProgramFiles}" _pf)
    file(TO_CMAKE_PATH "$ENV{SYSTEMDRIVE}"  _sd)

    # Probe every plausible installation directory layout.
    set(_probe_dirs "")
    foreach(_suffix IN ITEMS
            "OpenSSL-Win64"
            "OpenSSL-Win64-4" "OpenSSL-4-Win64" "OpenSSL 4"
            "OpenSSL-Win64-3" "OpenSSL-3-Win64" "OpenSSL 3")
        foreach(_base IN ITEMS "${_pf}" "${_sd}" "C:")
            list(APPEND _probe_dirs "${_base}/${_suffix}")
        endforeach()
    endforeach()
    unset(_pf)
    unset(_sd)

    set(_best_root "")
    set(_best_ver  "0.0.0")

    foreach(_dir IN LISTS _probe_dirs)
        set(_vh "${_dir}/include/openssl/opensslv.h")
        if(NOT EXISTS "${_vh}")
            continue()
        endif()

        # OpenSSL 3.0+ exposes OPENSSL_VERSION_STR "X.Y.Z"
        file(STRINGS "${_vh}" _line
            REGEX "^#[ \t]*define[ \t]+OPENSSL_VERSION_STR[ \t]+\"[0-9]")
        if(_line)
            string(REGEX REPLACE ".*\"([0-9]+\\.[0-9]+\\.[0-9]+)\".*" "\\1" _ver "${_line}")
        else()
            # Pre-3.0 legacy format: OPENSSL_VERSION_NUMBER 0xMNNFFPPS
            file(STRINGS "${_vh}" _line
                REGEX "^#[ \t]*define[ \t]+OPENSSL_VERSION_NUMBER[ \t]+0x[0-9a-fA-F]")
            if(NOT _line)
                continue()
            endif()
            # Extract major.minor as decimal digits for comparison purposes only.
            string(REGEX REPLACE
                ".*0x0([0-9a-fA-F])([0-9a-fA-F][0-9a-fA-F])[0-9a-fA-F]+.*"
                "\\1.\\2.0" _ver "${_line}")
        endif()

        if(_ver VERSION_GREATER _best_ver)
            set(_best_ver  "${_ver}")
            set(_best_root "${_dir}")
        endif()
    endforeach()
    unset(_probe_dirs)

    if(_best_root)
        message(STATUS "OpenSSL: auto-selected version ${_best_ver} at ${_best_root}")
        set(OPENSSL_ROOT_DIR "${_best_root}" CACHE PATH
            "OpenSSL root (auto-detected; override with -DOPENSSL_ROOT_DIR=...)")
    endif()
    unset(_best_root)
    unset(_best_ver)
endif()

find_package(OpenSSL 3 REQUIRED COMPONENTS Crypto SSL)

add_library(openssl INTERFACE)

target_link_libraries(openssl
  INTERFACE
    OpenSSL::Crypto
    OpenSSL::SSL)
