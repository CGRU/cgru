# This is RPM spec file:

%define _topdir %(echo $PWD)/RPM

Summary:       CGRU Common
License:       LGPL
Name:          cgru-common
Version:       @VERSION@
Release:       @RELEASE@
Group:         Applications/Graphics
URL:           http://cgru.info/

AutoReqProv:   no

%description
CGRU files.

%prep

%build

%install
cd ../..
dirs="opt"
for dir in $dirs; do
   mkdir -p $RPM_BUILD_ROOT/$dir
   mv $dir/* $RPM_BUILD_ROOT/$dir
done

%files
%defattr(-,root,root)
/opt

%clean

%post
echo "CGRU-common post install:"
find /opt/cgru -type d -exec chmod a+rwx {} \;
exit 0

%preun
echo "CGRU-common pre remove: $1"
[ "$1" != "0" ] && exit 0
/opt/cgru/utilities/keeper/cmdkeeper.sh quit
echo "Cleaning CGRU"
[ -d /opt/cgru ] && find /opt/cgru -type f -name *.pyc -exec rm -vf {} \;
[ -d /opt/cgru ] && find /opt/cgru -type d -name __pycache__ -exec rm -rvf {} \;
/opt/cgru/examples/clear.sh
exit 0
