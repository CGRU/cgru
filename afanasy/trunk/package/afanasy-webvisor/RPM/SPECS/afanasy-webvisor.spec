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
mv opt $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/opt

%clean
