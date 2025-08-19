DROP TABLE IF EXISTS `character_declinedname`;
CREATE TABLE `character_declinedname` (
  `guid` BIGINT UNSIGNED NOT NULL,
  `genitive` VARCHAR(32) NOT NULL DEFAULT '',
  `dative` VARCHAR(32) NOT NULL DEFAULT '',
  `accusative` VARCHAR(32) NOT NULL DEFAULT '',
  `instrumental` VARCHAR(32) NOT NULL DEFAULT '',
  `prepositional` VARCHAR(32) NOT NULL DEFAULT '',
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
