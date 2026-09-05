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

#include "tc_catch2.h"

#include "GridDefines.h"
#include "MapPartitioning.h"

TEST_CASE("PartitionGridRange: Contains bounds correctly")
{
    PartitionGridRange range{ .MinGridX = 10, .MaxGridX = 20, .MinGridY = 5, .MaxGridY = 15 };

    REQUIRE(range.Contains(10, 5));   // min corner
    REQUIRE(range.Contains(20, 15));  // max corner
    REQUIRE(range.Contains(15, 10));  // interior

    REQUIRE_FALSE(range.Contains(9, 5));    // just outside MinGridX
    REQUIRE_FALSE(range.Contains(21, 5));   // just outside MaxGridX
    REQUIRE_FALSE(range.Contains(10, 4));   // just outside MinGridY
    REQUIRE_FALSE(range.Contains(10, 16));  // just outside MaxGridY
}

TEST_CASE("MapPartitionLayout: 2x2 even split covers every grid coordinate exactly once")
{
    // Mirrors MapPartitionMgr's own BuildEvenSplit for a 2x2 split of the real
    // MAX_NUMBER_OF_GRIDS (64) grid space: 4 partitions, each a 32x32 quadrant.
    std::vector<PartitionGridRange> ranges
    {
        PartitionGridRange{ .MinGridX = 0,  .MaxGridX = 31, .MinGridY = 0,  .MaxGridY = 31 }, // partition 0: top-left
        PartitionGridRange{ .MinGridX = 32, .MaxGridX = 63, .MinGridY = 0,  .MaxGridY = 31 }, // partition 1: top-right
        PartitionGridRange{ .MinGridX = 0,  .MaxGridX = 31, .MinGridY = 32, .MaxGridY = 63 }, // partition 2: bottom-left
        PartitionGridRange{ .MinGridX = 32, .MaxGridX = 63, .MinGridY = 32, .MaxGridY = 63 }, // partition 3: bottom-right
    };
    MapPartitionLayout layout(ranges);

    REQUIRE(layout.GetPartitionCount() == 4);

    REQUIRE(layout.GetPartitionIndex(0, 0) == Optional<uint32>(0));
    REQUIRE(layout.GetPartitionIndex(31, 31) == Optional<uint32>(0));
    REQUIRE(layout.GetPartitionIndex(32, 0) == Optional<uint32>(1));
    REQUIRE(layout.GetPartitionIndex(63, 31) == Optional<uint32>(1));
    REQUIRE(layout.GetPartitionIndex(0, 32) == Optional<uint32>(2));
    REQUIRE(layout.GetPartitionIndex(31, 63) == Optional<uint32>(2));
    REQUIRE(layout.GetPartitionIndex(32, 32) == Optional<uint32>(3));
    REQUIRE(layout.GetPartitionIndex(63, 63) == Optional<uint32>(3));

    // The boundary line itself (x=31 vs x=32) must land in exactly one partition, never both
    // and never neither - every grid coordinate in [0, MAX_NUMBER_OF_GRIDS) x
    // [0, MAX_NUMBER_OF_GRIDS) resolves to exactly one partition index.
    for (uint32 x = 0; x < MAX_NUMBER_OF_GRIDS; ++x)
    {
        for (uint32 y = 0; y < MAX_NUMBER_OF_GRIDS; ++y)
        {
            Optional<uint32> partition = layout.GetPartitionIndex(x, y);
            REQUIRE(partition.has_value());
            REQUIRE(*partition < 4);
        }
    }
}

TEST_CASE("MapPartitionLayout: coordinate outside every configured range resolves to empty")
{
    std::vector<PartitionGridRange> ranges{ PartitionGridRange{ .MinGridX = 0, .MaxGridX = 10, .MinGridY = 0, .MaxGridY = 10 } };
    MapPartitionLayout layout(ranges);

    REQUIRE_FALSE(layout.GetPartitionIndex(20, 20).has_value());
}

