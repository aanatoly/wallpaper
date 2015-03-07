## miniconf makefiles ## 1.1 ##

TOPDIR := ..

#PKGS = GDK2 GTK2  GDK_PIXBUF X11
PKGS = GTK2 X11

wallpaper_src = wallpaper.c args.c
wallpaper_cflags += $(foreach v,$(PKGS),$($(v)_CFLAGS))
wallpaper_libs += $(foreach v,$(PKGS),$($(v)_LIBS))
wallpaper_type = bin


include $(TOPDIR)/.config/rules.mk
