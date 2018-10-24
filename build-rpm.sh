#!/bin/sh
#

# run the script like:
# docker run --volume $prefix:/home/install --volume /home/rpmbuild:/home/rpmbuild
#           -e "os=centos7" centos7_hyrax_builder /home/libdap4/build-rpm.sh

# e: exit immediately on non-zero exit value from a command
# u: treat unset env vars in substitutions as an error
set -eu

# This script will start with /home as the CWD since that's how the 
# centos6/7 hyrax build containers are configured. The PATH will be 
# set to include $prefix/bin and $prefix/deps/bin; $prefix will be
# $HOME/install. $HOME is /root for the build container.

echo "env:"
printenv

echo "pwd = `pwd`"

# Get the pre-built dependencies (all static libraries)
(cd /tmp && curl -s -O https://s3.amazonaws.com/opendap.travis.build/hyrax-dependencies-$os-static.tar.gz)

cd $HOME

# This dumps the dependencies in $HOME/install/deps/{lib,bin,...}
tar -xzvf /tmp/hyrax-dependencies-$os-static.tar.gz

# Get a fresh copy of the sources
git clone https://github.com/opendap/libdap4

cd $HOME/libdap4

# build (autoreconf; configure, make)
autoreconf -fiv

./configure --disable-dependency-tracking --prefix=$prefix 

# This will leave the package in $HOME/rpmbuild/RPMS/x86_64/*.rpm
make -j4 rpm
