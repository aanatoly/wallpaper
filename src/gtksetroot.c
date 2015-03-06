/* xsetbgpix: Sets X11 root window background image. 
 * Author: Anatoly Asviyan <aanatoly@gmail.com>
 * Licence: GPL v2
 */

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdlib.h>

#define PRN(args...) \
    do { if (verbose) printf(args); } while (0)

typedef enum { SCALE_NONE, SCALE_FIT, SCALE_COVER } scale_t;
typedef enum { GRV_CENTER } gravity_t;

typedef struct {
    char *str;
    int num;
} pair_t;

/* global parameters */
static char *colorname;
static GdkColor colorrgb;
static char *image;    /* image file name */
static scale_t scale = SCALE_COVER;  /* scale type */
static gravity_t gravity;
static int verbose;
static pair_t scale_pair[] = {
    { "none", SCALE_NONE },
    { "fit", SCALE_FIT },
    { "cover", SCALE_COVER },
    { NULL, 0 }
};

/* global vars */
static GdkPixbuf *pixbuf;
static GdkPixbuf *bg;
static Display *dpy;
static Window root;

static gboolean parse_pair(const gchar *option_name, const gchar *value,
        gpointer data, GError **error);

static GOptionEntry entries[] =
{
    { "verbose", 0, 0, G_OPTION_ARG_NONE, &verbose,
        "Verbose output", NULL },
    { "color", 0, 0, G_OPTION_ARG_STRING, &colorname,
        "Base color (in 0xRRGGBB format)", "#ffd2ac" },
    { "image", 0, G_OPTION_FLAG_FILENAME, G_OPTION_ARG_FILENAME, &image,
        "Background image file name", "" },
    { "scale", 0, 0, G_OPTION_ARG_CALLBACK, parse_pair,
        "Scale type: fit, cover or none", "type" },

    { NULL }
};

static char *desription = "For example:\n  gtksetroot --color=0x456789 "
  "--scale=fit  --image=~/pics/img002.jpg\n";

static int
str2num(const gchar *str, pair_t *p, int failed)
{
    while (p && p->str) {
        if (!g_strcmp0(p->str, str))
            return p->num;
        p++;
    }
    return failed;
}

static gboolean
parse_pair(const gchar *option_name, const gchar *value,
        gpointer data, GError **error)
{
    if (!g_strcmp0(option_name, "--scale")) {
        scale = str2num(value, scale_pair, -1);
        if (scale != -1)
            return TRUE;
    }
    g_error("%s - illegal value for %s", value, option_name);
    return FALSE;
}

static int
make_bg(void)
{
    GdkScreen *s;
    int w, h;

    s = gdk_screen_get_default();
    if (!s)
        return 0;

    w = gdk_screen_get_width(s);
    h = gdk_screen_get_height(s);
    PRN("screen %s\ndim %dx%d\n",
            gdk_display_get_name(gdk_display_get_default()),
            w, h);
    bg = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, w, h);
    if (!bg)
        return 0;

    if (!colorname)
        colorname = "#ffd2ac";
    if (gdk_color_parse(colorname, &colorrgb))
    {
        GdkColor g = colorrgb;

        /* create RGBA from RGB */
#define ADD(src, num) ((src & 0xFF00) << (num * 8))
        g.pixel = ADD(g.red, 2) | ADD(g.green, 1) | ADD(g.blue, 0) | 0xFF;
        PRN("color: name %s rgb %06x / %02x:%02x:%02x\n",
            colorname, g.pixel, g.red, g.green, g.blue);
        gdk_pixbuf_fill(bg, g.pixel);
    }
    return 1;
}

static int
read_image(void)
{
    GError *error = NULL;

    PRN("image %s\n", image);
    if (!image)
        return 1; /* no image is ok, bg will be painted with solid color */
    if (image[0] == '~') {
        gchar *tmp = g_build_filename(g_get_home_dir(), image + 1, NULL);
        g_free(image);
        image = tmp;
    }
    if (!(pixbuf = gdk_pixbuf_new_from_file(image, &error))) {
        fprintf(stderr, "%s\n", error->message);
        return 0;
    }
    PRN("dim %dx%d\n", gdk_pixbuf_get_width(pixbuf),
        gdk_pixbuf_get_height(pixbuf));
    return 1;
}

static int
apply_image(void)
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

    if (!pixbuf)
        return 1;
    wbg = gdk_pixbuf_get_width(bg);
    hbg = gdk_pixbuf_get_height(bg);
    wpx = gdk_pixbuf_get_width(pixbuf);
    hpx = gdk_pixbuf_get_height(pixbuf);
    if (scale == SCALE_COVER)
        scale_r = MAX((double)wbg / (double)wpx, (double)hbg / (double)hpx);
    else if (scale == SCALE_FIT)
        scale_r = MIN((double)wbg / (double)wpx, (double)hbg / (double)hpx);
    else
        scale_r = 1.0;
    PRN("scale %.2f\n", scale_r);
    dest_width  = wpx * scale_r;
    dest_height = hpx * scale_r;
    dest_x = 0;
    dest_y = 0;
    offset_x = 0.0;
    offset_y = 0.0;

    if (gravity == GRV_CENTER) {
        offset_x = (wbg - dest_width)  / 2;
        offset_y = (hbg - dest_height) / 2;

        if (offset_x > 0)
            dest_x = offset_x;
        if (offset_y > 0)
            dest_y = offset_y;
    }
    PRN("offset %+d%+d\n", (int)offset_x, (int)offset_y);
    if (dest_width > wbg)
        dest_width = wbg;
    if (dest_height > hbg)
        dest_height = hbg;
    PRN("dest geom=%dx%d pos=%dx%d\n", dest_width, dest_height,
            dest_x, dest_y);
    gdk_pixbuf_composite(pixbuf, bg,
            /* dest area */
            dest_x, dest_y, dest_width, dest_height,
            /* src transformation */
            offset_x, offset_y, scale_r, scale_r,
            GDK_INTERP_HYPER, 255);
    return 1;
}

