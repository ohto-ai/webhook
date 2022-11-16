########################################################################
####################### Makefile Template ##############################
########################################################################

# Compiler settings - Can be customized.
CC = g++
CXXFLAGS = -std=c++14 -O3 -Iinc/ -DCPPHTTPLIB_OPENSSL_SUPPORT
LDFLAGS = -lpthread  -lssl -lcrypto

# Makefile settings - Can be customized.
APPNAME = pull-hook
EXT = .cpp
SRCDIR = .
OBJDIR = obj

# code info
export CODE_VERSION = $(shell (git rev-parse --short HEAD))
export CODE_DATE = $(shell (git log -1 --format=%cd ))

CXXFLAGS += -DCODE_VERSION="$(CODE_VERSION)"
CXXFLAGS += -DCODE_DATE="$(CODE_DATE)"

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