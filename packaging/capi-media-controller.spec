Name:       capi-media-controller
Summary:    Multimedia Controller for player application
Version:    0.0.3
Release:    1
Group:      System/Libraries
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1:    mediacontroller.service
Source2:    mediacontroller.socket
Source1001: media-controller_create_db.sh
Requires(post):  /sbin/ldconfig
Requires(postun):  /sbin/ldconfig
BuildRequires:  cmake
BuildRequires:  sqlite
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(gio-2.0)
BuildRequires:  pkgconfig(sqlite3)
BuildRequires:  pkgconfig(db-util)
BuildRequires:  pkgconfig(aul)
BuildRequires:  pkgconfig(bundle)
BuildRequires:  pkgconfig(libsystemd-daemon)
BuildRequires:  pkgconfig(libtzplatform-config)

%description
A media controller library in SLP C API

%package -n mediacontroller
Summary:    media Controller service server

%description -n mediacontroller
A media controller library in SLP C API

%package devel
Summary:    Multimedia Controller for player Library (DEV)
Group:      Development/Libraries
Requires:   %{name} = %{version}-%{release}

%description devel
A media controller library in SLP C API

%prep
%setup -q

%build
export CFLAGS+=" -Wextra -Wno-array-bounds"
export CFLAGS+=" -Wno-ignored-qualifiers -Wno-unused-parameter -Wshadow"
export CFLAGS+=" -Wwrite-strings -Wswitch-default"
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
export CFLAGS+=" -DGST_EXT_TIME_ANALYSIS -include stdint.h"
%cmake . -DFULLVER=%{version} -DMAJORVER=${MAJORVER}
%__make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

# Daemon & socket activation
mkdir -p %{buildroot}%{_unitdir}
mkdir -p %{buildroot}%{_unitdir}/sockets.target.wants
install -m 644 %{SOURCE1} %{buildroot}%{_unitdir}/mediacontroller.service
install -m 644 %{SOURCE2} %{buildroot}%{_unitdir}/mediacontroller.socket
ln -s ../mediacontroller.socket %{buildroot}%{_unitdir}/sockets.target.wants/mediacontroller.socket

#Create DB
mkdir -p %{buildroot}%{_bindir}
install -m 0775 %{SOURCE1001} %{buildroot}%{_bindir}/media-controller_create_db.sh
mkdir -p %{buildroot}%{TZ_SYS_DB}
sqlite3 %{buildroot}%{TZ_SYS_DB}/.media_controller.db 'PRAGMA journal_mode = PERSIST; PRAGMA user_version=1;'

mkdir -p %{buildroot}/usr/share/license
cp LICENSE.APLv2.0 %{buildroot}/usr/share/license/%{name}

%post -p /sbin/ldconfig
chgrp %TZ_SYS_USER_GROUP %{_bindir}/media-controller_create_db.sh
%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_libdir}/*.so.*
%{_bindir}/media-controller_create_db.sh
#%{_bindir}/*			//disable tests
%manifest capi-media-controller.manifest
%attr(660,system,app) %{TZ_SYS_DB}/.media_controller.db
%attr(660,system,app) %{TZ_SYS_DB}/.media_controller.db-journal
%config(noreplace) %{TZ_SYS_DB}/.media_controller.db
%config(noreplace) %{TZ_SYS_DB}/.media_controller.db-journal
/usr/share/license/%{name}

%files -n mediacontroller
%defattr(-,system,system,-)
%{_bindir}/mediacontroller
%manifest media-controller-service.manifest
%{_unitdir}/mediacontroller.service
%{_unitdir}/mediacontroller.socket
%{_unitdir}/sockets.target.wants/mediacontroller.socket
#change owner
#chown 200:5000 %{TZ_SYS_DB}/.media_controller.db*

%files devel
%defattr(-,root,root,-)
%{_libdir}/*.so
%{_includedir}/media/*.h
%{_libdir}/pkgconfig/capi-media-controller.pc
