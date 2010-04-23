# This is RPM spec file:

%define _topdir %(echo $PWD)/RPM

Summary:       Afanasy examples
License:       GPL
Name:          afanasy-examples
Version:       @VERSION@
Release:       @RELEASE@
Group:         Applications/Graphics

%description
Afanasy examples.

%prep

%build

%install
cd ../..
mv opt $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/opt

%clean
