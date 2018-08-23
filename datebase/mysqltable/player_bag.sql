-- --------------------------------------------------------
-- 主机:                           127.0.0.1
-- 服务器版本:                        5.7.15-log - MySQL Community Server (GPL)
-- 服务器操作系统:                      Win64
-- HeidiSQL 版本:                  9.4.0.5125
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;


-- 导出 game_center 的数据库结构
CREATE DATABASE IF NOT EXISTS `game_center` /*!40100 DEFAULT CHARACTER SET utf8 */;
USE `game_center`;

-- 导出  表 game_center.player_bag 结构
CREATE TABLE IF NOT EXISTS `player_bag` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `uid` int(11) NOT NULL COMMENT '玩家ID',
  `equip_113` int(8) unsigned NOT NULL DEFAULT '0' COMMENT '钻石',
  `equip_114` int(8) unsigned NOT NULL DEFAULT '0' COMMENT '草药',
  `equip_115` int(8) unsigned NOT NULL DEFAULT '0' COMMENT '贝壳',
  `equip_116` int(8) unsigned NOT NULL DEFAULT '0' COMMENT '钥匙',
  `equip_117` int(8) unsigned NOT NULL DEFAULT '0' COMMENT '宝箱',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=2 DEFAULT CHARSET=utf8 COMMENT='玩家背包';

-- 正在导出表  game_center.player_bag 的数据：~0 rows (大约)
DELETE FROM `player_bag`;
/*!40000 ALTER TABLE `player_bag` DISABLE KEYS */;
INSERT INTO `player_bag` (`id`, `uid`, `equip_113`, `equip_114`, `equip_115`, `equip_116`, `equip_117`) VALUES
	(1, 4210, 100, 50, 80, 0, 0);
/*!40000 ALTER TABLE `player_bag` ENABLE KEYS */;

/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
