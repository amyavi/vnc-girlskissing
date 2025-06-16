FROM alpine AS builder
RUN apk add --no-cache \
    clang make cmake git \
    \
    libjpeg-turbo-dev libjpeg-turbo-static \
    libpng-dev libpng-static \
    zlib-dev zlib-static

ARG BUILD_TYPE=Release
ARG BUID=1000
ARG BGID=1000

# libvncserver
USER "${BUID}:${BGID}"
WORKDIR /src/libvncserver 
RUN git clone https://github.com/LibVNC/libvncserver.git /src/libvncserver && \
    cmake -B build -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_INSTALL_LIBDIR=lib \
        -DBUILD_SHARED_LIBS=OFF \
        \
        -DWITH_ZLIB=ON -DZLIB_LIBRARY_RELEASE=/usr/lib/libz.a \
        -DWITH_JPEG=ON -DJPEG_LIBRARY_RELEASE=/usr/lib/libjpeg.a \
        -DWITH_PNG=ON -DPNG_LIBRARY_RELEASE=/usr/lib/libpng.a \
        -DWITH_THREADS=ON \
        \
        -DWITH_LZO=OFF -DWITH_SDL=OFF -DWITH_XCB=OFF \
        -DWITH_GNUTLS=OFF -DWITH_OPENSSL=OFF -DWITH_GCRYPT=OFF \
        -DWITH_WEBSOCKETS=OFF -DWITH_SASL=OFF -DWITH_FFMPEG=OFF \
        -DWITH_SYSTEMD=OFF -DWITH_EXAMPLES=OFF -DWITH_TESTS=OFF -DWITH_QT=OFF && \
    cmake --build build -j$(nproc)

# don't @ me
USER 0:0
RUN cmake --install build

# app
USER "${BUID}:${BGID}"
WORKDIR /src/app
COPY . .

RUN cmake -B build -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
        -DBUILD_STATIC=ON && \
    cmake --build build --verbose -j$(nproc)

FROM scratch
ARG UID=1000
ARG GID=1000
USER "${UID}:${GID}"

COPY --chown=0:0 --chmod=555 --from=builder /src/app/build/vnc-girlskissing /vnc-girlskissing

EXPOSE 5900
ENTRYPOINT ["/vnc-girlskissing", "/image.png"]
