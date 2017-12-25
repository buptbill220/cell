#!/bin/sh

ARGV="$@"
NGINX="/home/app/web/sbin/app-server"

case $ARGV in
    start)
    echo "starting nginx"
    $NGINX
    ERROR=$?
    ;;
    stop|reload)
    echo $ARGV"ing nginx"
    $NGINX -s $ARGV
    ERROR=$?
    ;;
    restart)
    echo $ARGV"ing nginx"
    $NGINX -s stop
    ERROR=$?
    $NGINX
    ERROR=$?
    ;;
    rotate)
    echo "rotating"
    $NGINX -s reopen
    ERROR=$?
    ;;
    configtest)
    $NGINX -t
    ERROR=$?
    ;;
    *)
    echo $"Usage: $0 {start|stop|reload|restart|rotate|configtest}"
    exit 2
esac

exit  $ERROR
