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
