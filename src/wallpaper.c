#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdlib.h>

#include "common.h"

//#define DEBUGPRN
#include "dbg.h"

struct config_t conf;

/* global vars */
static Display *dpy;
static Window root;

#if 0

/* Get lower win from window stack and check if it is a dekstop win */
static Window
x11_get_desktop_win()
{
    Atom type;
    gulong nitems, bytes_after;
    gint format;
    guchar *data;
    int rc;
    Window win = None;
    char *npls = "_NET_CLIENT_LIST_STACKING";
    char *npwt = "_NET_WM_WINDOW_TYPE";
    char *npwtd = "_NET_WM_WINDOW_TYPE_DESKTOP";
    Atom als, awt, awtd, a;
    
    als = XInternAtom(dpy, npls, False);
    DBG("atom for %s, %lu\n", npls, als);
    rc = XGetWindowProperty(dpy, root, als,
        0L, 1L, False, XA_WINDOW,
        &type, &format, &nitems, &bytes_after,
        &data);
    DBG("get prop %s: %d\n", npls, rc);
    if (rc != Success)
        return None;
    
    if (type == XA_WINDOW && format == 32 && nitems > 0) 
        win = * (Window *) data;
    DBG("win %lx\n", win);
    XFree(data);

    if (win == None)
        return None;
    
    awt = XInternAtom(dpy, npwt, False);
    DBG("atom for %s, %lu\n", npwt, awt);
    awtd = XInternAtom(dpy, npwtd, False);
    DBG("atom for %s, %lu\n", npwtd, awtd);
    rc = XGetWindowProperty(dpy, win, awt,
        0L, 1L, False, XA_ATOM,
        &type, &format, &nitems, &bytes_after,
        &data);
    DBG("get prop %s: %d\n", npwt, rc);
    if (rc != Success)
        return None;

    if (type == XA_ATOM && format == 32 && nitems == 1) 
        a = * (Atom *) data;
    DBG("atom of win type of %lx is %lu\n", win, a);
    XFree(data);
        
    return a == awtd ? win : None;
}
#endif


static int
x11_get_win_depth(Window win)
{
    XWindowAttributes xwa;
    int rc;
    
    rc = XGetWindowAttributes(dpy, win, &xwa);
    DBG("get depth for %lx, rc %d , depth %d\n", win, rc, xwa.depth);
    return rc ? xwa.depth : 0;
}
    
    
static Pixmap
x11_create_pmap(Window win, GdkPixbuf *pix, int depth)
{
    Pixmap pmap;
    GdkPixmap *gpmap;
    int w, h;
    GdkGC *gc;
    
    w = gdk_pixbuf_get_width(pix);
    h = gdk_pixbuf_get_height(pix);
    pmap = XCreatePixmap(dpy, win, w, h, depth);
    DBG("new pmap %lx, %d x %d x %d\n", pmap, w, h, depth);
    if (pmap == None) {
        ERR("Can't create x11 pixmap of size %d x %d, depth %d\n",
            w, h, depth);
        return None;
    }
    gpmap = gdk_pixmap_foreign_new(pmap);
    if (gpmap == NULL) {
        XFreePixmap(dpy, pmap);
        ERR("CAn't get gdk handler for x11 pixmap %lx\n", pmap);
        return None;
    }
    gc = gdk_gc_new_with_values(gpmap, NULL, 0);
    gdk_draw_pixbuf(gpmap, gc, pix, 0, 0, 0, 0, w, h,
        GDK_RGB_DITHER_NONE, 0, 0);
    gdk_gc_unref(gc);
    return pmap;
}

static Pixmap
x11_set_bg_pix_real(GdkPixbuf *pix)
{
    Pixmap pmap;
    int depth;
    Window win;

    win = root;
    depth = x11_get_win_depth(win);
    if (!depth) 
        return None;
    pmap = x11_create_pmap(win, pix, depth);
    if (pmap == None)
        return None;
    DBG("set bg for win %lx, pmap %lx\n", win, pmap);
    XSetWindowBackgroundPixmap(dpy, win, pmap);
    XClearWindow(dpy, win);
    XFlush(dpy);

    return pmap;
#if 0
    win = x11_get_desktop_win();
    if (win == None)
        return;
    depth2 = x11_get_win_depth(win);
    if (!depth)
        return;
    if (depth != depth2)
        pmap = x11_create_pmap(win, pix, depth2);
    DBG("set bg for win %lx, pmap %lx\n", win, pmap);
    XSetWindowBackgroundPixmap(dpy, win, pmap);
    XClearWindow(dpy, win);
    XFlush(dpy);
#endif
}

static void
x11_handle_error(Display * d, XErrorEvent * ev)
{
    char buf[256];

    XGetErrorText(GDK_DISPLAY(), ev->error_code, buf, 256);
    DBG("fbpanel : X error: %s\n", buf);
}

