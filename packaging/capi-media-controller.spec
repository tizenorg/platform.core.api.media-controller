Name:       capi-media-controller
Summary:    A media controller library in Tizen Native API
Version:    0.1.19
Release:    1
Group:      Multimedia/API
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1:    mediacontroller.service
Source2:    mediacontroller.socket
Source3:    media-controller-user.service
Source4:    mediacontroller-ipc.socket
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

%if 0%{?TIZEN_PRODUCT_TV}
%define multi_user 0
%else
%define multi_user 1
%endif

%prep
%setup -q

%build
export CFLAGS+=" -Wextra -Wno-array-bounds"
export CFLAGS+=" -Wno-ignored-qualifiers -Wno-unused-parameter -Wshadow"
export CFLAGS+=" -Wwrite-strings -Wswitch-default"
export CFLAGS+=" -DGST_EXT_TIME_ANALYSIS -include stdint.h"
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
%cmake . -DFULLVER=%{version} \
%if 0%{?multi_user}
 -DMAJORVER=${MAJORVER} -DMULTI_USER=YES
%else
 -DMAJORVER=${MAJORVER}
%endif

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

# Setup DB creation in user session
%if 0%{?multi_user}
mkdir -p %{buildroot}%{_unitdir_user}
mkdir -p %{buildroot}%{_unitdir_user}/default.target.wants/
install -m 644 %{SOURCE3} %{buildroot}%{_unitdir_user}/media-controller-user.service
ln -s ../media-controller-user.service %{buildroot}%{_unitdir_user}/default.target.wants/media-controller-user.service
%endif

# Create DB
%if 0%{?multi_user}
mkdir -p %{buildroot}%{_bindir}
install -m 0775 %{SOURCE1001} %{buildroot}%{_bindir}/media-controller_create_db.sh
%endif

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
%if 0%{?multi_user}
%{_bindir}/media-controller_create_db.sh
%endif
%manifest media-controller-service.manifest
%defattr(-,system,system,-)
%{_unitdir}/mediacontroller.service
%{_unitdir}/mediacontroller.socket
%{_unitdir}/sockets.target.wants/mediacontroller.socket
%{_unitdir}/mediacontroller-ipc.socket
%{_unitdir}/sockets.target.wants/mediacontroller-ipc.socket
%if 0%{?multi_user}
%{_unitdir_user}/media-controller-user.service
%{_unitdir_user}/default.target.wants/media-controller-user.service
%endif
%{_datadir}/license/mediacontroller

%files devel
%{_libdir}/*.so
%{_includedir}/media/*.h
%{_libdir}/pkgconfig/capi-media-controller.pc
