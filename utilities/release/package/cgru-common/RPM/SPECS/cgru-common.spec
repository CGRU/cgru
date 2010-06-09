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
dirs="etc usr opt"
for dir in $dirs; do
   mkdir -p $RPM_BUILD_ROOT/$dir
   mv $dir/* $RPM_BUILD_ROOT/$dir
done

%files
%defattr(-,root,root)
/etc
/usr
/opt/cgru/bin
/opt/cgru/doc
/opt/cgru/lib
/opt/cgru/config_default.xml
/opt/cgru/setup.sh
%config /opt/cgru/config.xml

%clean

%post
echo "CGRU common POST INSTALL"
pydir="/opt/cgru/lib/python"
[ -d "${pydir}" ] && chmod a+rwx "${pydir}"
exit 0

%preun
echo "CGRU common PRE REMOVE: $1"
[ "$1" != "0" ] && exit 0
echo "Cleaning cgru/lib/python"
pydir="/opt/cgru/lib/python"
[ -d "${pydir}" ] && rm -rf "${pydir}"
exit 0
