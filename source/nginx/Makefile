
default:	build

clean:
	rm -rf Makefile objs

build:
	$(MAKE) -f objs/Makefile
	$(MAKE) -f objs/Makefile manpage

install:
	$(MAKE) -f objs/Makefile install

upgrade:
	/home/app/web/sbin/app-server -t

	kill -USR2 `cat /home/app/web/data/logs/nginx.pid`
	sleep 1
	test -f /home/app/web/data/logs/nginx.pid.oldbin

	kill -QUIT `cat /home/app/web/data/logs/nginx.pid.oldbin`
