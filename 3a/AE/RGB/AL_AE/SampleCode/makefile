CC := gcc 
AR = ar
CFLAGS = -g -Wall -fno-common -Wunused-but-set-variable -Wno-implicit-function-declaration -Wno-missing-braces -O3 -std=c99 

# directory where the object file stored
OBJ := ./obj

# directory where the library (.a) file stored
LIBDIR_ := ./lib

# header directory 
# separate each directory by 'space' or by '\' to newline
# make sure there is no 'space' after '\'
INCDIR_ := ../Ae/alAELib/include \
		../Awb/alAWBLib/Core/include \
		../Afd/alAFDLib/include \
		../Lsc/Gravity/include \
		../../../Include \
		../alWrapper/include

# find the .c files from the following path
# separate path by '\' to newline
# make sure there is no 'space' after '\'
SOURCEPATH_ := ./ \
	       ../Ae/alAELib \
		   ../Awb/alAWBLib \
		   ../Afd/alAFDLib \
		   ../Awb/alAWBLib/Core \
		   ../Lsc/Gravity \
	       ../alWrapper

# list .c sources file to be compiled
# separate by '\' to newline
# make sure there is no 'space' after '\'
SOURCES := alAELibCore.c \
	   alAELib_iAE.c \
	   alAWBLib_AWB.c \
	   Altek_AWB.c \
	   alAFDLib_Core.c \
	   alAFDLib_DFT.c \
	   allib_lsc_core.c

# target library
LIB := lib4aLib.a 

# wrapper for std C function implementation
WRAPPER := alwrapper.c 

TARGET_ :=ICC_RGBonly.c

ifeq ($(OS), Windows_NT)
	RM =del /f /q 
	SEP =$(subst /,\,$1)
else
	RM =rm -rf
	SEP =$1
endif
ABSOBJ := $(realpath $(OBJ))
TARGET :=$(basename $(TARGET_))
SOURCEPATH :=$(realpath $(call SEP,$(SOURCEPATH_))) 
VPATH= $(SOURCEPATH) 
DEPS :=$(call SEP,$(foreach hdir, $(INCDIR_), $(wildcard $(hdir)/*.h)))
OBJECTS :=$(patsubst %.c, $(ABSOBJ)/%.o, $(SOURCES))
WRAPPEROBJ :=$(WRAPPER:.c=.o)
LIBDIR :=$(call SEP,$(addprefix -L, $(realpath $(LIBDIR_))))
INCDIR :=$(call SEP,$(addprefix -I, $(realpath $(INCDIR_))))

all:$(LIBDIR_)/$(LIB) $(ABSOBJ)/$(WRAPPEROBJ) $(TARGET) 

$(LIBDIR_)/$(LIB): $(OBJECTS)
	$(AR) rcs $(call SEP,$@) $(call SEP,$^)

$(ABSOBJ)/%.o: %.c $(DEPS) 
	$(CC) $(CFLAGS) -c $(call SEP,$<) -o $(call SEP,$@) $(INCDIR)

$(ABSOBJ)/$(WRAPPEROBJ): $(WRAPPER) 
	$(CC) $(CFLAGS) -c $(call SEP,$<) -o $(call SEP,$@) $(INCDIR) -D CLIB

$(TARGET): $(TARGET_) $(call SEP,$(LIBDIR_)/$(LIB)) $(call SEP,$(OBJ)/$(WRAPPEROBJ)) 
	$(CC) $(CFLAGS) -o $@ $^ $(INCDIR) $(LIBDIR)
#AE_main_withPseudo: AE_main_withPseudo.c $(OBJ)/$(WRAPPEROBJ) $(LIBDIR_)/$(LIB)
#	$(CC) $(CFLAGS) -o $@ $(call SEP,$^) $(INCDIR) $(LIBDIR)

#AE_main_externalLux: AE_main_externalLux.c $(OBJ)/$(WRAPPEROBJ) $(LIBDIR_)/$(LIB)
#	$(CC) $(CFLAGS) -o $@ $(call SEP,$^) $(INCDIR) $(LIBDIR)

.PHONY: all clean
clean:
	$(RM) $(call SEP,$(realpath $(wildcard $(OBJ)/*.o))) $(call SEP,$(realpath $(LIBDIR_)/$(LIB))) $(call SEP,$(wildcard *.exe))
