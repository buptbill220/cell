Name:       app-server
Version:    0.0.1
Release:    %(echo $RELEASE)%{?dist}
Packager:   fangming
Summary:    mysql midware web
Vendor:     Cell Project
Group:      New Boys
URL:        None
License:    Commercial
BuildRoot:  %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
AutoReqProv:    no
BuildRequires:  pcre-devel, zlib-devel, openssl-devel
Requires:       pcre, zlib, openssl

%define _prefix /home/app
%define debug_package 1 

%description
Nginx [engine x] is an HTTP(S) server, HTTP(S) reverse proxy and IMAP/POP3
proxy server written by Igor Sysoev.

%build
cd ../../../
sh pre-build.sh "%{optflags}"

#_smp_mflags
# -j24
make %{?_smp_mflags}

%install
mkdir -p %{_builddir}/%{_prefix}/web/sbin/;
mkdir -p %{_builddir}/%{_prefix}/web/conf/;
mkdir -p %{_builddir}/%{_prefix}/web/logs/;
mkdir -p %{_builddir}/%{_prefix}/web/data/;
mkdir -p %{_builddir}/%{_prefix}/web/scripts/;

cp $OLDPWD/../nginx/objs/nginx              %{_builddir}/%{_prefix}/web/sbin/app-server;
cp $OLDPWD/../scripts/run.sh                %{_builddir}/%{_prefix}/web/sbin/;
cp $OLDPWD/../nginx/conf/*                  %{_builddir}/%{_prefix}/web/conf/;
cp $OLDPWD/../conf/app-server.conf          %{_builddir}/%{_prefix}/web/conf/;
cp $OLDPWD/../conf/redis_raw_query.lua      %{_builddir}/%{_prefix}/web/conf/;
cp $OLDPWD/../conf/redis_set_query.lua      %{_builddir}/%{_prefix}/web/conf/;
cp $OLDPWD/../conf/redis_get_query.lua      %{_builddir}/%{_prefix}/web/conf/;
cp $OLDPWD/../conf/check.lua                %{_builddir}/%{_prefix}/web/conf/;
cp $OLDPWD/../conf/check_session.lua        %{_builddir}/%{_prefix}/web/conf/;
cp $OLDPWD/../conf/user.lua                 %{_builddir}/%{_prefix}/web/conf/;
cp $OLDPWD/../conf/ugc.lua                  %{_builddir}/%{_prefix}/web/conf/;
cp $OLDPWD/../conf/app-server.lua           %{_builddir}/%{_prefix}/web/conf/;
cp $OLDPWD/../scripts/rlog.sh               %{_builddir}/%{_prefix}/web/scripts/;

#echo '%debug_package %{nil}' >> ~/.rpmmacros;
#/usr/lib/rpm/find-debuginfo.sh $OLDPWD/../nginx
%post

%preun

%postun

%clean

%files

%defattr(-,app,users)
%{_prefix}
%attr(755,app,users) %{_builddir}%{_prefix}/web/sbin/app-server

%changelog
*  Wed Feb 3 2015 fangming 0.0.1
-  modify it
*  Thu Jan 22 2015 jimmy.gj 0.0.1
-  create it
