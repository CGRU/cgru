# This is RPM spec file:

%define _topdir %(echo $PWD)/RPM

Summary:       Afanasy WEB visor
License:       GPL
Name:          afanasy-webvisor
Version:       @VERSION@
Release:       @RELEASE@
Group:         Applications/Graphics

%description
Afanasy WEB visor.

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
