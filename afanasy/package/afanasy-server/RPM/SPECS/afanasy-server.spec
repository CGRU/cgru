# This is RPM spec file:

%define _topdir %(echo $PWD)/RPM

Summary:       Afanasy server
License:       LGPL
Name:          afanasy-server
Version:       @VERSION@
Release:       @RELEASE@
Group:         Applications/Graphics
URL:           http://cgru.info/

Requires:      afanasy-common = @VERSION@

%description
Afanasy server init sctipts. Create user "render" if does not exists.

%prep

%build

%install
mkdir -p $RPM_BUILD_ROOT

%files
%defattr(-,root,root)

%clean

%pre
echo "Afanasy server PRE INSTALL"
systemctl stop afserver || true
systemctl disable afserver || true
systemctl daemon-reload || true
exit 0

%post
echo "Afanasy server POST INSTALL"
id render || useradd render -m
systemctl enable /opt/cgru/afanasy/systemd/afserver.service || true
systemctl daemon-reload || true
systemctl start afserver.service || true
exit 0

%preun
echo "Afanasy server PRE REMOVE: $1"
[ "$1" != "0" ] && exit 0
systemctl stop afserver || true
systemctl disable afserver || true
systemctl daemon-reload || true
exit 0
