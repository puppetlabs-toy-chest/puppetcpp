FROM gliderlabs/alpine:edge
MAINTAINER Peter Huene <peterhuene@gmail.com>
RUN apk add --no-cache make cmake gcc g++ boost-dev curl-dev libedit-dev git icu-dev openssl && \
    wget https://raw.githubusercontent.com/peterhuene/spirit/fcf705ef9ea316dded54e14fb6ba9d01a27ec0e4/include/boost/spirit/home/x3/operator/detail/sequence.hpp -O /usr/include/boost/spirit/home/x3/operator/detail/sequence.hpp && \
    git clone https://github.com/k-takata/Onigmo.git /tmp/onigmo && \
    cd /tmp/onigmo && \
    ./configure --enable-shared && \
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
    git clone --recursive https://github.com/puppetlabs/cpp-hocon.git /tmp/cpp-hocon && \
    cd /tmp/cpp-hocon && \
    cmake . && \
    make -j 4 && \
    make install && \
    cd && \
    rm -rf /tmp/cpp-hocon && \
    git clone --recursive https://github.com/puppetlabs/facter.git /tmp/facter && \
    cd /tmp/facter && \
    cmake . && \
    make -j 4 && \
    make install && \
    cd && \
    rm -rf /tmp/facter && \
    git clone --recursive https://github.com/peterhuene/puppetcpp.git /tmp/puppetcpp && \
    cd /tmp/puppetcpp && \
    cmake . && \
    make -j 4 && \
    make install && \
    cd && \
    rm -rf /tmp/puppetcpp && \
    mkdir -p /etc/puppetlabs/code/environments/production && \
    apk --no-cache del make cmake gcc g++ boost-dev curl-dev libedit-dev git icu-dev openssl && \
    apk --no-cache add libstdc++ boost boost-program_options libcurl libedit icu-libs
ENTRYPOINT ["/usr/local/bin/puppetcpp"]
