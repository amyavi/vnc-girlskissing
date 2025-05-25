#include "vnc.h"

#include <ctype.h>
#include <rfb/rfb.h>
#include <rfb/rfbproto.h>

#define MAX_CLIPBOARD_SIZE 4096

// Event handling
static rfbBool vncCheckPassword(rfbClientPtr client, const char* response, int len) { return TRUE; }  // looks good to me buddy

static void vncOnClientGone(rfbClientPtr client) {
    printf("Client %s disconnected\n", client->host);
    fflush(stdout);
}

static enum rfbNewClientAction vncOnNewClient(rfbClientPtr client) {
    client->clientGoneHook = vncOnClientGone;
    printf("Client %s connected\n", client->host);
    fflush(stdout);
    return RFB_CLIENT_ACCEPT;
}

// Screen
static inline int vncSetDesktopSize(int w, int h, int num, rfbExtDesktopScreen* screen, rfbClientPtr client) {
    return rfbExtDesktopSize_ResizeProhibited;
}

static inline int vncGetScreenNum(rfbClientPtr client) { return 1; }

static inline rfbBool vncGetScreen(int seq, rfbExtDesktopScreen* screen, rfbClientPtr client) {
    if (seq != 0) return FALSE;

    screen->id = 1;
    screen->width = client->scaledScreen->width;
    screen->height = client->scaledScreen->height;
    screen->x = screen->y = 0;
    screen->flags = 0;

    return TRUE;
}

// Keyboard
static inline void vncOnKeyPress(rfbBool down, rfbKeySym keySym, rfbClientPtr client) {}
static inline void vncOnSetClipboard(char* str, int len, rfbClientPtr client) {
    if (len > MAX_CLIPBOARD_SIZE) return;
    char buf[MAX_CLIPBOARD_SIZE + 1] = {0};

    int print_len = 0;
    for (int i = 0; i < len; i++) {
        const char c = str[i];
        if (c == 0) continue;

        buf[print_len] = isprint(c) ? c : '?';
        print_len++;
    }

    if (print_len == 0) return;
    buf[print_len] = '\n';

    printf("Client %s set clipboard: ", client->host);
    fwrite(buf, print_len + 1, 1, stdout);
    fflush(stdout);
}

// Mouse
static inline void vncOnMouseMove(int buttonMask, int x, int y, rfbClientPtr client) {}
static inline rfbCursorPtr vncGetCursor(rfbClientPtr client) { return NULL; }

rfbScreenInfoPtr vncNewServer(int port, const char* name) {
    rfbScreenInfoPtr server = calloc(1, sizeof(rfbScreenInfo));
    if (!server) return NULL;

    server->alwaysShared = TRUE;
    server->desktopName = name;
    server->listenInterface = htonl(INADDR_ANY);
    server->port = port;
    server->ipv6port = port;
    server->protocolMajorVersion = rfbProtocolMajorVersion;
    server->protocolMinorVersion = rfbProtocolMinorVersion;

    server->deferUpdateTime = 50;
    server->maxRectsPerUpdate = 50;
    server->fdQuota = 0.9;

    server->socketState = RFB_SOCKET_INIT;
    server->listenSock = RFB_INVALID_SOCKET;
    server->listen6Sock = RFB_INVALID_SOCKET;
    server->inetdSock = RFB_INVALID_SOCKET;
    server->udpSock = RFB_INVALID_SOCKET;
    server->httpListenSock = RFB_INVALID_SOCKET;
    server->httpListen6Sock = RFB_INVALID_SOCKET;
    server->httpSock = RFB_INVALID_SOCKET;

#ifdef LIBVNCSERVER_HAVE_LIBPTHREAD
    server->pipe_notify_listener_thread[0] = -1;
    server->pipe_notify_listener_thread[1] = -1;
#endif

    rfbClientListInit(server);

    // Yes, we do need this mutex, even if there is no cursor
    INIT_MUTEX(server->cursorMutex);

    server->passwordCheck = vncCheckPassword;
    server->kbdAddEvent = vncOnKeyPress;
    server->kbdReleaseAllKeys = rfbDoNothingWithClient;
    server->ptrAddEvent = vncOnMouseMove;
    server->setXCutText = vncOnSetClipboard;
#ifdef LIBVNCSERVER_HAVE_LIBZ
    server->setXCutTextUTF8 = vncOnSetClipboard;
#endif

    server->getCursorPtr = vncGetCursor;
    server->setTranslateFunction = rfbSetTranslateFunction;
    server->newClientHook = vncOnNewClient;

    server->setDesktopSizeHook = vncSetDesktopSize;
    server->numberOfExtDesktopScreensHook = vncGetScreenNum;
    server->getExtDesktopScreenHook = vncGetScreen;

    return server;
}

void vncChangeResolution(rfbScreenInfoPtr server, int width, int height, int bitsPerChannel, int channels, int bytesPerPixel) {
    rfbNewFramebuffer(server, server->frameBuffer, width, height, bitsPerChannel, channels, bytesPerPixel);
}

void vncCloseServer(rfbScreenInfoPtr server) {
    rfbShutdownServer(server, TRUE);
}
