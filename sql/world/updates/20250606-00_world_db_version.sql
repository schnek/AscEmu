-- UPDATE `creature_spawns` SET `movetype`=0 WHERE  `id`=2523 AND `min_build`=8606;
-- UPDATE `creature_spawns` SET `movetype`=0 WHERE  `id`=5874 AND `min_build`=12340;
-- UPDATE `creature_spawns` SET `movetype`=0 WHERE  `id`=18795 AND `min_build`=12340;
-- start bc

UPDATE `creature_spawns` SET `movetype`=0 WHERE  `id`=57798 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`=0 WHERE  `id`=57799 AND `min_build`=12340;
UPDATE `creature_spawns` SET `movetype`=0 WHERE  `id`=62252;
UPDATE `creature_spawns` SET `movetype`=0 WHERE  `id`=62245;

UPDATE `creature_spawns` SET `orientation`=6.280503 WHERE  `id`=57799 AND `min_build`=12340;

-- double spawn
DELETE FROM `creature_spawns` WHERE  `id`=289416;
DELETE FROM `creature_spawns` WHERE  `id`=34043;
DELETE FROM `creature_spawns` WHERE  `id`=5403;

-- deprecated for 4.x

INSERT INTO `world_db_version` (`id`, `LastUpdate`) VALUES ('144', '20250606-00_world_db_version');

