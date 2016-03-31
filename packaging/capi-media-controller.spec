Name:       capi-media-controller
Summary:    A media controller library in Tizen Native API
Version:    0.0.17
Release:    1
Group:      Multimedia/API
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1:    mediacontroller.service
Source2:    mediacontroller.socket
Source3:    media-controller-user.service
Source1001: media-controller_create_db.sh
BuildRequires:  cmake
BuildRequires:  sqlite
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(dbus-glib-1)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(gio-2.0)
BuildRequires:  pkgconfig(sqlite3)
BuildRequires:  pkgconfig(db-util)
BuildRequires:  pkgconfig(aul)
BuildRequires:  pkgconfig(bundle)
BuildRequires:  pkgconfig(libsystemd-daemon)
BuildRequires:  pkgconfig(libtzplatform-config)
BuildRequires:  pkgconfig(cynara-client)
BuildRequires:  pkgconfig(cynara-session)
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig

%description
This package provides a media controller library in Tizen Native API

%package -n mediacontroller
Summary:    A media controller service for media controller library
Group:      Multimedia/Service

%description -n mediacontroller
This packeage provides media controller service for media controller library

%package devel
Summary:    A media controller library in Tizen Native API (Development)
Group:      Multimedia/Development
Requires:   %{name} = %{version}-%{release}

%description devel
This package provides a media controller library in Tizen Native API(Development files included)

%prep
%setup -q

%build
export CFLAGS+=" -Wextra -Wno-array-bounds"
export CFLAGS+=" -Wno-ignored-qualifiers -Wno-unused-parameter -Wshadow"
export CFLAGS+=" -Wwrite-strings -Wswitch-default"
export CFLAGS+=" -DGST_EXT_TIME_ANALYSIS -include stdint.h"
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . -DFULLVER=%{version} -DMAJORVER=${MAJORVER}

%__make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install
mkdir -p %{buildroot}/%{_datadir}/license
cp -rf %{_builddir}/%{name}-%{version}/LICENSE.APLv2.0 %{buildroot}/%{_datadir}/license/%{name}
cp -rf %{_builddir}/%{name}-%{version}/LICENSE.APLv2.0 %{buildroot}/%{_datadir}/license/mediacontroller

# Daemon & socket activation
mkdir -p %{buildroot}%{_unitdir}
mkdir -p %{buildroot}%{_unitdir}/sockets.target.wants
# change
install -m 644 %{SOURCE1} %{buildroot}%{_unitdir}/mediacontroller.service
install -m 644 %{SOURCE2} %{buildroot}%{_unitdir}/mediacontroller.socket
ln -s ../mediacontroller.socket %{buildroot}%{_unitdir}/sockets.target.wants/mediacontroller.socket

# Setup DB creation in user session
mkdir -p %{buildroot}%{_unitdir_user}
mkdir -p %{buildroot}%{_unitdir_user}/default.target.wants/
install -m 644 %{SOURCE3} %{buildroot}%{_unitdir_user}/media-controller-user.service
ln -s ../media-controller-user.service %{buildroot}%{_unitdir_user}/default.target.wants/media-controller-user.service

# Create DB for multi-user
mkdir -p %{buildroot}%{_bindir}
install -m 0775 %{SOURCE1001} %{buildroot}%{_bindir}/media-controller_create_db.sh

%post -p /sbin/ldconfig
chgrp %TZ_SYS_USER_GROUP %{_bindir}/media-controller_create_db.sh
%postun -p /sbin/ldconfig

%files
%manifest %{name}.manifest
%{_bindir}/media-controller_create_db.sh
%defattr(-,root,root,-)
%{_libdir}/*.so.*
%{_datadir}/license/%{name}

%files -n mediacontroller
%defattr(-,root,root,-)
%{_bindir}/mediacontroller
%manifest media-controller-service.manifest
%defattr(-,system,system,-)
%{_unitdir}/mediacontroller.service
%{_unitdir}/mediacontroller.socket
%{_unitdir}/sockets.target.wants/mediacontroller.socket
%{_unitdir_user}/media-controller-user.service
%{_unitdir_user}/default.target.wants/media-controller-user.service
%{_datadir}/license/mediacontroller

%files devel
%{_libdir}/*.so
%{_includedir}/media/*.h
%{_libdir}/pkgconfig/capi-media-controller.pc
