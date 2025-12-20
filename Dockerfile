# The Dockerfile prepares a build environment for Doomseeker.

# Its purpose is to mimic the oldest supported Linux system, to the extent of
# downgrading the default GCC 11 for this distro to version 10. The image
# installs the necessary dependencies, but doesn't copy the source code
# itself. You must mount the source code at the mountpoint.
#
# Canonical name:     doomseeker/doomseeker-build
# Canonical tag:      ubuntu-22.04
# Source mountpoint:  /src/doomseeker
#
# Run the image like this:
#
#   # Assuming that $(pwd) is the path to this repo's root.
#   docker run -it -v "$(pwd):/src/doomseeker" doomseeker/doomseeker-build
#
# Configure and build the project with Qt5 with:
#
#   cmake ../doomseeker -DQt_PACKAGE=Qt5
#   cmake --build . -j$(nproc)
#
# With Qt6:
#
#   cmake ../doomseeker -DQt_PACKAGE=Qt6
#   cmake --build . -j$(nproc)
FROM ubuntu:22.04

# Install dependencies for compiling against either Qt5 or Qt6.
# (Don't install build-essential to avoid getting the default gcc.)
RUN apt-get update && \
	DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC apt-get -y --no-install-recommends install \
	ca-certificates \
	cmake \
	g++-10 \
	gcc-10 \
	git \
	libbz2-dev \
	make \
	qt6-base-dev \
	qt6-l10n-tools \
	qt6-multimedia-dev \
	qt6-tools-dev \
	qt6-tools-dev-tools \
	qtmultimedia5-dev \
	qttools5-dev \
	qttools5-dev-tools \
	tzdata \
	zlib1g-dev \
	&& rm -rf /var/lib/apt/lists/*

# Make GCC 10 the default.
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 10 \
	&& update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-10 10

# Prepare the build directories.
RUN mkdir -p /src/doomseeker /src/doomseeker-build
WORKDIR /src/doomseeker-build
