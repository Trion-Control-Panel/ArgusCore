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

#include "Banner.h"
#include "GitRevision.h"
#include "StringFormat.h"

void Trinity::Banner::Show(char const* applicationName, void(*log)(char const* text), void(*logExtraInfo)())
{
    log(Trinity::StringFormat("{} ({})", GitRevision::GetFullVersion(), applicationName).c_str());
    log(R"(<Ctrl-C> to stop.)" "\n");

    // ArgusCore startup banner — ASCII art spells out "ArgusCore" using the standard figlet font.
    // Each log() call prints one row. Raw string literals R"(...)" let us write backslashes without escaping.
    // The last row contains the g-descender bottom stroke on the left and the project label on the right.
    log(R"(    _                          ____               )");
    log(R"(   / \   _ __ __ _ _   _ ___ / ___|___  _ __ ___ )");
    log(R"(  / _ \ | '__/ _` | | | / __| |   / _ \| '__/ _ \)");
    log(R"( / ___ \| | | (_| | |_| \__ \ |__| (_) | | |  __/)");
    log(R"(/_/   \_\_|  \__, |\__,_|___/ \____\___/|_|  \___|)");
    log(R"(              |___/          A R G U S  C O R E   )" "\n");

    if (logExtraInfo)
        logExtraInfo();
}