TEST_CASE("MapPartitionLayout: IsNearBoundary is exact at the haloWidth threshold")
{
    // Two partitions split purely along X (full Y range in both), so the boundary is a single
    // vertical line - avoids corner-case complexity for this test, covered separately below.
    // Trinity::ComputeGridCoord's real formula (GridDefines.h: Compute<>, CENTER_GRID_ID=32,
    // CENTER_GRID_OFFSET=SIZE_OF_GRIDS/2) places the grid 31/32 boundary at exactly world x=0
    // (x=0 itself resolves to grid 32) - derived from the same constants IsNearBoundary itself
    // uses, not an arbitrary guess, so these test points exercise the real conversion.
    std::vector<PartitionGridRange> ranges
    {
        PartitionGridRange{ .MinGridX = 0,  .MaxGridX = 31, .MinGridY = 0, .MaxGridY = 63 },
        PartitionGridRange{ .MinGridX = 32, .MaxGridX = 63, .MinGridY = 0, .MaxGridY = 63 },
    };
    MapPartitionLayout layout(ranges);
    constexpr float haloWidth = 100.0f;

    // Deep in partition 0's interior - far past haloWidth from the x=0 boundary.
    REQUIRE_FALSE(layout.IsNearBoundary(-200.0f, 0.0f, haloWidth));

    // Just inside haloWidth of the boundary (distance 99 < 100).
    REQUIRE(layout.IsNearBoundary(-99.0f, 0.0f, haloWidth));

    // Just outside haloWidth of the boundary (distance 101 > 100).
    REQUIRE_FALSE(layout.IsNearBoundary(-101.0f, 0.0f, haloWidth));

    // Same shape on the partition 1 side.
    REQUIRE(layout.IsNearBoundary(99.0f, 0.0f, haloWidth));
    REQUIRE_FALSE(layout.IsNearBoundary(101.0f, 0.0f, haloWidth));

    // A position not covered by any configured range - false, matching GetPartitionIndex's own
    // "no claim past a well-formed layout" behavior, not a crash.
    std::vector<PartitionGridRange> partial{ PartitionGridRange{ .MinGridX = 0, .MaxGridX = 10, .MinGridY = 0, .MaxGridY = 10 } };
    MapPartitionLayout partialLayout(partial);
    REQUIRE_FALSE(partialLayout.IsNearBoundary(100000.0f, 100000.0f, haloWidth));
}

TEST_CASE("MapPartitionLayout: IsNearBoundary detects a corner where multiple partitions meet")
{
    // Real 2x2 layout (same ranges as the "covers every grid coordinate" test above) - the grid
    // 31/32 boundary sits at world x=0 AND world y=0 (identical derivation to the single-axis
    // test above, same formula/constants for both axes), so (0,0) is exactly where all 4
    // partitions meet.
    std::vector<PartitionGridRange> ranges
    {
        PartitionGridRange{ .MinGridX = 0,  .MaxGridX = 31, .MinGridY = 0,  .MaxGridY = 31 },
        PartitionGridRange{ .MinGridX = 32, .MaxGridX = 63, .MinGridY = 0,  .MaxGridY = 31 },
        PartitionGridRange{ .MinGridX = 0,  .MaxGridX = 31, .MinGridY = 32, .MaxGridY = 63 },
        PartitionGridRange{ .MinGridX = 32, .MaxGridX = 63, .MinGridY = 32, .MaxGridY = 63 },
    };
    MapPartitionLayout layout(ranges);
    constexpr float haloWidth = 100.0f;

    // Close to the corner along both axes (50 < 100 on each) - own partition is 0 (top-left);
    // the diagonally-touching partition 3 is only reachable via the axis probes finding a
    // different neighbor first (partition 1 or 2), proving the 4-cardinal-probe approach still
    // correctly flags corner proximity without needing a diagonal probe.
    REQUIRE(layout.IsNearBoundary(-50.0f, -50.0f, haloWidth));

    // Far from the corner on both axes - genuinely deep in partition 0's interior.
    REQUIRE_FALSE(layout.IsNearBoundary(-200.0f, -200.0f, haloWidth));
}

TEST_CASE("MapPartitionMgr: disabled globally returns nullptr regardless of configured map ids")
{
    MapPartitionMgr::instance()->Configure(false, "0:2x2,1:2x2"sv, 50, 400.0f);

    REQUIRE_FALSE(MapPartitionMgr::instance()->IsEnabled());
    REQUIRE(MapPartitionMgr::instance()->GetLayout(0) == nullptr);
    REQUIRE(MapPartitionMgr::instance()->GetLayout(1) == nullptr);
}

