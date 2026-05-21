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

#ifndef _VMAPDEFINITIONS_H
#define _VMAPDEFINITIONS_H

#include "ExtractorsVersion.h"
#include <cstdio>

#define LIQUID_TILE_SIZE (533.333f / 128.f)

namespace VMAP
{
    // "ARGUS_Vn" — assembled vmap files (8 bytes compared on load, no null in window)
    const char VMAP_MAGIC[]     = { 'A','R','G','U','S','_','V', char('0' + EXTRACTORS_VERSION), '\0' };
    // "ARGUSVn\0" — raw extracted vmap files (7 chars + null = 8 bytes compared on load)
    const char RAW_VMAP_MAGIC[] = { 'A','R','G','U','S','V',     char('0' + EXTRACTORS_VERSION), '\0' };
    const char GAMEOBJECT_MODELS[] = "GameObjectModels.dtree";

    // defined in VMapManager2.cpp currently...
    bool readChunk(FILE* rf, char *dest, const char *compare, uint32 len);
}

#endif
