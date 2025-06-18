.SUFFIXES:

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules

TARGET := Kekatsu
BUILD := build
RELEASE := release
SOURCES := source source/utils source/gui
DATA := data
INCLUDES := include
GRAPHICS := source/gfx source/gui/gfx
LANGUAGES := source/lang
VERSION := 1.1.0

ARCH := -march=armv5te -mtune=arm946e-s -mthumb
CFLAGS := -g -Wall -O2 -ffunction-sections -fdata-sections $(ARCH) $(INCLUDE) -DARM9
CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions
ASFLAGS := -g $(ARCH)
LDFLAGS := -specs=ds_arm9.specs -g $(ARCH) -Wl,-Map,$(notdir $*.map)
LIBS := -lminizip -lcurl -lmbedtls -lmbedcrypto -lmbedx509 -lpng -lm -lz -ldswifi9 -lfat -lnds9
LIBDIRS := $(PORTLIBS) $(LIBNDS)

ifneq ($(BUILDDIR), $(CURDIR))

export OUTPUT := $(CURDIR)/$(TARGET)
export VPATH := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
				$(foreach dir,$(DATA),$(CURDIR)/$(dir)) \
				$(foreach dir,$(GRAPHICS),$(CURDIR)/$(dir)) \
				$(foreach dir,$(LANGUAGES),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)

CFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
PNGFILES := $(foreach dir,$(GRAPHICS),$(notdir $(wildcard $(dir)/*.png)))
LANGFILES := $(foreach dir,$(LANGUAGES),$(notdir $(wildcard $(dir)/*.lang*)))

ifeq ($(strip $(CPPFILES)),)
export LD := $(CC)
else
export LD := $(CXX)
endif

export OFILES_LANG := $(addsuffix .o,$(LANGFILES))
export OFILES_SOURCES := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES := $(PNGFILES:.png=.o) $(OFILES_LANG) $(OFILES_SOURCES)
export HFILES := $(PNGFILES:.png=.h) $(addsuffix .h,$(subst .,_,$(LANGFILES)))
export INCLUDE := $(foreach dir,$(INCLUDES),-iquote $(CURDIR)/$(dir)) \
				$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
				-I$(CURDIR)/$(BUILD)
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

export GAME_TITLE := Kekatsu
export GAME_SUBTITLE1 := DS(i) content downloader
export GAME_SUBTITLE2 := Cavv
export GAME_ICON := $(CURDIR)/icon.bmp

.PHONY: $(BUILD) clean

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@make BUILDDIR=`cd $(BUILD) && pwd` --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

$(RELEASE): $(BUILD)
	@[ -d $@ ] || mkdir -p $@
	@mv $(TARGET).nds $(RELEASE)/$(TARGET).nds
	@echo $(VERSION) > $(RELEASE)/version.txt

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).nds $(RELEASE)

else

DEPENDS := $(OFILES:.o=.d)

$(OUTPUT).nds: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

%.s %.h: %.png %.grit
	grit $< -fts -o$*

%.lang.o %_lang.h : %.lang
	@echo $(notdir $<)
	@$(bin2o)

-include $(DEPENDS)

endif
