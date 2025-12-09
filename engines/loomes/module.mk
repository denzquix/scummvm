MODULE := engines/loomes

MODULE_OBJS = \
	loomes.o \
	console.o \
	events.o \
	messages.o \
	metaengine.o \
	view.o \
	view1.o

# This module can be built as a plugin
ifeq ($(ENABLE_LOOMES), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
