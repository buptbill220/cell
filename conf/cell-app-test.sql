/*
Navicat MySQL Data Transfer

Source Server         : app
Source Server Version : 50095
Source Host           : 182.92.218.59:3306
Source Database       : cell-app-test

Target Server Type    : MYSQL
Target Server Version : 50095
File Encoding         : 65001

Date: 2015-03-26 11:24:23
*/

SET FOREIGN_KEY_CHECKS=0;

-- ----------------------------
-- Table structure for `message`
-- ----------------------------
DROP TABLE IF EXISTS `message`;
CREATE TABLE `message` (
  `mid` bigint(20) unsigned NOT NULL auto_increment COMMENT '消息id',
  `uid` bigint(20) unsigned NOT NULL COMMENT '消息产生者uid',
  `content` varchar(512) NOT NULL COMMENT '消息文本',
  `post_images` varchar(1024) NOT NULL,
  `images_size` varchar(100) NOT NULL COMMENT '图像尺寸',
  `geohash` char(6) NOT NULL COMMENT '上传消息时的geohash',
  `latitude` float(9,6) NOT NULL COMMENT '纬度',
  `longitude` float(9,6) NOT NULL COMMENT '经度',
  `create_time` datetime default NULL COMMENT '消息产生时间',
  PRIMARY KEY  (`mid`),
  KEY `idx_geohash` USING HASH (`geohash`)
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
  `alive` int(10) unsigned NOT NULL default '5' COMMENT '存活数量',
  `max_depth` int(10) unsigned NOT NULL default '0' COMMENT '传播最大层次',
  `max_dist` float(8,3) unsigned NOT NULL default '0.000' COMMENT '传播最远距离',
  `total_dist` float(8,3) unsigned NOT NULL default '0.000' COMMENT '传播总距离',
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
-- Table structure for `message_pass`
-- ----------------------------
DROP TABLE IF EXISTS `message_pass`;
CREATE TABLE `message_pass` (
  `mstid` bigint(30) unsigned NOT NULL auto_increment COMMENT '每条消息每次传播id',
  `msid` bigint(20) unsigned NOT NULL COMMENT '来源于那个消息传播id',
  `mid` bigint(20) unsigned NOT NULL,
  `org_uid` bigint(20) unsigned NOT NULL COMMENT '原始消息创建者',
  `to_uid` bigint(20) unsigned NOT NULL COMMENT '当前用户传播或消灭者uid',
  `latitude` float(9,6) NOT NULL COMMENT '纬度',
  `longitude` float(9,6) NOT NULL COMMENT '经度',
  `create_time` datetime default NULL COMMENT '动作时间',
  PRIMARY KEY  (`mstid`),
  KEY `idx_msid` USING BTREE (`mstid`),
  KEY `idx_org_uid` USING BTREE (`org_uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of message_pass
-- ----------------------------

-- ----------------------------
-- Table structure for `message_post`
-- ----------------------------
DROP TABLE IF EXISTS `message_post`;
CREATE TABLE `message_post` (
  `mstid` bigint(30) unsigned NOT NULL auto_increment COMMENT '每条消息每次传播id',
  `msid` bigint(20) unsigned NOT NULL COMMENT '来源于那个消息传播id',
  `mid` bigint(20) unsigned NOT NULL,
  `org_uid` bigint(20) unsigned NOT NULL COMMENT '原始消息创建者',
  `to_uid` bigint(20) unsigned NOT NULL COMMENT '当前用户传播或消灭者uid',
  `latitude` float(9,6) NOT NULL COMMENT '纬度',
  `longitude` float(9,6) NOT NULL COMMENT '经度',
  `create_time` datetime default NULL COMMENT '动作时间',
  PRIMARY KEY  (`mstid`),
  KEY `idx_msid` USING BTREE (`mstid`),
  KEY `idx_org_uid` USING BTREE (`org_uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of message_post
-- ----------------------------

-- ----------------------------
-- Table structure for `message_seed`
-- ----------------------------
DROP TABLE IF EXISTS `message_seed`;
CREATE TABLE `message_seed` (
  `msid` bigint(20) unsigned NOT NULL auto_increment COMMENT '消息种子id，每条消息一个网格存储一个',
  `uid` bigint(20) unsigned NOT NULL COMMENT '消息发送者id',
  `mid` bigint(20) unsigned NOT NULL COMMENT '原始消息id',
  `alive` bigint(20) unsigned NOT NULL default '5' COMMENT '消息存活量，每传播一次+seed，消灭一次-1',
  `geohash` char(6) NOT NULL COMMENT '网格geohash',
  `update_time` datetime default NULL COMMENT '更新时间',
  `depth` int(10) unsigned NOT NULL default '0' COMMENT '传播层次',
  `dist` float(8,3) unsigned NOT NULL default '0.000' COMMENT '当前传播距离',
  `total_dist` float(8,3) unsigned NOT NULL default '0.000' COMMENT '传播总距离',
  `create_time` datetime default NULL COMMENT '创建时间',
  `from_msid` bigint(20) unsigned NOT NULL default '0' COMMENT '来自哪个种子id，自身用0表示',
  PRIMARY KEY  (`msid`),
  KEY `idx_mid` USING BTREE (`mid`),
  KEY `idx_geohash` USING HASH (`geohash`),
  KEY `idx_update_time` USING BTREE (`update_time`),
  KEY `idx_uid` USING HASH (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of message_seed
-- ----------------------------

-- ----------------------------
-- Table structure for `msg_blacklist_0`
-- ----------------------------
DROP TABLE IF EXISTS `msg_blacklist_0`;
CREATE TABLE `msg_blacklist_0` (
  `bid` bigint(20) unsigned NOT NULL auto_increment COMMENT '屏蔽列表id',
  `uid` bigint(20) unsigned NOT NULL COMMENT '用户id',
  `mid` bigint(20) unsigned NOT NULL COMMENT '屏蔽消息id',
  PRIMARY KEY  (`bid`),
  UNIQUE KEY `idx_bid` USING HASH (`bid`),
  KEY `idx_uid` USING BTREE (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of msg_blacklist_0
-- ----------------------------

-- ----------------------------
-- Table structure for `msg_blacklist_1`
-- ----------------------------
DROP TABLE IF EXISTS `msg_blacklist_1`;
CREATE TABLE `msg_blacklist_1` (
  `bid` bigint(20) unsigned NOT NULL auto_increment COMMENT '屏蔽列表id',
  `uid` bigint(20) unsigned NOT NULL COMMENT '用户id',
  `mid` bigint(20) unsigned NOT NULL COMMENT '屏蔽消息id',
  PRIMARY KEY  (`bid`),
  UNIQUE KEY `idx_bid` USING BTREE (`bid`),
  KEY `idx_uid` USING BTREE (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of msg_blacklist_1
-- ----------------------------

-- ----------------------------
-- Table structure for `msg_blacklist_2`
-- ----------------------------
DROP TABLE IF EXISTS `msg_blacklist_2`;
CREATE TABLE `msg_blacklist_2` (
  `bid` bigint(20) unsigned NOT NULL auto_increment COMMENT '屏蔽列表id',
  `uid` bigint(20) unsigned NOT NULL COMMENT '用户id',
  `mid` bigint(20) unsigned NOT NULL COMMENT '屏蔽消息id',
  PRIMARY KEY  (`bid`),
  UNIQUE KEY `idx_bid` (`bid`),
  KEY `idx_uid` (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of msg_blacklist_2
-- ----------------------------

-- ----------------------------
-- Table structure for `msg_blacklist_3`
-- ----------------------------
DROP TABLE IF EXISTS `msg_blacklist_3`;
CREATE TABLE `msg_blacklist_3` (
  `bid` bigint(20) unsigned NOT NULL auto_increment COMMENT '屏蔽列表id',
  `uid` bigint(20) unsigned NOT NULL COMMENT '用户id',
  `mid` bigint(20) unsigned NOT NULL COMMENT '屏蔽消息id',
  PRIMARY KEY  (`bid`),
  UNIQUE KEY `idx_bid` (`bid`),
  KEY `idx_uid` (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of msg_blacklist_3
-- ----------------------------

-- ----------------------------
-- Table structure for `msg_blacklist_4`
-- ----------------------------
DROP TABLE IF EXISTS `msg_blacklist_4`;
CREATE TABLE `msg_blacklist_4` (
  `bid` bigint(20) unsigned NOT NULL auto_increment COMMENT '屏蔽列表id',
  `uid` bigint(20) unsigned NOT NULL COMMENT '用户id',
  `mid` bigint(20) unsigned NOT NULL COMMENT '屏蔽消息id',
  PRIMARY KEY  (`bid`),
  UNIQUE KEY `idx_bid` (`bid`),
  KEY `idx_uid` (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of msg_blacklist_4
-- ----------------------------

-- ----------------------------
-- Table structure for `msg_blacklist_5`
-- ----------------------------
DROP TABLE IF EXISTS `msg_blacklist_5`;
CREATE TABLE `msg_blacklist_5` (
  `bid` bigint(20) unsigned NOT NULL auto_increment COMMENT '屏蔽列表id',
  `uid` bigint(20) unsigned NOT NULL COMMENT '用户id',
  `mid` bigint(20) unsigned NOT NULL COMMENT '屏蔽消息id',
  PRIMARY KEY  (`bid`),
  UNIQUE KEY `idx_bid` USING BTREE (`bid`),
  KEY `idx_uid` (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of msg_blacklist_5
-- ----------------------------

-- ----------------------------
-- Table structure for `msg_blacklist_6`
-- ----------------------------
DROP TABLE IF EXISTS `msg_blacklist_6`;
CREATE TABLE `msg_blacklist_6` (
  `bid` bigint(20) unsigned NOT NULL auto_increment COMMENT '屏蔽列表id',
  `uid` bigint(20) unsigned NOT NULL COMMENT '用户id',
  `mid` bigint(20) unsigned NOT NULL COMMENT '屏蔽消息id',
  PRIMARY KEY  (`bid`),
  UNIQUE KEY `idx_bid` (`bid`),
  KEY `idx_uid` (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of msg_blacklist_6
-- ----------------------------

-- ----------------------------
-- Table structure for `msg_blacklist_7`
-- ----------------------------
DROP TABLE IF EXISTS `msg_blacklist_7`;
CREATE TABLE `msg_blacklist_7` (
  `bid` bigint(20) unsigned NOT NULL auto_increment COMMENT '屏蔽列表id',
  `uid` bigint(20) unsigned NOT NULL COMMENT '用户id',
  `mid` bigint(20) unsigned NOT NULL COMMENT '屏蔽消息id',
  PRIMARY KEY  (`bid`),
  UNIQUE KEY `idx_bid` (`bid`),
  KEY `idx_uid` (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Records of msg_blacklist_7
-- ----------------------------

-- ----------------------------
-- Table structure for `user`
-- ----------------------------
DROP TABLE IF EXISTS `user`;
CREATE TABLE `user` (
  `uid` bigint(20) unsigned NOT NULL auto_increment COMMENT '用户id',
  `passwd` char(32) default NULL COMMENT '密码md5',
  `deviceid` varchar(100) NOT NULL default '' COMMENT '设备id，一个设备只能一个临时账户和多个注册账户',
  `phone` char(15) default NULL COMMENT '电话号码，每个用户唯一,初始化时设为NULL',
  `email` varchar(50) NOT NULL COMMENT '邮箱，默认空',
  `gender` tinyint(1) unsigned NOT NULL default '0',
  `birthday` date NOT NULL COMMENT '生日',
  `avada_url` char(55) NOT NULL default 'default/icon.jpg' COMMENT '用户图像',
  `avada_size` char(10) NOT NULL COMMENT '图像尺寸widthxheight',
  `user_name` varchar(20) NOT NULL,
  `reg_source` tinyint(1) unsigned NOT NULL default '0' COMMENT '注册来源',
  `add_time` datetime default NULL COMMENT '注册时间',
  `mod_time` datetime default NULL COMMENT '修改时间',
  `is_login` tinyint(1) unsigned NOT NULL default '0' COMMENT '是否登陆',
  `status` tinyint(1) unsigned NOT NULL default '0' COMMENT '状态，注册或未注册',
  `last_login_time` datetime default NULL COMMENT '最后登陆时间',
  `login_count` int(10) unsigned NOT NULL default '0' COMMENT '登陆次数',
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
  `total_passed_num` bigint(20) unsigned NOT NULL default '0' COMMENT '产生的消息被消灭的总次数',
  `total_alive` bigint(20) unsigned NOT NULL default '0' COMMENT '产生的消息存货数量，每传播一次，增加seed-1个存活量',
  `max_dist` float(8,3) unsigned NOT NULL default '0.000' COMMENT '传播的最远距离',
  `total_dist` double(20,3) unsigned NOT NULL default '0.000' COMMENT '被传播的总距离',
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
