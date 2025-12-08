MODULE := engines/lukas

MODULE_OBJS = \
	lukas.o \
	console.o \
	dialogue.o \
	events.o \
	resource.o \
	room.o \
	room_view.o \
	messages.o \
	metaengine.o \
	screen_anim_view.o \
	special_screen_view.o \
	tile_scene.o \
	view.o \
	view1.o

# This module can be built as a plugin
ifeq ($(ENABLE_LUKAS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
