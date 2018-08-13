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


-- 导出 auth_center 的数据库结构
CREATE DATABASE IF NOT EXISTS `auth_center` /*!40100 DEFAULT CHARACTER SET utf8 */;
USE `auth_center`;

-- 导出  表 auth_center.player_info 结构
CREATE TABLE IF NOT EXISTS `player_info` (
  `uid` int(8) unsigned NOT NULL AUTO_INCREMENT COMMENT '玩家唯一ID',
  `name` varchar(20) NOT NULL DEFAULT '""' COMMENT '玩家游戏中名字',
  `sex` int(2) unsigned NOT NULL DEFAULT '0' COMMENT '玩家性别0表示男1表示女',
  `pwd` varchar(50) NOT NULL DEFAULT '""' COMMENT '玩家登录密码',
  `is_guest` int(2) unsigned NOT NULL DEFAULT '0' COMMENT '玩家是否为游客',
  `guest_key` varchar(36) NOT NULL COMMENT '玩家为游客账号的key',
  `uface` int(8) unsigned NOT NULL DEFAULT '0' COMMENT '玩家头像id',
  `status` int(8) unsigned NOT NULL DEFAULT '0' COMMENT '玩家账号状态',
  `money` int(8) unsigned NOT NULL DEFAULT '0' COMMENT '玩家金币数量',
  `grade` int(8) unsigned NOT NULL DEFAULT '1' COMMENT '玩家等级',
  `phone` varchar(15) NOT NULL DEFAULT '""' COMMENT '玩家绑定的手机号',
  PRIMARY KEY (`uid`)
) ENGINE=InnoDB AUTO_INCREMENT=4211 DEFAULT CHARSET=utf8 COMMENT='玩家注册的信息';

-- 数据导出被取消选择。
/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
