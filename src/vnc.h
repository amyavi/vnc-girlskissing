#ifndef __VNC_H
#define __VNC_H

#include <rfb/rfb.h>

rfbScreenInfoPtr vncNewServer(int port, const char *name);
void vncChangeResolution(rfbScreenInfoPtr server, int width, int height, int bitsPerChannel, int channels, int bytesPerPixel);
void vncCloseServer(rfbScreenInfoPtr server);

#endif /* __VNC_H */
