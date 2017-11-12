FROM uavia-embedded-base
MAINTAINER Pierre Pele <pierre.pele@uavia.eu>

ARG enableTests="yes"
ARG enableSamples="yes"

# Prepare coreKit build folder.
RUN mkdir -p /uavia/coreKit_build
WORKDIR /uavia/coreKit_build
ADD build.sh build.sh
ADD coreKit coreKit

# Compile & Install coreKit.
RUN ENABLE_PARALLEL_BUILD=0 \
    RELEASE_DIR=/uavia/coreKit_release \
    ./build.sh \
        --enable-shared \
        --disable-static \
        --enable-tests=${enableTests} \
        --enable-samples=${enableSamples} \
        --enable-stacktrace=${enableStackTrace}

# Delete build directory.
WORKDIR /
RUN rm -rf /uavia/coreKit_build

# Update PKG_CONFIG_PATH variable.
ENV PKG_CONFIG_PATH="/uavia/coreKit_release/lib/pkgconfig:${PKG_CONFIG_PATH}"
