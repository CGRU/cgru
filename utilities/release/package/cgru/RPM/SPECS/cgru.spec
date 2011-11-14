# This is RPM spec file:

%define _topdir %(echo $PWD)/RPM

Summary:       CGRU
License:       GPL
Name:          cgru
Version:       @VERSION@
Release:       @RELEASE@
Group:         Applications/Graphics

Requires:      cgru-common = @VERSION@, afanasy-render = @VERSION@, afanasy-common = @VERSION@, PyQt4
AutoReqProv:   no

%description
Description of myrpmtest.

%prep

%build

%install
cd ../..
dirs="usr"
for dir in $dirs; do
   mkdir -p $RPM_BUILD_ROOT/$dir
   mv $dir/* $RPM_BUILD_ROOT/$dir
done

%files
%defattr(-,root,root)
/usr

%clean

%post

%preun
