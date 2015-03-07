## miniconf makefiles ## 1.1 ##

TOPDIR := ..

gtksetroot_src = gtksetroot.c
#gtksetroot_cflags += $(GDK_PIXBUF2_CFLAGS) $(X11_CFLAGS)
#gtksetroot_libs += $(GDK_PIXBUF2_LIBS) $(X11_LIBS)
gtksetroot_cflags += $(GTK2_CFLAGS) $(X11_CFLAGS)
gtksetroot_libs += $(GTK2_LIBS) $(X11_LIBS)
gtksetroot_type = bin


PKGS = GDK2 GTK2  GDK_PIXBUF X11

wallpaper_src = wallpaper.c args.c
wallpaper_cflags += $(foreach v,$(PKGS),$($(v)_CFLAGS))
wallpaper_libs += $(foreach v,$(PKGS),$($(v)_LIBS))
wallpaper_type = bin


include $(TOPDIR)/.config/rules.mk
