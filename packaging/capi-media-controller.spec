Name:       capi-media-controller
Summary:    Multimedia Controller for player application
Version:    0.0.1
Release:    1
Group:      System/Libraries
License:    Apache-2.0
Source0:    %{name}-%{version}.tar.gz
Source1:    mediacontroller.service
Requires(post):  /sbin/ldconfig
Requires(postun):  /sbin/ldconfig
BuildRequires:  cmake
BuildRequires:  pkgconfig(capi-base-common)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(glib-2.0)
BuildRequires:  pkgconfig(gio-2.0)
BuildRequires:  pkgconfig(sqlite3)
BuildRequires:  pkgconfig(db-util)
BuildRequires:  pkgconfig(aul)
BuildRequires:  pkgconfig(bundle)
BuildRequires:  pkgconfig(security-server)

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
MAJORVER=`echo %{version} | awk 'BEGIN {FS="."}{print $1}'`
export CFLAGS+=" -DGST_EXT_TIME_ANALYSIS -include stdint.h"

%if 0%{?sec_build_binary_debug_enable}
export CFLAGS+=" -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS+=" -DDTIZEN_DEBUG_ENABLE"
export FFLAGS+=" -DTIZEN_DEBUG_ENABLE"
%endif

cmake . -DCMAKE_INSTALL_PREFIX=/usr -DFULLVER=%{version} -DMAJORVER=${MAJORVER}

make %{?jobs:-j%jobs}

%install
rm -rf %{buildroot}
%make_install

mkdir -p %{buildroot}/usr/lib/systemd/system/multi-user.target.wants
install -m 644 %{SOURCE1} %{buildroot}/usr/lib/systemd/system/mediacontroller.service
ln -s ../mediacontroller.service %{buildroot}/usr/lib/systemd/system/multi-user.target.wants/mediacontroller.service

#Create DB
mkdir -p %{buildroot}/opt/usr/dbspace
sqlite3 %{buildroot}/opt/usr/dbspace/.media_controller.db 'PRAGMA journal_mode = PERSIST; PRAGMA user_version=1;'

mkdir -p %{buildroot}%{_libdir}/systemd/system/multi-user.target.wants
mkdir -p %{buildroot}/usr/share/license
cp LICENSE.APLv2.0 %{buildroot}/usr/share/license/%{name}

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%{_libdir}/*.so.*
#%{_bindir}/*			//disable tests
%manifest capi-media-controller.manifest
/usr/share/license/%{name}

%files -n mediacontroller
%defattr(-,root,root,-)
%{_bindir}/mediacontroller
%manifest media-controller-service.manifest
/usr/lib/systemd/system/mediacontroller.service
/usr/lib/systemd/system/multi-user.target.wants/mediacontroller.service
#change owner
#chown 200:5000 /opt/usr/dbspace/.media_controller.db*
%attr(660,system,app) /opt/usr/dbspace/.media_controller.db
%attr(660,system,app) /opt/usr/dbspace/.media_controller.db-journal
%config(noreplace) /opt/usr/dbspace/.media_controller.db
%config(noreplace) /opt/usr/dbspace/.media_controller.db-journal
/usr/share/license/%{name}

%files devel
%defattr(-,root,root,-)
%{_libdir}/*.so
%{_includedir}/media/*.h
%{_libdir}/pkgconfig/capi-media-controller.pc
