#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

include $(DEVKITPPC)/wii_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET := jumpnbump
#TARGET      :=  $(notdir $(CURDIR))
BUILD       :=  build
SOURCES     :=  . sdl
DATA        :=  data  
INCLUDES    :=


SDL_TARGET := sdl.a
MODIFY_TARGET := gobpack jnbpack jnbunpack
OBJS := fireworks.o main.o menu.o filter.o
BINARIES := $(TARGET) jumpnbump.svgalib jumpnbump.fbcon $(MODIFY_TARGET) \
	jnbmenu.tcl
PREFIX-JNB ?= /data

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------

SDL_CFLAGS := `sdl-config --cflags`
SDL_LIBS := `sdl-config --libs`
#CFLAGS = -Wall -O2 $(MACHDEP) -ffast-math -funroll-loops -Dstricmp=strcasecmp \
#    -Dstrnicmp=strncasecmp -DUSE_SDL -DNDEBUG -I. $(SDL_CFLAGS) \
#    -I$(DEVKITPPC)/local/include -DUSE_NET -DZLIB_SUPPORT -DBZLIB_SUPPORT


CFLAGS  = -g -O2 -mrvl -Wall $(MACHDEP) $(INCLUDE) -I$(DEVKITPPC)/local/include \
            -DUSE_SDL -DZLIB_SUPPORT $(SDL_CFLAGS)
CXXFLAGS    =   $(CFLAGS)

LDFLAGS =   -g $(MACHDEP) -mrvl -Wl,-Map,$(notdir $@).map

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
#LIBS    :=  -lwiisprite $(SDL_LIBS) -lSDL_mixer -lSDL_net -lbz2 -lpng -lz -lwiiuse -lbte -lfat -logc -lm
LIBS    :=  -lSDL_mixer -lz $(SDL_LIBS) -lwiiuse -lbte -lfat -logc -lm  


#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS := 

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT   :=  $(CURDIR)/$(TARGET)

export VPATH    :=  $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
					$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR  :=  $(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# automatically build a list of object files for our project
#---------------------------------------------------------------------------------
CFILES      :=  $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES    :=  $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
sFILES      :=  $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES      :=  $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
BINFILES    :=  $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
	export LD   :=  $(CC)
else
	export LD   :=  $(CXX)
endif

#export OFILES   :=  $(addsuffix .o,$(BINFILES))
export OFILES   :=  $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) \
					$(sFILES:.s=.o) $(SFILES:.S=.o)

#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE  :=  $(foreach dir,$(INCLUDES), -iquote $(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD) \
					-I$(LIBOGC_INC)

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
export LIBPATHS :=  $(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
					-L$(LIBOGC_LIB) -L$(DEVKITPPC)/local/lib

export OUTPUT   :=  $(CURDIR)/$(TARGET)
.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).dol

#clean:
#    cd sdl && make clean
#    cd modify && make clean
#    cd data && make clean
#    rm -f $(TARGET) *.o globals.h jnbmenu.tcl

#---------------------------------------------------------------------------------
else

DEPENDS :=  $(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).dol: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

.PHONY: data

all: $(BINARIES)

$(SDL_TARGET): globals.h
	cd sdl && make

$(MODIFY_TARGET): globals.h
	cd modify && make

$(TARGET): $(OBJS) $(SDL_TARGET) data globals.h
	$(CC) -o $(TARGET) $(OBJS) $(LIBS) $(SDL_TARGET)

$(OBJS): globals.h

globals.h: globals.pre
	sed -e "s#%%PREFIX%%#$(PREFIX-JNB)#g" < globals.pre > globals.h

jnbmenu.tcl: jnbmenu.pre
	sed -e "s#%%PREFIX%%#$(PREFIX-JNB)#g" < jnbmenu.pre > jnbmenu.tcl

data: jnbpack
	cd data && make

#---------------------------------------------------------------------------------
# This rule links in binary data with the .jpg extension
#---------------------------------------------------------------------------------
%.jpg.o :   %.jpg
#---------------------------------------------------------------------------------
	@echo $(notdir $<)
	$(bin2o)

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------

