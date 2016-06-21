Name:       capi-media-controller
Summary:    A media controller library in Tizen Native API
Version:    0.1.23
Release:    1
Group:      Multimedia/API
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1:    mediacontroller.service
Source2:    mediacontroller.socket
Source3:    media-controller-user.service
Source4:    mediacontroller-ipc.socket
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
BuildRequires:  pkgconfig(libsystemd-login)
BuildRequires:  pkgconfig(libtzplatform-config)
BuildRequires:  pkgconfig(cynara-client)
BuildRequires:  pkgconfig(cynara-session)

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
install -m 644 %{SOURCE4} %{buildroot}%{_unitdir}/mediacontroller-ipc.socket
ln -s ../mediacontroller.socket %{buildroot}%{_unitdir}/sockets.target.wants/mediacontroller.socket
ln -s ../mediacontroller-ipc.socket %{buildroot}%{_unitdir}/sockets.target.wants/mediacontroller-ipc.socket

%post

%postun

%files
%manifest %{name}.manifest
%defattr(-,root,root,-)
%{_libdir}/*.so.*
%{_datadir}/license/%{name}

%files -n mediacontroller
%defattr(-,root,root,-)
%{_bindir}/mediacontroller
%manifest media-controller-service.manifest
%defattr(-,multimedia_fw,multimedia_fw,-)
%{_unitdir}/mediacontroller.service
%{_unitdir}/mediacontroller.socket
%{_unitdir}/sockets.target.wants/mediacontroller.socket
%{_unitdir}/mediacontroller-ipc.socket
%{_unitdir}/sockets.target.wants/mediacontroller-ipc.socket
%{_datadir}/license/mediacontroller

%files devel
%{_libdir}/*.so
%{_includedir}/media/*.h
%{_libdir}/pkgconfig/capi-media-controller.pc
