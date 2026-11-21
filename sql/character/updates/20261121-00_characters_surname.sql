ALTER TABLE `characters` ADD COLUMN `surname` varchar(48) NOT NULL DEFAULT '' AFTER `name`;

INSERT INTO `character_db_version` (`id`, `LastUpdate`) VALUES ('21', '20261121-00_characters_surname');