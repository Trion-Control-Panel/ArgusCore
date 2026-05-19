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

// Header guard renamed from TrinityCore_ to ArgusCore_ as part of Phase 1 rebranding.
#ifndef ArgusCore_Banner_h__
#define ArgusCore_Banner_h__

#include "Define.h"

namespace Trinity
{
    namespace Banner
    {
        TC_COMMON_API void Show(char const* applicationName, void(*log)(char const* text), void(*logExtraInfo)());
    }
}

#endif // ArgusCore_Banner_h__
