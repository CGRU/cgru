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
