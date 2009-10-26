%define version 0.8.20

%define libpath /usr/lib
%ifarch x86_64
  %define libpath /usr/lib64
%endif

Summary: Software update module
Name: openpanel-mod-softwareupdate
Version: %version
Release: 1
License: GPLv2
Group: Development
Source: http://packages.openpanel.com/archive/openpanel-mod-softwareupdate-%{version}.tar.gz
Patch1: openpanel-mod-softwareupdate-00-makefile
BuildRoot: /var/tmp/%{name}-buildroot
Requires: openpanel-core >= 0.8.3
Requires: openpanel-swupd

%description
Software update module
Controls software updates through OpenPanel

%prep
%setup -q -n openpanel-mod-softwareupdate-%version
%patch1 -p0 -b .buildroot

%build
BUILD_ROOT=$RPM_BUILD_ROOT
./configure
make

%install
BUILD_ROOT=$RPM_BUILD_ROOT
rm -rf ${BUILD_ROOT}
mkdir -p ${BUILD_ROOT}/var/opencore/modules/SoftwareUpdate.module
cp -rf ./softwareupdatemodule.app ${BUILD_ROOT}/var/opencore/modules/SoftwareUpdate.module/
ln -sf ./softwareupdatemodule.app/exec ${BUILD_ROOT}/var/opencore/modules/SoftwareUpdate.module/action
cp module.xml ${BUILD_ROOT}/var/opencore/modules/SoftwareUpdate.module/module.xml
cp *.png ${BUILD_ROOT}/var/opencore/modules/SoftwareUpdate.module/
install -m 755 verify ${BUILD_ROOT}/var/opencore/modules/SoftwareUpdate.module/verify

%post
mkdir -p /var/opencore/conf/staging/SoftwareUpdate
chown opencore:authd /var/opencore/conf/staging/SoftwareUpdate

%files
%defattr(-,root,root)
/