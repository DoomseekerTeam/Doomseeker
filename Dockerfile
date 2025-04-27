FROM ubuntu:20.04

# Installs dependencies that support compiling against Qt5.
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get \
	install -y --no-install-recommends \
	build-essential \
	cmake \
	git \
	libbz2-dev \
	zlib1g-dev \
	qt5-default \
	qttools5-dev \
	qttools5-dev-tools \
	qtmultimedia5-dev \
	&& rm -rf /var/lib/apt/lists/*

COPY . /doomseeker
WORKDIR /doomseeker
