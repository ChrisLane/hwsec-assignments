override CFLAGS  += -Wall -Werror -Wno-unused-variable -Wno-unused-function -Wno-unused-parameter -Wno-missing-braces -fstack-protector -O3 -g
override LDFLAGS += -shared -Wl,--as-needed

SRCDIR = .
OBJDIR = obj

# Source and header files
SRC = $(shell find $(SRCDIR) -type f -name '*.c')
INC = $(shell find $(INCDIR) -type f -name '*.h')
OBJ = $(patsubst $(SRCDIR)%,$(OBJDIR)%,$(patsubst %.c, %.o, $(SRC)))

# Target declarations
LIB = liblongnum
SHARED_LIB = $(LIB).so
STATIC_LIB = $(LIB).a

.PHONY: default all build
default: all
all: build
build: $(SHARED_LIB) $(STATIC_LIB)

# Compilation
$(SHARED_LIB): $(OBJ)
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(STATIC_LIB): $(OBJ)
	$(AR) rcs $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(INC)
	@mkdir -p $(@D)
	$(CC) -fPIC $(INCLUD) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	$(RM) -r *.i *.s $(OBJDIR) $(SHARED_LIB) $(STATIC_LIB)
