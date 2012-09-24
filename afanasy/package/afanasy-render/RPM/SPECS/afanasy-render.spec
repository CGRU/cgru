# This is RPM spec file:

%define _topdir %(echo $PWD)/RPM

Summary:       Afanasy render client
License:       LGPL
Name:          afanasy-render
Version:       @VERSION@
Release:       @RELEASE@
Group:         Applications/Graphics
URL:           http://cgru.info/

Requires:      afanasy-common = @VERSION@

%description
Afanasy render init sctipts. Create user "render" if does not exists.

%prep

%build

%install
mkdir -p $RPM_BUILD_ROOT

%files
%defattr(-,root,root)

%clean

%pre
echo "Afanasy render PRE INSTALL"
if [ -e "/etc/init.d/afrender" ]; then
   echo "Trying to stop previously installed service..."
   "/etc/init.d/afrender" stop || true
fi
exit 0

%post
echo "Afanasy render POST INSTALL"
id render || useradd render -m
/opt/cgru/afanasy/init/initlinks.sh c add afrender || true
"/etc/init.d/afrender" start || true
exit 0

%preun
echo "Afanasy render PRE REMOVE: $1"
[ "$1" != "0" ] && exit 0
"/etc/init.d/afrender stop" || true
/opt/cgru/afanasy/init/initlinks.sh c rm afrender || true
exit 0
