# This is RPM spec file:

%define _topdir %(echo $PWD)/RPM

Summary:       Afanasy plugins
License:       GPL
Name:          afanasy-plugins
Version:       @VERSION@
Release:       @RELEASE@
Group:         Applications/Graphics

%description
Afanasy plugins.

%prep

%build

%install
cd ../..
mv opt $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/opt

%clean
