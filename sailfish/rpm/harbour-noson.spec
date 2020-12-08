Name:           harbour-noson
Version:        4.4.5
Release:        15
Summary:        SONOS device controller
License:        GPL-3.0-or-later
Group:          Productivity/Multimedia/Sound/Players
URL:            http://janbar.github.io/noson-app/index.html
Source0:        https://github.com/janbar/noson-app/archive/%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  extra-cmake-modules
BuildRequires:  git
BuildRequires:  gcc-c++
BuildRequires:  openssl-devel
BuildRequires:  zlib-devel
BuildRequires:  pulseaudio-devel
BuildRequires:  pkgconfig
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Svg)
BuildRequires:  pkgconfig(Qt5Xml)

%description
A controller for SONOS devices. It allows for browsing the music
library, and playing tracks or radio on any zones. Zone groups,
queues and playlists can be managed, and playback be controlled.

%prep
%setup -q -n %{name}-%{version}

%build
mkdir -p build-%{_target_cpu}
pushd build-%{_target_cpu}
%cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SAILFISHOS=ON ../..
make -j2
popd

%install
pushd build-%{_target_cpu}
%make_install -C build
popd

%files
%{_bindir}/harbour-noson
%{_datadir}/applications/harbour-noson.desktop
%{_datadir}/icons/hicolor/

%changelog
