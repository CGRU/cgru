# This is RPM spec file:

%define _topdir %(echo $PWD)/RPM

Summary:       Afanasy binary files
License:       LGPL
Name:          afanasy-common
Version:       @VERSION@
Release:       @RELEASE@
Group:         Applications/Graphics
URL:           http://cgru.info/

Requires:      cgru-common = @VERSION@

%description
Afanasy binary files.

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
pkill afwatch || true
exit 0
