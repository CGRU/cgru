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
Afanasy server SystemD sctipts. Create user "render" if does not exists.

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
/usr/lib/systemd/system/afserver.service

%clean

%pre
echo "Afanasy server PRE INSTALL"
systemctl stop afserver || true
systemctl disable afserver || true
systemctl daemon-reload || true
exit 0

%post
echo "Afanasy server POST INSTALL"
if ! id render; then
	if getent group render; then
		useradd render -g render -m
	else
		useradd render -m
	fi
fi
systemctl enable afserver.service || true
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