/* This source was taken from chbg project, which in turn, I guess, took it
 * from gnome control center */
static Pixmap
gnomecc_make_root_pixmap(int width, int height)
{
    Pixmap result;
    Display *display;

    gdk_flush();

    display = XOpenDisplay(gdk_display_get_name(gdk_display_get_default()));
    XSetCloseDownMode(display, RetainPermanent);

    result = XCreatePixmap(display,
            DefaultRootWindow(display),
            width, height,
            DefaultDepthOfScreen(DefaultScreenOfDisplay(GDK_DISPLAY())));
    PRN("new root pixmap %lx\n", result);
    XCloseDisplay(display);

    if (!result)
        fprintf(stderr, "can't make root pixmap\n");
    return result;
}

static void
gnomecc_set_root_pixmap(Pixmap pixmap)
{
    Atom type;
    gulong nitems, bytes_after;
    gint format;
    guchar *data_esetroot;
    int result;
    Atom ae, ax;

    XGrabServer(dpy);
    ae = XInternAtom(dpy, "ESETROOT_PMAP_ID", False);
    ax = XInternAtom(dpy, "_XROOTPMAP_ID", False);
    result = XGetWindowProperty(dpy, root,
            ae,
            0L, 1L, False, XA_PIXMAP,
            &type, &format, &nitems, &bytes_after,
            &data_esetroot);

    if (result == Success && type == XA_PIXMAP &&
            format == 32 && nitems == 1) {
        XKillClient(dpy, *(Pixmap*)data_esetroot);
        PRN("removing old pixmap %p\n", (Pixmap*)data_esetroot);
    }

    if (data_esetroot != NULL) {
        XFree(data_esetroot);
    }

    if (pixmap != None) {
        XChangeProperty(dpy, root,
                ae,
                XA_PIXMAP, 32, PropModeReplace,
                (guchar *) &pixmap, 1);
        XChangeProperty(dpy, root,
                ax,
                XA_PIXMAP, 32, PropModeReplace,
                (guchar *) &pixmap, 1);

        XSetWindowBackgroundPixmap(dpy, root, pixmap);
    } else {
        XDeleteProperty(dpy, root, ae);
        XDeleteProperty(dpy, root, ax);

    }

    XClearWindow(dpy, root);
    XUngrabServer(dpy);

    XFlush(dpy);
}

static int
set_bg(void)
{
    Pixmap p;
    GdkPixmap *gp;
    int w, h;
    GdkGC *gc;

    w = gdk_pixbuf_get_width(bg);
    h = gdk_pixbuf_get_height(bg);
    if (!(p = gnomecc_make_root_pixmap(w, h)))
        return 0;
    if (!(gp = gdk_pixmap_foreign_new(p))) {
        fprintf(stderr, "can't get gdk wrapper for root pixmap\n");
        return 0;
    }
    gc = gdk_gc_new_with_values(gp, NULL, 0);
    gdk_draw_pixbuf(gp, gc, bg, 0, 0, 0, 0, w, h, GDK_RGB_DITHER_NONE, 0, 0);
    gdk_gc_unref(gc);
    gnomecc_set_root_pixmap(p);
    return 1;
}
#define ARG_FILE_LEN 80

static void save_args(int argc, char *argv[])
{
    int i;
    FILE *fp;
    char path[ARG_FILE_LEN];

    snprintf(path, sizeof(path), "%s/.bg_cmd", getenv("HOME"));
    fp = fopen(path, "w");
    if (!fp)
        return;
    fprintf(fp, "%s ", argv[0]);
    for (i = 1; i < argc; i++)
        fprintf(fp, "\"%s\" ", argv[i]);
    fprintf(fp, "\n");
}

int
main(int argc, char *argv[])
{
    GOptionContext *context;
    GError *error = NULL;

    gtk_set_locale();
    context = g_option_context_new("- sets desktop background image");
    g_option_context_add_main_entries(context, entries, NULL);
    g_option_context_add_group(context, gtk_get_option_group(TRUE));
    g_option_context_set_description(context, desription);
    save_args(argc, argv);
    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_print ("%s\n", error->message);
        exit (1);
    }
    gtk_init(&argc, &argv);
    if (argc == 2 && !image) {
        image = argv[1];
        argc--;
    }
    if (argc > 1) {
        g_print("Unknown option %s.\nRun '%s --help' for description\n",
           argv[1],  g_get_prgname());
        exit(1);
    }
    dpy = GDK_DISPLAY();
    root = GDK_ROOT_WINDOW();
    if (make_bg() && read_image() && apply_image() && set_bg())
        return 0;

    return 1;
}
