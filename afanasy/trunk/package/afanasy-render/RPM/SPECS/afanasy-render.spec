# This is RPM spec file:

%define _topdir %(echo $PWD)/RPM

Summary:       Afanasy render client
License:       GPL
Name:          afanasy-render
Version:       @VERSION@
Release:       @RELEASE@
Group:         Applications/Graphics

%description
Afanasy render client.

%prep

%build

%install
cd ../..
mv opt $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
/opt

%clean

%pre
echo "Afanasy render PRE INSTALL"
if [ -e /etc/init.d/afrender ]; then
   echo "Trying to stop previously installed service..."
   /etc/init.d/afrender stop || true
fi
exit 0

%post
echo "Afanasy render POST INSTALL"
id renderer || useradd renderer --create-home
/opt/cgru/afanasy/init/initlinks.sh c add afrender || true
/etc/init.d/afrender start || true
exit 0

%preun
echo "Afanasy render PRE REMOVE:"
/etc/init.d/afrender stop || true
/opt/cgru/afanasy/init/initlinks.sh c rm afrender || true
exit 0
