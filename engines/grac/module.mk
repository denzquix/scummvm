MODULE := engines/grac

MODULE_OBJS = \
	amos/memorybank.o \
	grac.o \
	console.o \
	data.o \
	events.o \
	game.o \
	room.o \
	messages.o \
	metaengine.o \
	script.o \
	view.o \
	view1.o

# This module can be built as a plugin
ifeq ($(ENABLE_GRAC), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
