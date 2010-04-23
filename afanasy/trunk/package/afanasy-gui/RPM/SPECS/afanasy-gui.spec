# This is RPM spec file:

%define _topdir %(echo $PWD)/RPM

Summary:       Afanasy GUI
License:       GPL
Name:          afanasy-gui
Version:       @VERSION@
Release:       @RELEASE@
Group:         Applications/Graphics

%description
Afanasy GUI.

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

%preun
echo "Afanasy GUI PRE REMOVE:"
procs="afwatch aftalk afmonitor"
for p in $procs; do pkill $p || true; done
exit 0
