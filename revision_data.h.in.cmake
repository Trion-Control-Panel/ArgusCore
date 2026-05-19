/*
 * This file is part of the ArgusCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// Header guard renamed from TRINITYCORE_ to ARGUSCORE_ as part of Phase 1 rebranding.
// The macro names inside (TRINITY_*) are kept as-is until Phase 4 code modernization.
#ifndef ARGUSCORE_REVISION_DATA_H
#define ARGUSCORE_REVISION_DATA_H

// Git commit information
#define TRINITY_GIT_COMMIT_HASH     "@rev_hash@"
#define TRINITY_GIT_COMMIT_DATE     "@rev_date@"
#define TRINITY_GIT_COMMIT_BRANCH   R"(@rev_branch@)"

// Build OS information
#define TRINITY_BUILD_HOST_SYSTEM                   R"(@TRINITY_BUILD_HOST_SYSTEM@)"
#define TRINITY_BUILD_HOST_SYSTEM_VERSION           R"(@TRINITY_BUILD_HOST_SYSTEM_RELEASE@)"
#cmakedefine TRINITY_BUILD_HOST_DISTRO_NAME              R"(@TRINITY_BUILD_HOST_DISTRO_NAME@)"
#cmakedefine TRINITY_BUILD_HOST_DISTRO_VERSION_ID        R"(@TRINITY_BUILD_HOST_DISTRO_VERSION_ID@)"

// Build target information
#define TRINITY_BUILD_PROCESSOR     R"(@CMAKE_SYSTEM_PROCESSOR@)"

// CMake build information
#define TRINITY_BUILD_CMAKE_COMMAND             R"(@CMAKE_COMMAND@)"
#define TRINITY_BUILD_CMAKE_VERSION             R"(@CMAKE_VERSION@)"
#define TRINITY_BUILD_CMAKE_SOURCE_DIRECTORY    R"(@CMAKE_SOURCE_DIR@)"
#define TRINITY_BUILD_CMAKE_BUILD_DIRECTORY     R"(@BUILDDIR@)"

// Database updater base information
#define DATABASE_MYSQL_EXECUTABLE   R"(@MYSQL_EXECUTABLE@)"
#define DATABASE_FULL_DATABASE      "TDB_world_735.26972_2025_05_11.sql"
#define DATABASE_HOTFIXES_DATABASE  "TDB_hotfixes_735.26972_2025_05_11.sql"

// Windows resource defines — these string values show up in the .exe properties on Windows.
// The macro names (TRINITY_*) are intentionally kept as-is until Phase 4 code modernization.
#define TRINITY_COMPANYNAME_STR         "ArgusCore Team"
#define TRINITY_LEGALCOPYRIGHT_STR      "(c)2008-@rev_year@ ArgusCore"
#define TRINITY_FILEVERSION             0,0,0
#define TRINITY_FILEVERSION_STR         "@rev_hash@ @rev_date@ (@rev_branch@ branch)"
#define TRINITY_PRODUCTVERSION          TRINITY_FILEVERSION
#define TRINITY_PRODUCTVERSION_STR      TRINITY_FILEVERSION_STR

#endif // ARGUSCORE_REVISION_DATA_H
