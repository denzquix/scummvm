MODULE := engines/lukas

MODULE_OBJS = \
	lukas.o \
	console.o \
	events.o \
	resource.o \
	room.o \
	messages.o \
	metaengine.o \
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
