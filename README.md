# wallpaper #

## what ##
**wallpaper** is gtk2 only wallpaper seting utility for X11 desktop. It can use
an image or color or both.

Supported image types, thanks to gtk2, are: jpg, gif, png, svg, tiff, bmp, xbm, pcx, ico and few more.

## why ##
I needed some background seting tool for my gtk2/openbox desktop, that does not
use extra libs (like imlib2 or ImageMagick) for things that gtk2 perfectly does
itself.

## how ##
The operation is quite simple: scale an image, position it, fill the rest of the desktop with a color.

```
$ wallpaper -h
Usage:
  wallpaper [OPTION...] - sets desktop background image

Help Options:
  -h, --help       Show help options

Application Options:
  --verbose        Verbose output
  --color=C        BG color, name or '#RRGGBB' format
  --scale=type     Scale type: fit, cover or none

For example:
  wallpaper --color=#ffd2ac --scale=fit  ~/pics/img002.jpg

```

### Scaling ###
Scaling always keeps image's aspect ratio and has 2 modes:

 * **cover** - scale an image to cover entire desktop.
 * **fit** - scale image to fit into the desktop. 


### Placement ###
Currently, image is placed at the center. In a future, I'll add alignment to
edges (left, right, top, bottom).


## extra ##
There is also helper GUI tool - **wallpaper-gui**, written in python

<a href="http://aanatoly.github.io/wallpaper/shot.jpeg">
<img src="http://aanatoly.github.io/wallpaper/shot-th.jpeg"></a>
