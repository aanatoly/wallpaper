## miniconf makefiles ## 1.1 ##

TOPDIR := ..

#PKGS = GDK2 GTK2  GDK_PIXBUF X11
PKGS = GTK2 X11

wallpaper_src = wallpaper.c args.c
wallpaper_cflags += $(foreach v,$(PKGS),$($(v)_CFLAGS))
wallpaper_libs += $(foreach v,$(PKGS),$($(v)_LIBS))
wallpaper_type = bin


# desktop entry file
all: wallpaper.desktop
CLEANLIST += wallpaper.desktop wallpaper.mime-types

wallpaper.desktop: wallpaper.desktop.in wallpaper.mime-types
	$(call summary,TEXT  ,$@)
	$Qcat $? > $@

wallpaper.mime-types:
	$Q./gdk_pixbuf_mime-types > $@

install:
	$Qinstall -D -m 755 -T wallpaper-gui \
		$(DESTDIR)$(BINDIR)/wallpaper-gui
	$Qinstall -D -m 644 -T wallpaper.desktop \
		$(DESTDIR)$(DATADIR)/applications/wallpaper.desktop


include $(TOPDIR)/.config/rules.mk
