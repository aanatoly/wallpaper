
#include <glib.h>
#include <glib/gprintf.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk/gdk.h>


#include "common.h"

//#define DEBUGPRN
#include "dbg.h"


typedef struct {
    char *str;
    int num;
} pair_t;

static GdkColor rgb = { .red = 0xff, .green = 0xd2, .blue = 0xac };
static scale_t scale = SCALE_COVER; 
static gravity_t gravity = GRV_CENTER;
static int verbose;


static pair_t scale_pair[] = {
    { "none", SCALE_NONE },
    { "fit", SCALE_FIT },
    { "cover", SCALE_COVER },
    { NULL, 0 }
};

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
parse_color(const gchar *option_name, const gchar *value,
    gpointer data, GError **error)
{
    return gdk_color_parse(value, &rgb);
}
        

static gboolean
parse_scale(const gchar *option_name, const gchar *value,
        gpointer data, GError **error)
{
    scale = str2num(value, scale_pair, -1);
    return (scale != -1);
}

static char *desription = "For example:\n  wallpaper --color=#ffd2ac "
  "--scale=fit  ~/pics/img002.jpg\n";

static GOptionEntry entries[] =
{
    { "verbose", 0, 0, G_OPTION_ARG_NONE, &verbose,
        "Verbose output", NULL },
    { "color", 0, 0, G_OPTION_ARG_CALLBACK, &parse_color,
        "BG color, name or '#RRGGBB' format", "C" },
    { "scale", 0, 0, G_OPTION_ARG_CALLBACK, parse_scale,
        "Scale type: fit, cover or none", "type" },

    { NULL }
};

int
parse_args(struct config_t *c, int argc, char *argv[])
{
    GOptionContext *context;
    GError *error = NULL;
    
    context = g_option_context_new("- sets desktop background image");
    g_option_context_add_main_entries(context, entries, NULL);
    g_option_context_set_description(context, desription);

    //save_args(argc, argv);
    if (!g_option_context_parse (context, &argc, &argv, &error)) 
        g_error("%s\n", error->message);

    c->gravity = gravity;
    c->verbose = verbose;
    c->scale = scale;
    c->rgb = rgb;
    c->image = argv[1]; // should be NULL if image was not set
    DBG("gravity %d, verbose %d, scale %d, color (%x,%x,%x)\n",
        c->gravity, c->verbose, c->scale,
        c->rgb.red, c->rgb.green, c->rgb.blue);
    DBG("image '%s'\n", c->image);
    return 0;
}
