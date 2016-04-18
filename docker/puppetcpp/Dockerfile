FROM gliderlabs/alpine:edge
MAINTAINER Peter Huene <peterhuene@gmail.com>
RUN apk add --no-cache make cmake gcc g++ boost-dev curl-dev libedit-dev git && \
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
    git clone --recursive https://github.com/peterhuene/puppetcpp.git -b docker /tmp/puppetcpp && \
    cd /tmp/puppetcpp && \
    cmake . && \
    make -j 4 && \
    cp bin/puppetcpp /usr/bin/ && \
    cp lib/libpuppet.so.0.1.0 /usr/lib/ && \
    cd && \
    rm -rf /tmp/puppetcpp && \
    mkdir -p /etc/puppetlabs/code/environments/production && \
    apk --no-cache del make cmake gcc g++ boost-dev curl-dev libedit-dev git && \
    apk --no-cache add libstdc++ boost boost-program_options libcurl libedit
ENTRYPOINT ["/usr/bin/puppetcpp"]
