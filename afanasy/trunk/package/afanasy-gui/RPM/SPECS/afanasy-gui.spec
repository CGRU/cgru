# This is RPM spec file:

%define _topdir %(echo $PWD)/RPM

Summary:       Afanasy GUI
License:       GPL
Name:          afanasy-gui
Version:       @VERSION@
Release:       @RELEASE@
Group:         Applications/Graphics

Requires:      cgru-common = @VERSION@, afanasy-common = @VERSION@

%description
Afanasy GUI.

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

%preun
echo "Afanasy GUI PRE REMOVE:"
procs="afwatch aftalk afmonitor"
for p in $procs; do pkill $p || true; done
exit 0
