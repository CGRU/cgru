# This is RPM spec file:

%define _topdir %(echo $PWD)/RPM

Summary:       Afanasy plugins
License:       GPL
Name:          afanasy-plugins
Version:       @VERSION@
Release:       @RELEASE@
Group:         Applications/Graphics

Requires:      afanasy-common = @VERSION@, cgru-common = @VERSION@

%description
Afanasy plugins.

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
echo "Afanasy plugins POST INSTALL"
[ -d /opt/cgru/afanasy/plugins ] && find /opt/cgru/afanasy/plugins -type d -exec chmod a+rwx {} \;
exit 0

%preun
echo "Afanasy plugins PRE REMOVE:"
[ "$1" != "0" ] && exit 0
pkill afcmd || true
echo "Cleaning afanasy/plugins"
[ -d /opt/cgru/afanasy/plugins ] && find /opt/cgru/afanasy/plugins -type f -name *.pyc -exec rm -vf {} \;
exit 0
