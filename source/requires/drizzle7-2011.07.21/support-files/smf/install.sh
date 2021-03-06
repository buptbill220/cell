#! /bin/pfsh
#  Copyright (C) 2009 Sun Microsystems, Inc.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; version 2 of the License.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

prefix=/usr/local
exec_prefix=${prefix}

grep solaris.smf.value.drizzle /etc/security/auth_attr > /dev/null
if [ $? -ne 0 ]
then
  ed /etc/security/auth_attr > /dev/null <<EOF
a
solaris.smf.value.drizzle:::Change Drizzle value properties::
solaris.smf.manage.drizzle:::Manage Drizzle service states::
.
w
q
EOF
  if [ $? -ne 0 ]
  then
    echo "Failed to add authorization definitions"
    exit 1
  fi
fi

grep solaris.smf.manage.drizzle /etc/security/prof_attr > /dev/null
if [ $? -ne 0 ]
then
  ed /etc/security/prof_attr > /dev/null <<EOF
a
Drizzle Administration::::auths=solaris.smf.manage.drizzle,solaris.smf.value.drizzle
.
w
q
EOF

  if [ $? -ne 0 ]
  then
    echo "Failed to add profile definitions"
    exit 1
  fi
fi

getent group drizzled > /dev/null
if [ $? -ne 0 ]
then
  groupadd drizzled
  if [ $? -ne 0 ]
  then
    echo "Failed to create group drizzled"
    exit 1
  fi
fi

getent passwd drizzled > /dev/null
if [ $? -ne 0 ]
then
  roleadd -c "Drizzle daemon" -d ${prefix}/var/drizzle -g drizzled \
          -A solaris.smf.value.drizzle,solaris.smf.manage.drizzle drizzled
  if [ $? -ne 0 ]
  then
    echo "Failed to create role drizzled"
    exit 1
  fi

  mkdir -p ${prefix}/var/drizzle/data
  mkdir -p ${prefix}/var/drizzle/crash
  chown -R drizzled:drizzled ${prefix}/var/drizzle
fi

/usr/sbin/install -f /lib/svc/method drizzle
if [ $? -ne 0 ]
then
  echo "Failed to install smf startup script"
  exit 1
fi

/usr/sbin/install -f /var/svc/manifest/application -m 0444 drizzle.xml
if [ $? -ne 0 ]
then
  echo "Failed to install smf definition"
  exit 1
fi

svccfg import /var/svc/manifest/application/drizzle.xml
if [ $? -ne 0 ]
then
  echo "Failed to import smf definition"
  exit 1
fi

