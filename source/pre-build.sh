if [ $# -eq 0 ] ; then
    optflags="-O2 -g -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions -fstack-protector --param=ssp-buffer-size=4 -m64 -mtune=generic";
fi
if [ $# -ge 1 ] ;then
    optflags=$1;
fi

ts=$(dirname $(readlink -f $0));
cd $ts/mod_thirdParty/LuaJIT/src;
touch buildvm_*.h;
cd $ts/mod_thirdParty/LuaJIT;
make
luajit_root=$ts/mod_thirdParty/luajit_setup;
rm -rf $luajit_root && mkdir -p $luajit_root;
make install PREFIX=$luajit_root/luajit

export LUAJIT_LIB=$luajit_root/luajit/lib
export LUAJIT_INC=$luajit_root/luajit/include/luajit-2.1
export DESTDIR=$luajit_root
export WITH_TAIR_LUA=1
export DESTDIR=$luajit_root

# lua-redis-parser, change to static linking
cd $ts/../mod_thirdParty/lua-redis-parser-0.10;
make LUA_INCLUDE_DIR=$luajit_root/luajit/include/luajit-2.1
make install LUA_LIB_DIR=/luajit/lib

SVN_PROTO_PATH="svn://10.162.211.161/proto"
PROTO_VERSION="-r 98"
PROTO_PATH="./mod_app-server/proto"
cd $ts;
svn export $PROTO_VERSION $SVN_PROTO_PATH/common_data.proto $PROTO_PATH/common_data.proto
svn export $PROTO_VERSION $SVN_PROTO_PATH/initial.proto $PROTO_PATH/initial.proto
svn export $PROTO_VERSION $SVN_PROTO_PATH/account.proto $PROTO_PATH/account.proto
svn export $PROTO_VERSION $SVN_PROTO_PATH/ugc.proto $PROTO_PATH/ugc.proto
protoc $PROTO_PATH/common_data.proto --cpp_out=$PROTO_PATH -I$PROTO_PATH
protoc $PROTO_PATH/initial.proto --cpp_out=$PROTO_PATH -I$PROTO_PATH
protoc $PROTO_PATH/account.proto --cpp_out=$PROTO_PATH -I$PROTO_PATH
protoc $PROTO_PATH/ugc.proto --cpp_out=$PROTO_PATH -I$PROTO_PATH

root=/home/app/web;
name=app-server;
cd $ts/nginx;
./configure \
    --user=app \
    --group=users \
    --prefix=$root/data \
    --sbin-path=$root/sbin/$name \
    --conf-path=$root/conf/$name.conf \
    --lock-path=$root/sbin/$name.lock \
    --http-log-path=$root/logs/access.log \
    --error-log-path=$root/logs/error.log \
    --with-http_stub_status_module \
    --with-http_ssl_module \
    --with-ld-opt="-Wl,-rpath,/usr/local/lib" \
    --with-pcre=/home/thirds/pcre-8.36 \
    --with-zlib=/home/thirds/zlib-1.2.8 \
    --without-select_module \
    --without-poll_module \
    --without-http_userid_module \
    --without-http_auth_basic_module \
    --without-http_autoindex_module \
    --without-http_map_module \
    --without-http_memcached_module \
    --without-http_empty_gif_module \
    --without-http_uwsgi_module \
    --without-http_scgi_module \
    --without-http_split_clients_module \
    --add-module=../mod_thirdParty/ngx_lua-0.9.13 \
    --add-module=../mod_thirdParty/ngx_devel_kit-0.2.19 \
    --add-module=../mod_thirdParty/drizzle-nginx-module-0.1.8 \
    --add-module=../mod_thirdParty/echo-nginx-module-0.57 \
    --add-module=../mod_thirdParty/rds-json-nginx-module-0.13 \
    --add-module=../mod_thirdParty/set-misc-nginx-module-0.27 \
    --add-module=../mod_thirdParty/redis2-nginx-module-0.11 \
    --add-module=../mod_app-server/ \
    --add-module=../mod_app-server/app_common/ \
    --add-module=../mod_app-server/common/ \
    --add-module=../mod_app-server/proto/ \
    --add-module=../mod_app-server/user/ \
    --add-module=../mod_app-server/ugc/ \
    --with-cc-opt="$optflags -Werror-implicit-function-declaration";

