########################################################################
####################### Makefile Template ##############################
########################################################################

# Compiler settings - Can be customized.
CC = g++
CXXFLAGS = -std=c++17 -O3 -Isrc/inc/ -DCPPHTTPLIB_OPENSSL_SUPPORT
LDFLAGS = -lpthread  -lssl -lcrypto

# Makefile settings - Can be customized.
APPNAME = webhook
EXT = .cpp
SRCDIR = src
OBJDIR = obj

# code info
CODE_VERSION = $(shell (git rev-parse --short HEAD 2>/dev/null || echo '0000000'))
CODE_DATE = $(shell (git log -1 --format=%cd 2>/dev/null || echo 'Fri Nov 18 00:00:00 2022 +0800'))
BUILD_MACHINE_INFO = $(shell uname -srp)
BUILD_MACHINE_FULL_INFO = $(shell uname -a)

# Get the latest git tag
LATEST_GIT_TAG := $(shell git describe --tags --abbrev=0)

# If the tag starts with 'v', add it to the macro definition
ifeq ($(firstword $(LATEST_GIT_TAG)),v)
	CXXFLAGS += -DVERSION_TAG="\"$(LATEST_GIT_TAG)\""
else
	CXXFLAGS += -DVERSION_TAG="\"Dev_$(LATEST_GIT_TAG)\""
endif

CXXFLAGS += -DAPPNAME="\"$(APPNAME)\""
CXXFLAGS += -DCODE_VERSION="\"$(CODE_VERSION)\""
CXXFLAGS += -DCODE_DATE="\"$(CODE_DATE)\""
CXXFLAGS += -DBUILD_MACHINE_INFO="\"$(BUILD_MACHINE_INFO)\""
CXXFLAGS += -DBUILD_MACHINE_FULL_INFO="\"$(BUILD_MACHINE_FULL_INFO)\""
CXXFLAGS += -DLATEST_GIT_TAG="\"$(LATEST_GIT_TAG)\""

############## Do not change anything from here downwards! #############
SRC = $(wildcard $(SRCDIR)/*$(EXT))
OBJ = $(SRC:$(SRCDIR)/%$(EXT)=$(OBJDIR)/%.o)
DEP = $(OBJ:$(OBJDIR)/%.o=%.d)
# UNIX-based OS variables & settings
RM = rm
DELOBJ = $(OBJ)
# Windows OS variables & settings
DEL = del
EXE = .exe
WDELOBJ = $(SRC:$(SRCDIR)/%$(EXT)=$(OBJDIR)\\%.o)

########################################################################
####################### Targets beginning here #########################
########################################################################

all: $(APPNAME)

# Builds the app
$(APPNAME): $(OBJ)
	$(CC) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Creates the dependecy rules
%.d: $(SRCDIR)/%$(EXT)
	$(shell if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi)
	@$(CPP) $(CFLAGS) $< -MM -MT $(@:%.d=$(OBJDIR)/%.o) >$@

# Includes all .h files
-include $(DEP)

# Run the app
run: $(APPNAME)
	@./$(APPNAME)

# Building rule for .o files and its .c/.cpp in combination with all .h
$(OBJDIR)/%.o: $(SRCDIR)/%$(EXT)
	$(CC) $(CXXFLAGS) -o $@ -c $<

################### Cleaning rules for Unix-based OS ###################
# Cleans complete project
.PHONY: clean
clean:
	$(RM) -f $(DELOBJ) $(DEP) $(APPNAME)

# Cleans only all files with the extension .d
.PHONY: cleandep
cleandep:
	$(RM) -f $(DEP)

#################### Cleaning rules for Windows OS #####################
# Cleans complete project
.PHONY: cleanw
cleanw:
	$(DEL) $(WDELOBJ) $(DEP) $(APPNAME)$(EXE)

# Cleans only all files with the extension .d
.PHONY: cleandepw
cleandepw:
	$(DEL) $(DEP)