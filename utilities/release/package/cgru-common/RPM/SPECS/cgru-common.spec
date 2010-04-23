# This is RPM spec file:

%define _topdir %(echo $PWD)/RPM

Summary:       CGRU Common
License:       GPL
Name:          cgru-common
Version:       @VERSION@
Release:       @RELEASE@
Group:         Applications/Graphics

%description
Description of myrpmtest.

%prep

%build

%install
cd ../..
mv etc $RPM_BUILD_ROOT
mv usr $RPM_BUILD_ROOT
mv opt $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/etc
/usr
/opt
%config /opt/cgru/config.xml

%clean

%post
echo "CGRU common POST INSTALL"
pydir="/opt/cgru/lib/python"
[ -d "${pydir}" ] && chmod a+rwx "${pydir}"
exit 0

%preun
echo "CGRU common PRE REMOVE:"
pydir="/opt/cgru/lib/python"
[ -d "${pydir}" ] && rm -rf "${pydir}"
exit 0