static void
x11_set_bg_pix(GdkPixbuf *pix)
{
    Atom type;
    gulong nitems, bytes_after;
    gint format;
    guchar *data;
    int result;
    Atom ae, ax;
    Pixmap pmap;

    XSetCloseDownMode(dpy, RetainPermanent);
    XGrabServer(dpy);
    XSetErrorHandler((XErrorHandler) x11_handle_error);

    DBG("grab ok\n");
    ae = XInternAtom(dpy, "ESETROOT_PMAP_ID", False);
    ax = XInternAtom(dpy, "_XROOTPMAP_ID", False);
    result = XGetWindowProperty(dpy, root, ae, 
        0L, 1L, False, XA_PIXMAP,
        &type, &format, &nitems, &bytes_after, &data);

    if (result == Success && type == XA_PIXMAP &&
        format == 32 && nitems == 1) {
        DBG("removing old pixmap %lx\n", *(Pixmap*)data);
        XKillClient(dpy, *(Pixmap*)data);
    }

    if (result == Success) {
        XFree(data);
    }
    pmap = x11_set_bg_pix_real(pix);
    if (pmap != None) {
        XChangeProperty(dpy, root, ae, XA_PIXMAP, 32, PropModeReplace,
            (guchar *) &pmap, 1);
        XChangeProperty(dpy, root, ax, XA_PIXMAP, 32, PropModeReplace,
            (guchar *) &pmap, 1);
    } else {
        XDeleteProperty(dpy, root, ae);
        XDeleteProperty(dpy, root, ax);
    }
    XUngrabServer(dpy);
    XFlush(dpy);
}


static void
pix_composite(struct config_t *conf, GdkPixbuf *bg, GdkPixbuf *img)
{
    int wbg, hbg;
    int wpx, hpx;
    int dest_width;
    int dest_height;
    int dest_x;
    int dest_y;
    double offset_x;
    double offset_y;
    double scale_r;


    wbg = gdk_pixbuf_get_width(bg);
    hbg = gdk_pixbuf_get_height(bg);
    wpx = gdk_pixbuf_get_width(img);
    hpx = gdk_pixbuf_get_height(img);
    if (conf->scale == SCALE_COVER)
        scale_r = MAX((double)wbg / (double)wpx, (double)hbg / (double)hpx);
    else if (conf->scale == SCALE_FIT)
        scale_r = MIN((double)wbg / (double)wpx, (double)hbg / (double)hpx);
    else
        scale_r = 1.0;
    DBG("scale %.2f\n", scale_r);
    dest_width  = wpx * scale_r;
    dest_height = hpx * scale_r;
    dest_x = 0;
    dest_y = 0;
    offset_x = 0.0;
    offset_y = 0.0;

    if (conf->gravity == GRV_CENTER) {
        offset_x = (wbg - dest_width)  / 2;
        offset_y = (hbg - dest_height) / 2;

        if (offset_x > 0)
            dest_x = offset_x;
        if (offset_y > 0)
            dest_y = offset_y;
    }
    DBG("offset %+d%+d\n", (int)offset_x, (int)offset_y);
    if (dest_width > wbg)
        dest_width = wbg;
    if (dest_height > hbg)
        dest_height = hbg;
    DBG("dest geom=%dx%d pos=%dx%d\n", dest_width, dest_height,
        dest_x, dest_y);
    gdk_pixbuf_composite(img, bg,
        /* dest area */
        dest_x, dest_y, dest_width, dest_height,
        /* src transformation */
        offset_x, offset_y, scale_r, scale_r,
        GDK_INTERP_HYPER, 255);
}

static GdkPixbuf *
pix_create(struct config_t *conf)
{
    GError *error = NULL;
    guint32 p;
    GdkColor c;
    GdkPixbuf *bg, *img;
    GdkScreen *s = gdk_screen_get_default();
    int sw, sh;
    
    if (!s)
        return NULL;
    sw = gdk_screen_get_width(s);
    sh = gdk_screen_get_height(s);
    bg = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, sw, sh);
    if (!bg) {
        ERR("Can't create pixbuf %d x %d\n", sw, sh);
        return NULL;
    }
    c = conf->rgb;
    p = (c.red >> 8) << 24 | (c.green >> 8) << 16 | (c.blue >> 8) << 8;
    gdk_pixbuf_fill(bg, p);
    DBG("bgimg filled with #%x%x%x color is ready\n", c.red, c.green, c.blue);
    if (!conf->image)
        return bg;
    
    img = gdk_pixbuf_new_from_file(conf->image, &error);
    if (!img) {
        ERR("Can't load %s: %s\n", conf->image, error->message);
        return NULL;
    }
    pix_composite(conf, bg, img);
    DBG("bgimg with composited image is ready\n");
    g_object_unref(img);

    return bg;
}


int
main(int argc, char *argv[])
{
    GdkPixbuf *pix;
  
    gtk_init(&argc, &argv);
    parse_args(&conf, argc, argv);
    pix = pix_create(&conf);
    if (!pix)
        exit(1);
    dpy = GDK_DISPLAY();
    root = GDK_ROOT_WINDOW();
    //gdk_pixbuf_save(pix, "test.png", "png", NULL, NULL);
    x11_set_bg_pix(pix);
    return 0;
}
