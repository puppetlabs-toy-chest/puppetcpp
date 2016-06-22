FROM gliderlabs/alpine:edge
MAINTAINER Peter Huene <peterhuene@gmail.com>
RUN apk add --no-cache make cmake gcc g++ boost-dev boost-program_options curl-dev libedit-dev git py-pip icu-dev && \
    pip install --user codecov && \
    git clone https://github.com/k-takata/Onigmo.git /tmp/onigmo && \
    cd /tmp/onigmo && \
    ./configure --enable-shared --enable-multithread && \
    make -j 4 && \
    make install && \
    cd && \
    rm -rf /tmp/onigmo && \
    git clone https://github.com/jbeder/yaml-cpp.git /tmp/yaml-cpp && \
    cd /tmp/yaml-cpp && \
    cmake . -DBUILD_SHARED_LIBS=ON && \
    make -j 4 && \
    make install && \
    cd && \
    rm -rf /tmp/yaml-cpp && \
    git clone --recursive https://github.com/puppetlabs/leatherman.git /tmp/leatherman && \
    cd /tmp/leatherman && \
    cmake . && \
    make -j 4 && \
    make install && \
    cd && \
    rm -rf /tmp/leatherman && \
    git clone --recursive https://github.com/puppetlabs/facter.git /tmp/facter && \
    cd /tmp/facter && \
    cmake . && \
    make -j 4 && \
    make install && \
    cd && \
    rm -rf /tmp/facter && \
    mkdir -p /etc/puppetlabs/code/environments/production
