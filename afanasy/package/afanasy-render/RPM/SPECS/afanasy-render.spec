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
Afanasy render SystemD sctipts. Create user "render" if does not exists.

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
/usr/lib/systemd/system/afrender.service

%clean

%pre
echo "Afanasy render PRE INSTALL"
systemctl stop afrender || true
systemctl disable afrender || true
systemctl daemon-reload || true
exit 0

%post
echo "Afanasy render POST INSTALL"
if ! id render; then
	if getent group render; then
		useradd render -g render -m
	else
		useradd render -m
	fi
fi
systemctl enable afrender.service || true
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
