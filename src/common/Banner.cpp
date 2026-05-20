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
#include <cstdio>

#if TRINITY_PLATFORM == TRINITY_PLATFORM_WINDOWS
#  include <Windows.h>
#else
#  include <unistd.h>
#endif

namespace
{
bool EnableBannerColors()
{
#if TRINITY_PLATFORM == TRINITY_PLATFORM_WINDOWS
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
        return false;
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode))
        return false;
    return SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
#else
    return isatty(STDOUT_FILENO) != 0;
#endif
}
} // namespace

void Trinity::Banner::Show(char const* applicationName, void(*log)(char const* text), void(*logExtraInfo)())
{
    log(Trinity::StringFormat("{} ({})", GitRevision::GetFullVersion(), applicationName).c_str());
    log("<Ctrl-C> to stop.\n");

    // ASCII art printed directly to stdout so ANSI codes reach the terminal
    // regardless of how the log callback is wired.
    //
    // Each row is split at column 29 — left side is "Argus" (green shades),
    // right side is "Core" (blue shades). Rows shade from bright (top) to dim
    // (bottom) to create a top-lit 3D look using only 2 hues.
    bool colors = EnableBannerColors();

    char const* g1    = colors ? "\033[92m"   : "";  // bright green — highlight (rows 1-2)
    char const* g2    = colors ? "\033[32m"   : "";  // normal green — mid face  (row 3)
    char const* g3    = colors ? "\033[2;32m" : "";  // dim green    — shadow    (rows 4-5)
    char const* b1    = colors ? "\033[94m"   : "";  // bright blue  — highlight
    char const* b2    = colors ? "\033[34m"   : "";  // normal blue  — mid face
    char const* b3    = colors ? "\033[2;34m" : "";  // dim blue     — shadow
    char const* reset = colors ? "\033[0m"    : "";

    //  Row split verification (each argus part is exactly 29 chars):
    //  row1: "    _                        " + "  ____               "
    //  row2: "   / \   _ __ __ _ _   _ ___" + " / ___|___  _ __ ___ "
    //  row3: "  / _ \ | '__/ _` | | | / __" + "| |   / _ \| '__/ _ \"
    //  row4: " / ___ \| | | (_| | |_| \__ \" + " |__| (_) | | |  __/"
    //  row5: "/_/   \_\_|  \__, |\__,_|___/" + " \____\___/|_|  \___|"

    printf("%s    _                        %s  ____               %s\n",        g1, b1, reset);
    printf("%s   / \\   _ __ __ _ _   _ ___%s / ___|___  _ __ ___ %s\n",        g1, b1, reset);
    printf("%s  / _ \\ | '__/ _` | | | / __%s| |   / _ \\| '__/ _ \\%s\n",      g2, b2, reset);
    printf("%s / ___ \\| | | (_| | |_| \\__ \\%s |__| (_) | | |  __/%s\n",       g3, b3, reset);
    printf("%s/_/   \\_\\_|  \\__, |\\__,_|___/%s \\____\\___/|_|  \\___|%s\n",      g3, b3, reset);
    printf("%s              |___/          %sA R G U S%s  %sC O R E%s   \n\n", g3, g1, reset, b1, reset);

    if (logExtraInfo)
        logExtraInfo();
}
