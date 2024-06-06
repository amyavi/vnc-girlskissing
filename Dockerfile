FROM alpine AS builder
RUN apk add --no-cache \
    clang make cmake git

# TODO: build as another user
WORKDIR /src

# libvncserver
RUN apk add --no-cache \
    libjpeg-turbo-dev libjpeg-turbo-static \
    libpng-dev libpng-static \
    zlib-dev zlib-static
 
RUN git clone https://github.com/LibVNC/libvncserver.git /src/libvncserver && \
    cd /src/libvncserver && \
    cmake -B build -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_INSTALL_LIBDIR=lib \
        -DBUILD_SHARED_LIBS=OFF \
        \
        -DWITH_ZLIB=ON -DZLIB_LIBRARY_RELEASE=/lib/libz.a \
        -DWITH_JPEG=ON -DJPEG_LIBRARY_RELEASE=/usr/lib/libjpeg.a \
        -DWITH_PNG=ON -DPNG_LIBRARY_RELEASE=/usr/lib/libpng.a \
        -DWITH_THREADS=ON \
        \
        -DWITH_LZO=OFF -DWITH_SDL=OFF -DWITH_XCB=OFF \
        -DWITH_GNUTLS=OFF -DWITH_OPENSSL=OFF -DWITH_GCRYPT=OFF \
        -DWITH_WEBSOCKETS=OFF -DWITH_SASL=OFF -DWITH_FFMPEG=OFF \
        -DWITH_SYSTEMD=OFF -DWITH_EXAMPLES=OFF -DWITH_TESTS=OFF -DWITH_QT=OFF && \
    cmake --build build -j$(nproc) && \
    cmake --install build && \
    cd ..

# app
WORKDIR /src/app
COPY . .

RUN cmake -B build -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_STATIC=ON && \
    cmake --build build --verbose -j$(nproc)

FROM scratch
COPY --from=builder /src/app/build/vnc-girlskissing /vnc-girlskissing

USER 1000:1000

ENTRYPOINT ["/vnc-girlskissing", "/image.png"]
