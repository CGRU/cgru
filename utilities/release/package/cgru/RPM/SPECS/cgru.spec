# This is RPM spec file:

%define _topdir %(echo $PWD)/RPM

Summary:       CGRU
License:       GPL
Name:          cgru
Version:       @VERSION@
Release:       @RELEASE@
Group:         Applications/Graphics

%description
Description of myrpmtest.

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
