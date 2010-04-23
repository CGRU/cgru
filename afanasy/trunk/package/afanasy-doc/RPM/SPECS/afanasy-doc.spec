# This is RPM spec file:

%define _topdir %(echo $PWD)/RPM

Summary:       Afanasy documentation
License:       GPL
Name:          afanasy-doc
Version:       @VERSION@
Release:       @RELEASE@
Group:         Applications/Graphics

%description
Afanasy documentation.

%prep

%build

%install
cd ../..
mv opt $RPM_BUILD_ROOT
mv usr $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/opt
/usr

%clean
