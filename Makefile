# Nuke built-in rules and variables.
override MAKEFLAGS += -rR
 
# This is the name that our final kernel executable will have.
# Change as needed.
override KERNEL := myos.elf
 
# Convenience macro to reliably declare user overridable variables.
define DEFAULT_VAR =
    ifeq ($(origin $1),default)
        override $(1) := $(2)
    endif
    ifeq ($(origin $1),undefined)
        override $(1) := $(2)
    endif
endef
 
# It is highly recommended to use a custom built cross toolchain to build a kernel.
# We are only using "cc" as a placeholder here. It may work by using
# the host system's toolchain, but this is not guaranteed.
override DEFAULT_CC := cc
$(eval $(call DEFAULT_VAR,CC,$(DEFAULT_CC)))
 
# Same thing for "ld" (the linker).
override DEFAULT_LD := ld
$(eval $(call DEFAULT_VAR,LD,$(DEFAULT_LD)))
 
# User controllable C flags.
override DEFAULT_CFLAGS := -g -O2 -pipe
$(eval $(call DEFAULT_VAR,CFLAGS,$(DEFAULT_CFLAGS)))
 
# User controllable C preprocessor flags. We set none by default.
override DEFAULT_CPPFLAGS :=
$(eval $(call DEFAULT_VAR,CPPFLAGS,$(DEFAULT_CPPFLAGS)))
 
# User controllable nasm flags.
override DEFAULT_NASMFLAGS := -F dwarf -g
$(eval $(call DEFAULT_VAR,NASMFLAGS,$(DEFAULT_NASMFLAGS)))
 
# User controllable linker flags. We set none by default.
override DEFAULT_LDFLAGS :=
$(eval $(call DEFAULT_VAR,LDFLAGS,$(DEFAULT_LDFLAGS)))
 
# Internal C flags that should not be changed by the user.
override CFLAGS += \
    -Wall \
    -Wextra \
    -std=gnu11 \
    -ffreestanding \
    -fno-stack-protector \
    -fno-stack-check \
    -fno-lto \
    -fno-PIE \
    -fno-PIC \
    -m64 \
    -march=x86-64 \
    -mabi=sysv \
    -mno-80387 \
    -mno-mmx \
    -mno-sse \
    -mno-sse2 \
    -mno-red-zone \
    -mcmodel=kernel

override CFLAGS += \
    -g \
    -O0 \

# Internal C preprocessor flags that should not be changed by the user.
override CPPFLAGS := \
    -I. \
    $(CPPFLAGS) \
    -MMD \
    -MP
 
# Internal linker flags that should not be changed by the user.
override LDFLAGS += \
    -nostdlib \
    -static \
    -m elf_x86_64 \
    -z max-page-size=0x1000 \
    -T linker.ld
 
# Check if the linker supports -no-pie and enable it if it does.
ifeq ($(shell $(LD) --help 2>&1 | grep 'no-pie' >/dev/null 2>&1; echo $$?),0)
    override LDFLAGS += -no-pie
endif
 
# Internal nasm flags that should not be changed by the user.
override NASMFLAGS += \
    -Wall \
    -f elf64
 
# Use "find" to glob all *.c, *.S, and *.asm files in the tree and obtain the
# object and header dependency file names.
override CFILES := $(shell find -L . -type f -name '*.c' | grep -v 'limine/' | grep -v '6.828-qemu/')
override ASFILES := $(shell find -L . -type f -name '*.S' | grep -v 'limine/' | grep -v '6.828-qemu/')
override NASMFILES := $(shell find -L . -type f -name '*.asm' | grep -v 'limine/' | grep -v '6.828-qemu/')
override FONTFILES := $(shell find -L . -type f -name '*.psf' | grep -v 'limine/' | grep -v '6.828-qemu/')
override OBJ := $(CFILES:.c=.o) $(ASFILES:.S=.o) $(NASMFILES:.asm=.o) $(FONTFILES:.psf=.o)
override HEADER_DEPS := $(CFILES:.c=.d) $(ASFILES:.S=.d)
 
# Default target.
.PHONY: all
all: $(KERNEL)
 
# Link rules for the final kernel executable.
$(KERNEL): $(OBJ)
	$(LD) $(OBJ) $(LDFLAGS) -o $@
 
# Include header dependencies.
-include $(HEADER_DEPS)
 
# Compilation rules for *.c files.
%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@
 
# Compilation rules for *.S files.
%.o: %.S
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@
 
# Compilation rules for *.asm (nasm) files.
%.o: %.asm
	nasm $(NASMFLAGS) $< -o $@

%.o: %.psf
	objcopy -O elf64-x86-64 -B i386 -I binary $<  $@

# Remove object files and the final executable.
.PHONY: clean
clean:
	rm -rf $(KERNEL) $(OBJ) $(HEADER_DEPS)

.PHONY: iso
iso:
	./build.sh

.PHONY: run
run:
	./run.sh