TEST_CASE("MapPartitionMgr: enabled with a valid even split resolves the opted-in map, not others")
{
    MapPartitionMgr::instance()->Configure(true, "0:2x2"sv, 75, 400.0f);

    REQUIRE(MapPartitionMgr::instance()->IsEnabled());
    REQUIRE(MapPartitionMgr::instance()->GetMinPopulationForFanout() == 75);
    REQUIRE(MapPartitionMgr::instance()->GetHaloWidth() == 400.0f);

    MapPartitionLayout const* eastern = MapPartitionMgr::instance()->GetLayout(0);
    REQUIRE(eastern != nullptr);
    REQUIRE(eastern->GetPartitionCount() == 4);

    // Exercises the real BuildEvenSplit (anonymous namespace, MapPartitioning.cpp) through
    // Configure() rather than a hand-built comparison layout - an off-by-one regression in the
    // actual remainder-handling logic wouldn't be caught by GetPartitionCount() alone.
    PartitionGridRange const& topLeft = eastern->GetRange(0);
    REQUIRE(topLeft.MinGridX == 0);
    REQUIRE(topLeft.MaxGridX == 31);
    REQUIRE(topLeft.MinGridY == 0);
    REQUIRE(topLeft.MaxGridY == 31);

    PartitionGridRange const& bottomRight = eastern->GetRange(3);
    REQUIRE(bottomRight.MinGridX == 32);
    REQUIRE(bottomRight.MaxGridX == 63);
    REQUIRE(bottomRight.MinGridY == 32);
    REQUIRE(bottomRight.MaxGridY == 63);

    // Kalimdor (mapId 1) was never listed in this Configure() call.
    REQUIRE(MapPartitionMgr::instance()->GetLayout(1) == nullptr);
}

TEST_CASE("MapPartitionMgr: malformed entries are skipped individually, not fatal to the whole list")
{
    MapPartitionMgr::instance()->Configure(true, "0:2x2,notanumber:2x2,1:0x2,2:2x2"sv, 50, 400.0f);

    REQUIRE(MapPartitionMgr::instance()->GetLayout(0) != nullptr);   // well-formed, kept
    REQUIRE(MapPartitionMgr::instance()->GetLayout(1) == nullptr);   // 0 columns, rejected
    REQUIRE(MapPartitionMgr::instance()->GetLayout(2) != nullptr);   // well-formed, kept
}

TEST_CASE("MapPartitionMgr: duplicate map id entries keep the first, skip the rest")
{
    MapPartitionMgr::instance()->Configure(true, "0:2x2,0:3x3"sv, 50, 400.0f);

    MapPartitionLayout const* layout = MapPartitionMgr::instance()->GetLayout(0);
    REQUIRE(layout != nullptr);
    REQUIRE(layout->GetPartitionCount() == 4); // the 2x2 from the first occurrence, not 3x3
}

TEST_CASE("MapPartitionMgr: re-Configure clears any previously configured maps")
{
    MapPartitionMgr::instance()->Configure(true, "0:2x2"sv, 50, 400.0f);
    REQUIRE(MapPartitionMgr::instance()->GetLayout(0) != nullptr);

    MapPartitionMgr::instance()->Configure(true, "1:2x2"sv, 50, 400.0f);
    REQUIRE(MapPartitionMgr::instance()->GetLayout(0) == nullptr);
    REQUIRE(MapPartitionMgr::instance()->GetLayout(1) != nullptr);
}

