-- Death Knight: Blood Tap (45529) - Blood baseline ability, entirely missing from ArgusCore.
-- Confirmed genuine throughout all of Legion (7.0.3 redesign through removal in 8.0.1, per
-- Warcraft Wiki patch history) - a real gap, not backward drift, despite initially looking
-- like a pre-Legion (Blood Presence-era) mechanic.
--
-- Both spell_dk_blood_charge (passive: every 15 Runic Power spent generates 1 Blood Charge,
-- capped at 12) and spell_dk_blood_tap (active cast: consumes 5 Blood Charges to instantly
-- ready a random fully-depleted rune) are bound to the same spell id (45529), matching the
-- single combined tooltip describing both facets as one ability. See the in-code comments on
-- both classes: none of the four reference cores implement the 7.0.3-redesigned mechanic (all
-- four still carry the pre-7.0.3 "2 charges per damaging Death Coil/Frost Strike/Rune Strike
-- hit" version verbatim, confirmed stale via patch history), and none of them implement the
-- active cast/consume side at all - both were written fresh against the confirmed Legion
-- tooltip rather than ported.
DELETE FROM `spell_script_names` WHERE `spell_id` = 45529 AND `ScriptName` IN ('spell_dk_blood_charge', 'spell_dk_blood_tap');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(45529, 'spell_dk_blood_charge'),
(45529, 'spell_dk_blood_tap');
