# This file is part of the ArgusCore Project. See AUTHORS file for Copyright information
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

set(BOOST_SEARCH_HINTS)
if(WIN32)
  if(DEFINED ENV{BOOST_ROOT})
    set(BOOST_ROOT $ENV{BOOST_ROOT})
  endif()
  if(DEFINED BOOST_ROOT AND MSVC)
    file(TO_CMAKE_PATH "${BOOST_ROOT}" BOOST_ROOT)

    # MSVC_TOOLSET_VERSION is a 3-digit number like 143 (VS2022) or 144 (VS2025).
    # We split the last digit off to get the dot-separated form: 143 -> major=14, minor=3.
    string(LENGTH "${MSVC_TOOLSET_VERSION}" _tc_len)
    math(EXPR _tc_len "${_tc_len} - 1" OUTPUT_FORMAT DECIMAL)
    string(SUBSTRING "${MSVC_TOOLSET_VERSION}" 0 ${_tc_len} _tc_major)
    string(SUBSTRING "${MSVC_TOOLSET_VERSION}" ${_tc_len} -1 _tc_minor)

    # Build a list of cmake hint paths starting from the current toolset and working
    # down to 14.3. Boost libraries built with an older toolset (e.g. 14.3) are
    # binary-compatible with newer compilers (e.g. 14.4, 14.5), so this lets the
    # build find whatever version of Boost the developer has installed without
    # needing to recompile Boost just because they upgraded Visual Studio.
    set(BOOST_SEARCH_HINTS)
    set(_tc_minor_cur ${_tc_minor})
    while(_tc_minor_cur GREATER_EQUAL 3)
      list(APPEND BOOST_SEARCH_HINTS "${BOOST_ROOT}/lib${PLATFORM}-msvc-${_tc_major}.${_tc_minor_cur}/cmake")
      math(EXPR _tc_minor_cur "${_tc_minor_cur} - 1" OUTPUT_FORMAT DECIMAL)
    endwhile()

    unset(_tc_len)
    unset(_tc_major)
    unset(_tc_minor)
    unset(_tc_minor_cur)
  endif()

  set(Boost_USE_STATIC_LIBS        ON)
  set(Boost_USE_MULTITHREADED      ON)
  set(Boost_USE_STATIC_RUNTIME     OFF)
endif()

if (WIN32)
  # Windows minimum: 1.83 is the floor where BoostConfig.cmake became stable on MSVC.
  # Confirmed working with 1.91 locally and 1.86 on GitHub CI runners.
  set(BOOST_REQUIRED_VERSION 1.83)
else()
  # Linux minimum kept conservative to support Ubuntu 22.04 LTS apt packages (1.74).
  set(BOOST_REQUIRED_VERSION 1.74)
endif()

find_package(Boost ${BOOST_REQUIRED_VERSION}
  REQUIRED
  COMPONENTS 
    filesystem
    program_options
    regex
    locale
  CONFIG
  HINTS
    ${BOOST_SEARCH_HINTS})

if(NOT Boost_FOUND)
  if(NOT DEFINED ENV{BOOST_ROOT} AND NOT DEFINED Boost_DIR AND NOT DEFINED BOOST_ROOT AND NOT DEFINED BOOSTROOT)
    message(FATAL_ERROR "No BOOST_ROOT environment variable could be found! Please make sure it is set and the points to your Boost installation.")
  endif()
endif()

add_library(boost INTERFACE)

target_link_libraries(boost
  INTERFACE
    ${Boost_LIBRARIES})

target_include_directories(boost
  INTERFACE
    ${Boost_INCLUDE_DIRS})

target_compile_definitions(boost
  INTERFACE
    BOOST_ALL_NO_LIB
    BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE
    BOOST_ASIO_NO_DEPRECATED
    BOOST_BIND_NO_PLACEHOLDERS
    BOOST_SYSTEM_USE_UTF8)

if (WITH_BOOST_STACKTRACE AND NOT WIN32)
  message(STATUS "libbacktrace will be linked")

  include(CheckIncludeFile)

  if (BOOST_STACKTRACE_BACKTRACE_INCLUDE_FILE)
    check_include_file("${BOOST_STACKTRACE_BACKTRACE_INCLUDE_FILE}" HAS_BACKTRACE)
    target_compile_definitions(boost
      INTERFACE
        BOOST_STACKTRACE_BACKTRACE_INCLUDE_FILE=${BOOST_STACKTRACE_BACKTRACE_INCLUDE_FILE})
  else()
    check_include_file("backtrace.h" HAS_BACKTRACE)
  endif()

  if (NOT HAS_BACKTRACE)
    message(FATAL_ERROR "Required header 'backtrace.h' not found. If building with a compiler other than GCC, please specify the full path in the CMake option BOOST_STACKTRACE_BACKTRACE_INCLUDE_FILE.")
  endif()

  target_compile_definitions(boost
    INTERFACE
      BOOST_STACKTRACE_USE_BACKTRACE)

  target_link_libraries(boost
    INTERFACE
      backtrace)
endif()
