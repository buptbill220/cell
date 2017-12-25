#!/bin/bash

SRC_ERR_LOG=/home/a/opendsp/ef/logs/error.log
SRC_ACC_PV_LOG=/home/a/opendsp/ef/logs/ef_log
DST_PV_ROOT="/home/a/opendsp/ef/logs/"
DST_LOG_ROOT="/home/a/opendsp/ef/logs/cronolog"

YEAR_MON=$(date -d -1hour +"%Y")/$(date -d -1hour +"%m")
YEAR_MON_DAY=`date -d -1hour +%Y-%m-%d`
YEAR_MON_DAY_HOUR=`date -d -1hour +%Y-%m-%d-%H`

DST_ERR_LOG=${DST_LOG_ROOT}/${YEAR_MON}/${YEAR_MON_DAY}-error_log
DST_ACC_PV_LOG=${DST_PV_ROOT}/ef_log.${YEAR_MON_DAY_HOUR}

mkdir -p ${DST_LOG_ROOT}/${YEAR_MON}
chown ads ${DST_LOG_ROOT} -R

mv ${SRC_ACC_PV_LOG} ${DST_ACC_PV_LOG}

if [ `date +%Y-%m-%d` != $YEAR_MON_DAY ] 
then
    mv ${SRC_ERR_LOG} ${DST_ERR_LOG}
fi

kill -USR1 `cat /home/a/opendsp/ef/sbin/t-opendsp-ef.pid`

if [ `date +%Y-%m-%d` != $YEAR_MON_DAY ] 
then
    gzip -c ${DST_ERR_LOG} > ${DST_ERR_LOG}.gz
    rm ${DST_ERR_LOG}
    chown ads ${DST_ERR_LOG}.gz
fi
