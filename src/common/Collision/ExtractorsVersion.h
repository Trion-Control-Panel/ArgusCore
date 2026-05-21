/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
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

#ifndef ExtractorsVersion_h__
#define ExtractorsVersion_h__

#include "Define.h"

// ──────────────────────────────────────────────────────────────────────────────
// Single source of truth for all extractor tool output versioning.
//
// Bump EXTRACTORS_VERSION whenever ANY extractor produces different output —
// whether that is a file-format change OR a logic change that affects the data
// (e.g. a race-condition fix that now produces more complete vmap doodad sets).
//
// This value propagates automatically to:
//   • MapVersionMagic   (maps/*.map  files)
//   • MMAP_VERSION      (mmaps/*.mmtile files)
//   • VMAP_MAGIC        (vmaps/ assembled tree/tile files)
//   • RAW_VMAP_MAGIC    (vmaps/ raw extracted model files)
//
// The server refuses to load data whose embedded version does not match. A
// mismatch means the data must be re-extracted and re-assembled from the client.
// ──────────────────────────────────────────────────────────────────────────────

inline constexpr uint32 EXTRACTORS_VERSION = 1;

// VMAP magic strings encode the version as a single decimal digit.
// Supported range: 1–9. If EXTRACTORS_VERSION ever reaches 10, extend the
// encoding below (e.g. use two digit characters or a different scheme).
static_assert(EXTRACTORS_VERSION >= 1 && EXTRACTORS_VERSION <= 9,
    "VMAP magic strings only encode a single digit version — update the encoding in ExtractorsVersion.h");

#endif // ExtractorsVersion_h__
