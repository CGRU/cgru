# This is RPM spec file:

%define _topdir %(echo $PWD)/RPM

Summary:       Afanasy examples
License:       GPL
Name:          afanasy-examples
Version:       @VERSION@
Release:       @RELEASE@
Group:         Applications/Graphics

Requires:      afanasy-plugins = @VERSION@

%description
Afanasy examples.

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
