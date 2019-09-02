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
systemctl stop afrender || true
systemctl disable afrender || true
systemctl daemon-reload || true
exit 0

%post
echo "Afanasy render POST INSTALL"
id render || useradd render -m
systemctl enable /opt/cgru/afanasy/systemd/afrender.service || true
systemctl daemon-reload || true
systemctl start afrender.service || true
exit 0

%preun
echo "Afanasy render PRE REMOVE: $1"
[ "$1" != "0" ] && exit 0
systemctl stop afrender || true
systemctl disable afrender || true
systemctl daemon-reload || true
exit 0
