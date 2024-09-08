#include <lodepng/lodepng.h>
#include <rfb/rfb.h>
#include <signal.h>
#include <stdlib.h>

#include "vnc.h"

typedef struct {
    unsigned int width, height;
    unsigned char* data;
} image;

static volatile sig_atomic_t running = 1;
static void onShutdown(int unused) {
    (void)unused;
    running = 0;
}

const char* loadImage(image* out, const char* filename) {
    unsigned int err = lodepng_decode32_file(&out->data, &out->width, &out->height, filename);
    return err ? lodepng_error_text(err) : NULL;
}

rfbScreenInfoPtr newImageVNC(int port, const char* name, image img) {
    rfbScreenInfoPtr server = vncNewServer(port, name);
    if (!server) return NULL;

    vncChangeResolution(server, img.width, img.height, 8, 4, 4);
    server->frameBuffer = (char*)img.data;

    rfbInitServer(server);
    rfbRunEventLoop(server, -1, TRUE);
    return server;
}

int main(int argc, char* argv[]) {
    image image = {0};

    const char* imagePath = argc >= 2 ? argv[1] : "image.png";
    const char* err = loadImage(&image, imagePath);
    if (err != NULL) {
        fprintf(stderr, "loading %s: %s\n", imagePath, err);
        return EXIT_FAILURE;
    }

    const char* vncName = argc >= 3 ? argv[2] : "amyavi/vnc-girlskissing";

    rfbLogEnable(0);
    rfbScreenInfoPtr server = newImageVNC(5900, vncName, image);
    if (!server) {
        fprintf(stderr, "allocating vnc server: out of memory\n");
        return EXIT_FAILURE;
    }

    printf("VNC server listening!\n");

    signal(SIGINT, onShutdown);
    signal(SIGTERM, onShutdown);
    while (running) pause();

    vncCloseServer(server);
    free(server);
    free(image.data);

    return 0;
}