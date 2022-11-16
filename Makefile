########################################################################
####################### Makefile Template ##############################
########################################################################

# Compiler settings - Can be customized.
CC = g++
CXXFLAGS = -std=c++14 -O3 -Iinc/ -DCPPHTTPLIB_OPENSSL_SUPPORT
LDFLAGS = -lpthread  -lssl -lcrypto

# Makefile settings - Can be customized.
APPNAME = webhook
EXT = .cpp
SRCDIR = .
OBJDIR = obj

# code info
export CODE_VERSION = $(shell (git rev-parse --short HEAD))
export CODE_DATE = $(shell (git log -1 --format=%cd ))
export CODE_SERVER_PATH = $(shell git remote -v | awk 'NR==1' | sed 's/[()]//g' | sed 's/\t/ /g' |cut -d " " -f2)
export BUILD_MACHINE_INFO = $(shell uname -srp)
export BUILD_MACHINE_FULL_INFO = $(shell uname -a)

CXXFLAGS += -DAPPNAME="\"$(APPNAME)\""
CXXFLAGS += -DCODE_VERSION="\"$(CODE_VERSION)\""
CXXFLAGS += -DCODE_DATE="\"$(CODE_DATE)\""
CXXFLAGS += -DCODE_SERVER_PATH="\"$(CODE_SERVER_PATH)\""
CXXFLAGS += -DBUILD_MACHINE_INFO="\"$(BUILD_MACHINE_INFO)\""
CXXFLAGS += -DBUILD_MACHINE_FULL_INFO="\"$(BUILD_MACHINE_FULL_INFO)\""

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

USERNAME_REPONAME=`git remote -v | grep -Po "(?<=:)[a-zA-Z0-9\\._/-]+?(?=((\\.git)? \\(fetch\\)))"`

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