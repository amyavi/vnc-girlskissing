#include <lodepng/lodepng.h>
#include <rfb/rfb.h>
#include <signal.h>
#include <stdlib.h>

#include "vnc.h"

#define SERVER_COUNT 11

typedef struct {
    unsigned int width, height;
    unsigned char* data;
} image;

static volatile sig_atomic_t running = 1;
static void onSIGINT(int unused) {
    (void)unused;
    running = 0;
}

const char* loadImage(image* out, const char* filename) {
    unsigned int err = lodepng_decode32_file(&out->data, &out->width, &out->height, filename);
    return err ? lodepng_error_text(err) : NULL;
}

rfbScreenInfoPtr newImageVNC(int port, const char* name, image img) {
    rfbScreenInfoPtr server = vncNewServer(port, name);
    if (!server) {
        fprintf(stderr, "allocating vnc server on port %d: out of memory\n", port);
        return NULL;
    }

    vncChangeResolution(server, img.width, img.height, 8, 4, 4);
    server->frameBuffer = (char*)img.data;

    rfbInitServer(server);
    rfbRunEventLoop(server, -1, TRUE);
    return server;
}

int main(int argc, char* argv[]) {
    image image = {0};
    const char* err = loadImage(&image, argc >= 2 ? argv[1] : "image.png");
    if (err != NULL) {
        fprintf(stderr, "loading %s: %s\n", argc >= 2 ? argv[1] : "image.png", err);
        return EXIT_FAILURE;
    }

    const char* vncName = argc >= 3 ? argv[2] : "amyavi/vnc-girlskissing";

    rfbLogEnable(0);
    rfbScreenInfoPtr servers[SERVER_COUNT] = {0};
    for (int i = 0; i < SERVER_COUNT; i++) {
        servers[i] = newImageVNC(5900 + i, vncName, image);
        if (servers[i] == NULL) {
            fprintf(stderr, "allocating vnc server %d: out of memory\n", i + 1);
            return EXIT_FAILURE;
        }
    }

    printf("all VNC instances listening!\n");

    signal(SIGINT, onSIGINT);
    while (running) pause();

    for (int i = 0; i < SERVER_COUNT; i++) {
        rfbScreenInfoPtr server = servers[i];
        if (!server) continue;

        vncCloseServer(server);
        free(server);
    }

    free(image.data);

    return 0;
}