TEST_CASE("MapPartitionMgr: non-power-of-two split has correct remainder handling and full coverage")
{
    // MAX_NUMBER_OF_GRIDS (64) is not evenly divisible by 3 - exercises BuildEvenSplit's
    // remainder-absorption path (the last column/row gets whatever the integer division drops),
    // not just the power-of-two case the other tests use. 64 / 3 = 21 remainder 1.
    MapPartitionMgr::instance()->Configure(true, "0:3x3"sv, 50, 400.0f);

    MapPartitionLayout const* layout = MapPartitionMgr::instance()->GetLayout(0);
    REQUIRE(layout != nullptr);
    REQUIRE(layout->GetPartitionCount() == 9);

    PartitionGridRange const& firstCol = layout->GetRange(0);
    REQUIRE(firstCol.MinGridX == 0);
    REQUIRE(firstCol.MaxGridX == 20);

    PartitionGridRange const& secondCol = layout->GetRange(1);
    REQUIRE(secondCol.MinGridX == 21);
    REQUIRE(secondCol.MaxGridX == 41);

    // Last column/row absorbs the remainder rather than being clipped to another 21-wide slice -
    // must reach exactly MAX_NUMBER_OF_GRIDS - 1, not 21+21+21-1=62.
    PartitionGridRange const& lastCol = layout->GetRange(2);
    REQUIRE(lastCol.MinGridX == 42);
    REQUIRE(lastCol.MaxGridX == MAX_NUMBER_OF_GRIDS - 1);

    // Full coverage: every grid coordinate resolves to exactly one of the 9 partitions, no gaps.
    for (uint32 x = 0; x < MAX_NUMBER_OF_GRIDS; ++x)
    {
        for (uint32 y = 0; y < MAX_NUMBER_OF_GRIDS; ++y)
        {
            Optional<uint32> partition = layout->GetPartitionIndex(x, y);
            REQUIRE(partition.has_value());
            REQUIRE(*partition < 9);
        }
    }
}

// Phase 7 (ARGUSCORE_FIXES.md, "Observability + kill-switch"). Uses a deliberately unlikely-to-
// collide mapId (999999, never a real WoW map) since MapPartitionMgr::instance() is a shared
// singleton across every test case in this binary, and this state (unlike _layouts) is NOT reset
// by Configure() - each test explicitly cleans up its own mapId at the end so it can't leak into
// a later test case.

TEST_CASE("MapPartitionMgr: force-disabled state starts false for any mapId, including ones never opted in via Configure()")
{
    REQUIRE_FALSE(MapPartitionMgr::instance()->IsForceDisabled(999999));
}

TEST_CASE("MapPartitionMgr: SetForceDisabled(true) then IsForceDisabled reports true, independent of Configure()'s own state")
{
    // Deliberately never call Configure() for this mapId at all - the kill-switch is independent
    // state, settable/queryable regardless of whether the mapId was ever opted into partitioning.
    REQUIRE_FALSE(MapPartitionMgr::instance()->IsForceDisabled(999998));

    MapPartitionMgr::instance()->SetForceDisabled(999998, true);
    REQUIRE(MapPartitionMgr::instance()->IsForceDisabled(999998));

    MapPartitionMgr::instance()->SetForceDisabled(999998, false);
    REQUIRE_FALSE(MapPartitionMgr::instance()->IsForceDisabled(999998));
}

TEST_CASE("MapPartitionMgr: force-disabling one mapId does not affect another")
{
    MapPartitionMgr::instance()->SetForceDisabled(999997, true);
    REQUIRE(MapPartitionMgr::instance()->IsForceDisabled(999997));
    REQUIRE_FALSE(MapPartitionMgr::instance()->IsForceDisabled(999996));

    MapPartitionMgr::instance()->SetForceDisabled(999997, false);
    REQUIRE_FALSE(MapPartitionMgr::instance()->IsForceDisabled(999997));
}

TEST_CASE("MapPartitionMgr: Configure() does not clear a previously-set force-disable")
{
    // The kill-switch is deliberately separate state from _layouts/_enabled/etc - a live
    // .reload config must not silently re-enable a map an operator just emergency-disabled.
    MapPartitionMgr::instance()->SetForceDisabled(999995, true);
    MapPartitionMgr::instance()->Configure(true, "999995:2x2"sv, 50, 400.0f);

    REQUIRE(MapPartitionMgr::instance()->GetLayout(999995) != nullptr);   // opted in via Configure()
    REQUIRE(MapPartitionMgr::instance()->IsForceDisabled(999995));         // kill-switch still engaged

    // Clean up: clear the kill-switch and remove the opt-in so this mapId doesn't leak into any
    // later test that might (now or in the future) also probe an unused-looking high mapId.
    MapPartitionMgr::instance()->SetForceDisabled(999995, false);
    MapPartitionMgr::instance()->Configure(true, ""sv, 50, 400.0f);
    REQUIRE(MapPartitionMgr::instance()->GetLayout(999995) == nullptr);
}
