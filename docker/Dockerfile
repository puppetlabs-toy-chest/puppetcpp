FROM l3iggs/archlinux
MAINTAINER Peter Huene <peterhuene@gmail.com>
RUN pacman -Sy --noconfirm gcc git make cmake boost yaml-cpp && \
    yes | pacman -Scc && \
    git clone --recursive https://github.com/puppetlabs/facter.git /tmp/facter && \
    pushd /tmp/facter && \
    cmake . && \
    make && \
    make install && \
    popd && \
    rm -rf /tmp/facter

