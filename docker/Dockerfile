FROM l3iggs/archlinux
MAINTAINER Peter Huene <peterhuene@gmail.com>
RUN pacman -Sy --noconfirm gcc git make cmake boost yaml-cpp python-pip && \
    yes | pacman -Scc && \
    pip install --user codecov && \
    git clone --recursive https://github.com/puppetlabs/leatherman.git /tmp/leatherman && \
    pushd /tmp/leatherman && \
    cmake . && \
    make && \
    make install && \
    popd && \
    rm -rf /tmp/leatherman && \
    git clone --recursive https://github.com/puppetlabs/facter.git /tmp/facter && \
    pushd /tmp/facter && \
    cmake . && \
    make && \
    make install && \
    popd && \
    rm -rf /tmp/facter
