FROM ubuntu:16.04

# Installs dependencies that support compiling against either Qt4 or Qt5.
RUN apt-get update && apt-get -y install \
	build-essential \
	cmake \
	mercurial \
	libbz2-dev \
	zlib1g-dev \
	qt5-default \
	qttools5-dev \
	qttools5-dev-tools \
	qtmultimedia5-dev

COPY . /doomseeker
WORKDIR /doomseeker
