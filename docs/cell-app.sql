/*
Navicat MySQL Data Transfer

Source Server         : app
Source Server Version : 50095
Source Host           : 182.92.4.156:3306
Source Database       : cell-app

Target Server Type    : MYSQL
Target Server Version : 50095
File Encoding         : 65001

Date: 2015-02-03 19:21:22
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `message`
-- ----------------------------
DROP TABLE IF EXISTS `message`;
CREATE TABLE `message` (
  `mid` bigint(20) unsigned NOT NULL auto_increment COMMENT '消息id',
  `uid` bigint(20) unsigned NOT NULL COMMENT '消息产生者uid',
  `content` varchar(512) default NULL COMMENT '消息文本',
  `post_images` varchar(512) default NULL,
  `geohash` varchar(15) NOT NULL COMMENT '上传消息时的geohash',
  `latitude` float(9,6) NOT NULL COMMENT '纬度',
  `longitude` float(9,6) NOT NULL COMMENT '经度',
  `create_time` datetime default NULL COMMENT '消息产生时间',
  PRIMARY KEY  (`mid`),
  UNIQUE KEY `idx_geohash` USING BTREE (`geohash`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of message
-- ----------------------------

-- ----------------------------
-- Table structure for `message_achievement`
-- ----------------------------
DROP TABLE IF EXISTS `message_achievement`;
CREATE TABLE `message_achievement` (
  `maid` bigint(20) unsigned NOT NULL auto_increment COMMENT '消息成就id',
  `mid` bigint(20) unsigned NOT NULL COMMENT '消息id，一条消息对应一条',
  `post_num` int(10) unsigned NOT NULL default '0' COMMENT '被传播的次数',
  `pass_num` int(10) unsigned NOT NULL default '0' COMMENT '被消灭的次数',
  `alive` int(10) unsigned NOT NULL default '0' COMMENT '存活数量',
  `max_deepth` int(10) unsigned NOT NULL default '0' COMMENT '传播最大层次',
  `max_dist` int(10) unsigned NOT NULL default '0' COMMENT '传播最远距离',
  `total_dist` int(10) unsigned NOT NULL default '0' COMMENT '传播总距离',
  `create_time` datetime default NULL COMMENT '创建时间',
  `update_time` datetime default NULL COMMENT '更新时间',
  PRIMARY KEY  (`maid`),
  UNIQUE KEY `idx_mid` USING BTREE (`mid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of message_achievement
-- ----------------------------

-- ----------------------------
-- Table structure for `message_comment`
-- ----------------------------
DROP TABLE IF EXISTS `message_comment`;
CREATE TABLE `message_comment` (
  `mcid` bigint(20) unsigned NOT NULL auto_increment COMMENT '消息评论id',
  `msid` bigint(20) unsigned NOT NULL COMMENT '消息种子id',
  `mid` bigint(20) unsigned NOT NULL COMMENT '消息id',
  `uid` bigint(20) unsigned NOT NULL COMMENT '评论者id',
  `org_uid` bigint(20) unsigned NOT NULL COMMENT '消息源uid',
  `comment` varchar(256) default NULL COMMENT '评论内容',
  `create_time` datetime default NULL COMMENT '评论时间',
  PRIMARY KEY  (`mcid`),
  KEY `idx_msid` USING BTREE (`msid`),
  KEY `idx_mid` USING BTREE (`mid`),
  KEY `idx_org_uid` USING BTREE (`org_uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of message_comment
-- ----------------------------

-- ----------------------------
-- Table structure for `message_seed`
-- ----------------------------
DROP TABLE IF EXISTS `message_seed`;
CREATE TABLE `message_seed` (
  `msid` bigint(20) unsigned NOT NULL auto_increment COMMENT '消息种子id，每条消息一个网格存储一个',
  `mid` bigint(20) unsigned NOT NULL COMMENT '原始消息id',
  `alive` bigint(20) unsigned NOT NULL COMMENT '消息存活量，每传播一次+(seed-1)，消灭一次-1',
  `geohash` varchar(15) NOT NULL COMMENT '网格geohash',
  `update_time` datetime default NULL COMMENT '更新时间',
  `deepth` int(10) unsigned NOT NULL default '0' COMMENT '传播层次',
  `dist` int(10) unsigned NOT NULL default '0' COMMENT '当前传播距离',
  `total_dist` int(10) unsigned NOT NULL default '0' COMMENT '传播总距离',
  `create_time` datetime default NULL COMMENT '创建时间',
  PRIMARY KEY  (`msid`),
  KEY `idx_mid` USING BTREE (`mid`),
  KEY `idx_geohash` USING HASH (`geohash`),
  KEY `idx_update_time` USING BTREE (`update_time`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of message_seed
-- ----------------------------

-- ----------------------------
-- Table structure for `message_transe_detail`
-- ----------------------------
DROP TABLE IF EXISTS `message_transe_detail`;
CREATE TABLE `message_transe_detail` (
  `mstid` bigint(30) unsigned NOT NULL auto_increment COMMENT '每条消息每次传播id',
  `msid` bigint(20) unsigned NOT NULL COMMENT '来源于那个消息传播id',
  `org_uid` bigint(20) unsigned NOT NULL COMMENT '原始消息创建者',
  `to_uid` bigint(20) unsigned NOT NULL COMMENT '当前用户传播或消灭者uid',
  `latitude` float(9,6) NOT NULL COMMENT '纬度',
  `longitude` float(9,6) NOT NULL COMMENT '经度',
  PRIMARY KEY  (`mstid`),
  KEY `idx_msid` USING BTREE (`mstid`),
  KEY `idx_org_uid` USING BTREE (`org_uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of message_transe_detail
-- ----------------------------

-- ----------------------------
-- Table structure for `test`
-- ----------------------------
DROP TABLE IF EXISTS `test`;
CREATE TABLE `test` (
  `id` int(11) NOT NULL auto_increment,
  `img` varchar(50) default 'a.gif',
  `phone` varchar(10) default NULL,
  PRIMARY KEY  (`id`),
  KEY `idx_phone` USING BTREE (`phone`)
) ENGINE=MyISAM AUTO_INCREMENT=7 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of test
-- ----------------------------
INSERT INTO `test` VALUES ('1', 'a.gif', '11');
INSERT INTO `test` VALUES ('2', '2015-01-22/aa/a.gif', '11');
INSERT INTO `test` VALUES ('3', 'a.gif', 'a');
INSERT INTO `test` VALUES ('4', 'a.gif', 'b');
INSERT INTO `test` VALUES ('5', 'a.gif', 'c');
INSERT INTO `test` VALUES ('6', 'a.gif', 'd');

-- ----------------------------
-- Table structure for `user`
-- ----------------------------
DROP TABLE IF EXISTS `user`;
CREATE TABLE `user` (
  `uid` bigint(20) unsigned NOT NULL auto_increment COMMENT '用户id',
  `passwd` char(32) default NULL COMMENT '密码md5',
  `deviceid` varchar(50) NOT NULL default '' COMMENT '设备id，一个设备只能一个临时账户和多个注册账户',
  `phone` char(15) default NULL COMMENT '电话号码，每个用户唯一,初始化时设为NULL',
  `email` varchar(50) default NULL COMMENT '邮箱，默认空',
  `birthday` date default NULL COMMENT '生日',
  `avanda_url` char(55) NOT NULL default 'default/icon.jpg' COMMENT '用户图像',
  `user_name` varchar(20) default NULL,
  `reg_source` tinyint(1) unsigned default '0' COMMENT '注册来源',
  `add_time` datetime default NULL COMMENT '注册时间',
  `mod_time` datetime default NULL COMMENT '修改时间',
  `is_login` tinyint(1) unsigned default '0' COMMENT '是否登陆',
  `status` tinyint(1) unsigned default '0' COMMENT '状态，注册或未注册',
  `last_login_time` datetime default NULL COMMENT '最后登陆时间',
  `login_count` int(10) unsigned default '0' COMMENT '登陆次数',
  PRIMARY KEY  (`uid`),
  UNIQUE KEY `idx_phone` USING BTREE (`phone`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of user
-- ----------------------------

-- ----------------------------
-- Table structure for `user_achievement`
-- ----------------------------
DROP TABLE IF EXISTS `user_achievement`;
CREATE TABLE `user_achievement` (
  `uaid` bigint(20) unsigned NOT NULL auto_increment COMMENT '用户成就id',
  `uid` bigint(20) unsigned NOT NULL COMMENT '用户uid,一个用户一条',
  `total_create` int(10) unsigned NOT NULL default '0' COMMENT '用户产生的消息数',
  `total_post_num` int(10) unsigned NOT NULL default '0' COMMENT '用户传播的消息数',
  `total_pass_num` int(10) unsigned NOT NULL default '0' COMMENT '用户消灭的消息树',
  `total_posted_num` bigint(20) unsigned NOT NULL default '0' COMMENT '产生的消息被传播的总次数',
  `total_passed_num` bigint(20) unsigned NOT NULL default '0' COMMENT '产生的消息被',
  `total_alive` bigint(20) unsigned NOT NULL default '0' COMMENT '产生的消息存货数量，每传播一次，增加seed-1个存活量',
  `max_dist` int(10) unsigned NOT NULL default '0' COMMENT '传播的最远距离',
  `total_dist` bigint(20) unsigned NOT NULL default '0' COMMENT '被传播的总距离',
  `create_time` datetime default NULL COMMENT '创建时间',
  `update_time` datetime default NULL COMMENT '更新时间',
  `level` smallint(5) unsigned NOT NULL default '1' COMMENT '用户等级，与seed有关',
  `seed` smallint(5) unsigned NOT NULL default '5' COMMENT '一条消息最多能传到附近seed个用户，与level有关',
  PRIMARY KEY  (`uaid`),
  UNIQUE KEY `idx_uid` USING HASH (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of user_achievement
-- ----------------------------
