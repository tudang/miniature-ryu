# project name (generate executable with this name)
TARGET   = netpaxos

CC       = gcc
# compiling flags here
CFLAGS   = -std=c99 -Wall -I.

LINKER   = gcc -o
# linking flags here
LFLAGS   = -Wall -I. -levent

# change these to set the proper directories where each files shoould be
SRCDIR   = src
OBJDIR   = obj

SOURCES  := $(wildcard $(SRCDIR)/*.c)
INCLUDES := $(wildcard $(SRCDIR)/*.h)
OBJECTS  := $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)


$(TARGET): $(OBJECTS)
	@$(LINKER) $@ $(LFLAGS) $(OBJECTS)
	@echo "Linking complete!"

$(OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c | $(OBJDIR)
	@$(CC) $(CFLAGS) -c $< -o $@
	@echo "Compiled "$<" successfully!"

$(OBJDIR):
	mkdir -p $(OBJDIR)
.PHONEY: clean
clean:
	rm -rf $(TARGET) $(OBJDIR)
	@echo "Cleanup complete!"
