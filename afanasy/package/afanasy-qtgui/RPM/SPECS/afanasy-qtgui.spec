# This is RPM spec file:

%define _topdir %(echo $PWD)/RPM

Summary:       Afanasy Qt GUI files
License:       LGPL
Name:          afanasy-qtgui
Version:       @VERSION@
Release:       @RELEASE@
Group:         Applications/Graphics
URL:           http://cgru.info/

Requires:      cgru-common = @VERSION@

%description
Afanasy Qt GUI files.

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

%preun
echo "Afanasy common pre remove, exiting Afanasy applications: $1"
[ "$1" != "0" ] && exit 0
for p in afcmd afwatch aftalk afmonitor; do pkill $p || true; done
exit 0
