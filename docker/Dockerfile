FROM l3iggs/archlinux
MAINTAINER Peter Huene <peterhuene@gmail.com>
RUN pacman -Sy --noconfirm gcc git make cmake boost yaml-cpp && \
    yes | pacman -Scc && \
    git clone -b fix-api --recursive https://github.com/peterhuene/facter.git /tmp/facter && \
    pushd /tmp/facter && \
    cmake . && \
    make && \
    make install && \
    popd && \
    rm -rf /tmp/facter

