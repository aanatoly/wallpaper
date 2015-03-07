
#ifndef _COMMON_H_
#define _COMMON_H_

#include <gdk/gdk.h>

typedef enum { SCALE_NONE, SCALE_FIT, SCALE_COVER } scale_t;
typedef enum { GRV_CENTER } gravity_t;

struct config_t {
    GdkColor rgb;
    char *image;    /* image file name */
    scale_t scale;  /* scale type */
    gravity_t gravity;
    int verbose;
};

int parse_args(struct config_t *c, int argc, char *argv[]);

#endif
