# This is RPM spec file:

%define _topdir %(echo $PWD)/RPM

Summary:       Afanasy documentation
License:       GPL
Name:          afanasy-doc
Version:       @VERSION@
Release:       @RELEASE@
Group:         Applications/Graphics

Requires:      cgru-common = @VERSION@

%description
Afanasy documentation.

%prep

%build

%install
cd ../..
dirs="usr opt"
for dir in $dirs; do
   mkdir -p $RPM_BUILD_ROOT/$dir
   mv $dir/* $RPM_BUILD_ROOT/$dir
done

%files
%defattr(-,root,root)
/opt
/usr

%clean
