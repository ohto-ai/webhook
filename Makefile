CC = g++
CXXFLAGS = -std=c++14 -O3 -Iinc/ -DCPPHTTPLIB_OPENSSL_SUPPORT
LDFLAGS = -lpthread  -lssl -lcrypto

APPNAME = webhook
EXT = .cpp
SRCDIR = src
OBJDIR = obj

# code info
COMMIT_HASH = $(shell (git rev-parse --short HEAD 2>/dev/null || echo '0000000'))
COMMIT_DATE = $(shell (git log -1 --format=%cd 2>/dev/null || echo 'Fri Nov 18 00:00:00 2022 +0800'))
BUILD_MACHINE_INFO = $(shell uname -srp)
BUILD_MACHINE_FULL_INFO = $(shell uname -a)

CXXFLAGS += -DAPPNAME="\"$(APPNAME)\""
CXXFLAGS += -DCOMMIT_HASH="\"$(COMMIT_HASH)\""
CXXFLAGS += -DCOMMIT_DATE="\"$(COMMIT_DATE)\""
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