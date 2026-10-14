# Pepsiman (PSX / SLPS-01762) — matching decompilation.
#
# DO NOT RUN `make` DIRECTLY to build the executable. `./build-and-verify.sh`
# is the canonical entry point and the only thing that checks the result
# against the retail bytes; a build that is not verified tells you nothing.
# `make extract` is the one target meant to be run by hand.

GAME_ID   := slps01762
GAME      := pepsiman
TARGET    := SLPS_017.62

# --- toolchain -------------------------------------------------------------
# Everything is local to the repo, built or fetched by tools/setup.sh. Nothing
# here comes from $PATH: a host binutils that happens to be installed is how
# two machines silently build different bytes.
CROSS     := tools/binutils/bin/mipsel-linux-gnu-
AS        := $(CROSS)as
LD        := $(CROSS)ld
NM        := $(CROSS)nm
OBJCOPY   := $(CROSS)objcopy
OBJDUMP   := $(CROSS)objdump

# GCC 2.8.1, the Psy-Q compiler ("Sony Playstation" is in cc1's own banner).
# `cpp` is the same release's preprocessor — a modern cpp expands differently
# enough to move code. (2.6.3's cpp gives identical output on every unit.)
GCC_DIR   := tools/gcc
CPP       := $(GCC_DIR)/cpp
CC1       := $(GCC_DIR)/cc1

PYTHON    := .venv/bin/python3
SPLAT     := $(PYTHON) -m splat split
MASPSX    := $(PYTHON) tools/maspsx/maspsx.py

# --- flags -----------------------------------------------------------------
# NOTE `-fno-builtin` is a cc1 flag only; the Psy-Q cpp rejects it outright.
CPP_FLAGS  := -Iinclude -Iinclude/psyq -undef -Wall -lang-c -nostdinc
CPP_FLAGS  += -Dmips -D__GNUC__=2 -D__OPTIMIZE__ -D__mips__ -D__mips -Dpsx
CPP_FLAGS  += -D__psx__ -D__psx -D_PSYQ -D__EXTENSIONS__ -D_MIPSEL
CPP_FLAGS  += -D_LANGUAGE_C -DLANGUAGE_C

CC_FLAGS   := -mips1 -mcpu=3000 -quiet -Wall -fno-builtin -mno-abicalls
# -G8: 2.8.1 splits %hi/%lo itself, so maspsx never sees a symbolic load to
# turn gp-relative; cc1 has to. -G4 scores the same on every probe so far.
CC_FLAGS   += -funsigned-char -G8 -O2

# 2.56: retail assembles `li 0xFF` as addiu and `li 0xEFFE` as ori, which is
# GNU as's li, i.e. no maspsx li expansion (aspsx >= 2.50). 2.56..2.86 score
# the same so far. No --nop-at-expansion: lsddecomp's retail has a nop
# between a load and a following lui-$at store of the loaded register, and
# Pepsiman's has none at any of its 15 such sites (func_80015450 and five
# others); without the flag the image is unchanged and func_80015450 matches.
MASPSX_FLAGS := --aspsx-version=2.56 --dont-force-G0 --gp-symbols=config/gp-symbols.txt

# Each unit gets its own gp list (tools/gpsyms.py): the symbols retail
# reaches through $gp from that unit's code. A later --gp-symbols wins, so
# MASPSX_FLAGS keeps the whole-game list for tools that compile outside a unit.
UNIT_GP     = --gp-symbols=config/gp/$(notdir $*).txt

AS_FLAGS   := -Iinclude -Iinclude/psyq -march=r3000 -mtune=r3000 -EL
AS_FLAGS   += -no-pad-sections -G0 -O2

LD_FLAGS   := --no-check-sections -nostdlib -EL

# --- files -----------------------------------------------------------------
BUILD_DIR := build
CONFIG    := config

# `find`, not `wildcard`: wildcard has no recursive form, so the moment src/
# grows a subdirectory it silently returns fewer units and every count derived
# from it drops without a word.
# asm/ovl/ and src/ovl/ are the overlays' (see "overlays" below), never the
# executable's.
S_FILES  := $(shell find asm -name '*.s' -not -path 'asm/nonmatchings/*' -not -path 'asm/ovl/*' 2>/dev/null)
C_FILES  := $(shell find src -name '*.c' -not -path 'src/ovl/*' 2>/dev/null)
# Prebuilt Psy-Q library objects (splat `o` segments). lib/ is produced from
# the user's own SDK disc by tools/setup.sh and is gitignored; the linker
# script names them under build/lib/, so they are mirrored there.
# `-L`: in a runner worktree lib/ is a SYMLINK to the main checkout's (see
# tools/setup-worktree.sh), and plain `find lib` does not descend into a
# symlinked directory -- it returned nothing, build/lib/ stayed empty and the
# link failed on every `build/lib/<x>.o` the script names (first worktree
# provisioned after the SDK objects landed, 2026-09-11).
LIB_FILES := $(shell find -L lib -name '*.o' 2>/dev/null)
O_FILES  := $(foreach f,$(S_FILES),$(BUILD_DIR)/$(f).o) \
            $(foreach f,$(C_FILES),$(BUILD_DIR)/$(f).o) \
            $(foreach f,$(LIB_FILES),$(BUILD_DIR)/$(f))

ELF      := $(BUILD_DIR)/$(GAME).elf
EXE      := $(BUILD_DIR)/$(TARGET)

.PHONY: all build check extract clean format expected diff-init progress
.PHONY: overlays check-overlays
.DEFAULT_GOAL := all

# The compile rule is a PIPELINE ending in `as -o $@`. When cc1 fails, `as`
# still creates $@ from empty input; pipefail makes the recipe fail, but make
# leaves the (newer, empty) object in place, judges the unit up to date on the
# NEXT build, and silently links the stale object. Round 56 reproduced it:
# a header edit with the unit's .c untouched, red SHA1, no compile error in
# any later log. This is GNU make's own switch for exactly that: delete the
# target of any recipe that fails.
.DELETE_ON_ERROR:

all: build check overlays check-overlays

build: $(EXE)

check:
	sha1sum --check build.sha1

# --- link ------------------------------------------------------------------
# The image ends where its header says (tools/exe_trim.py): the last .sbss
# object runs past it, and only its zeros are cut.
$(EXE): $(ELF)
	$(OBJCOPY) -O binary $< $@
	$(PYTHON) tools/exe_trim.py $@

# psyq-objects.ld goes FIRST: it claims the Sony objects' bss sections (NOLOAD,
# pinned) before the splat script's trailing /DISCARD/ would swallow them.
$(ELF): $(O_FILES) $(GAME).ld $(CONFIG)/psyq-objects.ld \
		$(CONFIG)/undefined_syms.$(GAME_ID).$(GAME).txt
	@mkdir -p $(dir $@)
	$(LD) -o $@ \
		-Map $(BUILD_DIR)/$(GAME).map \
		-T $(CONFIG)/psyq-objects.ld \
		-T $(GAME).ld \
		-T $(CONFIG)/undefined_syms.$(GAME_ID).$(GAME).txt \
		-T $(CONFIG)/undefined_syms_auto.$(GAME_ID).$(GAME).txt \
		-T $(CONFIG)/undefined_funcs_auto.$(GAME_ID).$(GAME).txt \
		$(LD_FLAGS)

# Every rule from here on may use $$ in its prerequisites.
.SECONDEXPANSION:

# --- overlays --------------------------------------------------------------
# Code LoadOverlay reads from the disc to 0x800F0000 at run time
# (tools/extract_overlays.py puts the retail files in disk/ovl/). Each is its
# own image with its own splat config (config/ovl/<ovl>.yaml), linked alone
# against the executable's symbol addresses and checked against its line in
# check-overlays.sha1. An overlay is built once its name is in OVERLAYS.
#
# An overlay's C units are src/ovl/<ovl>.c and any src/ovl/<ovl>_*.c (the
# chases' <ovl>_tail.c), with their asm under asm/ovl/<ovl>/. Code several
# overlays share word for word lives in an include beside them
# (src/ovl/world_intro.inc), which every overlay unit depends on.
OVERLAYS  := ovl00 ovl02 ovl03 ovl04 ovl05 ovl06 ovl07 ovl08 ovl09 ovl10 ovl11 ovl12 ovl13 ovl14 ovl15 ovl16 ovl17 ovl18 ovl19 ovl20 ovl21 ovl22
# -G0, not the executable's -G8: retail overlay code reaches every global,
# a scalar like gActiveBuff included, by cc1's own %hi/%lo split over two
# registers (lui $v0 hoisted, lw $v1 later), where the executable's code has
# the assembler's same-register expansion. -G0 reproduces that order in
# ovl18's func_800F1318; -G8 does not. No overlay instruction uses $gp.
OVL_CC_FLAGS := $(filter-out -G8,$(CC_FLAGS)) -G0
OVL_DIR   := $(BUILD_DIR)/ovl
OVL_BINS  := $(foreach o,$(OVERLAYS),$(OVL_DIR)/$(o).bin)
OVL_S_FILES := $(shell find asm/ovl -name '*.s' -not -path '*/nonmatchings/*' 2>/dev/null)
OVL_OBJS   = $(BUILD_DIR)/src/ovl/$(1).c.o \
             $(foreach f,$(wildcard src/ovl/$(1)_*.c),$(BUILD_DIR)/$(f).o) \
             $(foreach f,$(filter asm/ovl/$(1)/%,$(OVL_S_FILES)),$(BUILD_DIR)/$(f).o) \
             $(foreach f,$(wildcard assets/$(1)/*.bin),$(BUILD_DIR)/$(f).o)

# Bytes splat keeps as a `bin` subsegment (assets/<ovl>/, generated, never
# committed): ovl00's file ends in a halfword its data words cannot hold.
$(BUILD_DIR)/assets/%.bin.o: assets/%.bin
	@mkdir -p $(dir $@)
	printf '.section .data\n.incbin "%s"\n' $< | $(AS) $(AS_FLAGS) -o $@

overlays: $(OVL_BINS)

# The retail hashes of the overlays in OVERLAYS, with disk/ swapped for build/.
# An empty selection fails: sha1sum rejects input with no checksum lines.
check-overlays:
	grep -E ' disk/ovl/($(subst $(eval) ,|,$(OVERLAYS)))\.bin$$' check-overlays.sha1 \
		| sed 's| disk/| $(BUILD_DIR)/|' | sha1sum --check

# Kept: make deletes the intermediate of a pattern chain, and funcdiff reads
# a matched function's range from this ELF's symbols.
.SECONDARY: $(OVL_BINS:.bin=.elf)

$(OVL_DIR)/%.bin: $(OVL_DIR)/%.elf
	$(OBJCOPY) -O binary $< $@

# Every name the executable's symbols file gives, as linker assignments: an
# overlay's C may name any executable symbol, not only those splat saw retail
# reach (it saw `D_80095768 + 0x2` where ovl18's C says gFogFade).
EXE_SYMS_LD := $(OVL_DIR)/exe_syms.ld
$(EXE_SYMS_LD): $(CONFIG)/symbols.$(GAME_ID).$(GAME).txt
	@mkdir -p $(dir $@)
	sed 's|//.*||' $< > $@

# The linker script is splat's (make extract writes it into build/ovl/).
$(OVL_DIR)/%.elf: $$(call OVL_OBJS,$$*) $(OVL_DIR)/%.ld $(EXE_SYMS_LD) \
		$(CONFIG)/undefined_syms.$(GAME_ID).$(GAME).txt \
		$(CONFIG)/ovl/undefined_syms_auto.%.txt $(CONFIG)/ovl/undefined_funcs_auto.%.txt
	$(LD) -o $@ \
		-Map $(OVL_DIR)/$*.map \
		-T $(OVL_DIR)/$*.ld \
		-T $(EXE_SYMS_LD) \
		-T $(CONFIG)/undefined_syms.$(GAME_ID).$(GAME).txt \
		-T $(CONFIG)/ovl/undefined_syms_auto.$*.txt \
		-T $(CONFIG)/ovl/undefined_funcs_auto.$*.txt \
		$(LD_FLAGS)

# --- compile ---------------------------------------------------------------
$(BUILD_DIR)/lib/%.o: lib/%.o
	@mkdir -p $(dir $@)
	cp $< $@

$(BUILD_DIR)/%.s.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(AS_FLAGS) -o $@ $<

# The Psy-Q pipeline. maspsx sits between cc1 and gas because Sony's ASPSX
# assembler expanded macros (div, li, ...) differently from GNU as, and those
# expansions are part of the retail bytes.
#
# A unit's .c depends on every .s it INCLUDE_ASMs, so re-extracting or matching
# a function rebuilds the unit, and on its <unit>_tables.inc, the data tables
# it #includes in place.
#
# IT ALSO DEPENDS ON EVERY HEADER, coarsely and on purpose. Without this a
# header edit does not rebuild anything, `make` reports success, and the next
# funcdiff scores a struct layout you changed minutes ago against an object
# that never saw it -- a stale number that is plausible and self-consistent,
# which is the worst kind. A full build is a few seconds; correctness is
# cheaper than the fine-grained version here.
HEADERS := $(wildcard include/*.h include/*.inc include/psyq/*.h \
                      include/psyq/sys/*.h)

# An overlay unit takes its $gp list from config/ovl/gp.<unit>.txt (written by
# hand from the overlay's own code until tools/gpsyms.py learns overlays) and
# its INCLUDE_ASMs from asm/ovl/<ovl>/nonmatchings/<unit>/. This rule's stem is
# shorter than the generic one's, so make prefers it for src/ovl/.
$(BUILD_DIR)/src/ovl/%.c.o: src/ovl/%.c $(HEADERS) $(wildcard src/ovl/*.inc src/ovl/engine/*.inc) config/ovl/gp.%.txt $$(wildcard asm/ovl/*/nonmatchings/$$*/*.s)
	@mkdir -p $(dir $@)
	$(CPP) $(CPP_FLAGS) $< | $(CC1) $(OVL_CC_FLAGS) | \
		$(MASPSX) $(MASPSX_FLAGS) --gp-symbols=config/ovl/gp.$*.txt | \
		$(AS) $(AS_FLAGS) -o $@

$(BUILD_DIR)/%.c.o: %.c $(HEADERS) config/gp/$$(notdir $$*).txt $$(wildcard $$*_tables.inc) $$(wildcard asm/nonmatchings/$$(notdir $$*)/*.s)
	@mkdir -p $(dir $@)
	$(CPP) $(CPP_FLAGS) $< | $(CC1) $(CC_FLAGS) | $(MASPSX) $(MASPSX_FLAGS) $(UNIT_GP) | \
		$(AS) $(AS_FLAGS) -o $@

# --- NON_MATCHING check ----------------------------------------------------
# Compiles every unit a SECOND time with -DNON_MATCHING into its own tree, so
# the readable bodies kept under `#ifdef NON_MATCHING` (FINISHING-PLAN.md,
# track 1b) are proven to at least compile. Nothing here is linked, verified
# or read by funcdiff: build/nonmatching/ is not the oracle and never becomes
# it. Drive it through tools/check-nonmatching.sh, which also checks that the
# bodies reference only symbols the real link defines.
NM_DIR   := $(BUILD_DIR)/nonmatching
NM_OBJS  := $(foreach f,$(C_FILES),$(NM_DIR)/$(f).o)

.PHONY: nonmatching
nonmatching: $(NM_OBJS)

$(NM_DIR)/src/ovl/%.c.o: src/ovl/%.c $(HEADERS) $(wildcard src/ovl/*.inc src/ovl/engine/*.inc) config/ovl/gp.%.txt $$(wildcard asm/ovl/*/nonmatchings/$$*/*.s)
	@mkdir -p $(dir $@)
	$(CPP) $(CPP_FLAGS) -DNON_MATCHING $< | $(CC1) $(OVL_CC_FLAGS) | \
		$(MASPSX) $(MASPSX_FLAGS) --gp-symbols=config/ovl/gp.$*.txt | \
		$(AS) $(AS_FLAGS) -o $@

$(NM_DIR)/%.c.o: %.c $(HEADERS) config/gp/$$(notdir $$*).txt $$(wildcard $$*_tables.inc) $$(wildcard asm/nonmatchings/$$(notdir $$*)/*.s)
	@mkdir -p $(dir $@)
	$(CPP) $(CPP_FLAGS) -DNON_MATCHING $< | $(CC1) $(CC_FLAGS) | $(MASPSX) $(MASPSX_FLAGS) $(UNIT_GP) | \
		$(AS) $(AS_FLAGS) -o $@

# --- extract ---------------------------------------------------------------
# splat NEVER deletes what it stops generating, and asm/ is gitignored, so
# orphaned .s files accumulate in a working checkout forever. They corrupt
# every tool that treats the nonmatchings tree as ground truth (ownership,
# hazard scans, retail-side instruction counts), and asm/data/ the same way:
# a plain sdata segment handed to a C unit left its .s behind, and
# tools/smalldata.py went on counting it. Wiping all of asm/ first makes the
# tree mean what it looks like it means.
#
# config/gp-symbols.txt and config/gp/<unit>.txt are derived from the sdata/
# sbss labels this writes, and
# maspsx reads it, so it is regenerated here and every object depends on it:
# round 73 went red at a merge because the file was stale after an extract and
# nothing recompiled when it was fixed by hand.
extract:
	rm -rf asm
	$(SPLAT) $(CONFIG)/splat.$(GAME_ID).$(GAME).yaml
	$(foreach o,$(OVERLAYS),$(SPLAT) $(CONFIG)/ovl/$(o).yaml && ) true
	$(PYTHON) tools/gpsyms.py

# --- housekeeping ----------------------------------------------------------
clean:
	rm -rf $(BUILD_DIR) asm $(GAME).ld
	rm -f $(CONFIG)/undefined_syms_auto.*.txt $(CONFIG)/undefined_funcs_auto.*.txt
	rm -f $(CONFIG)/ovl/undefined_syms_auto.*.txt $(CONFIG)/ovl/undefined_funcs_auto.*.txt

format:
	clang-format -i $$(find src include -name '*.c' -o -name '*.h' -o -name '*_tables.inc' -o -path 'src/ovl/*.inc' | grep -v -e include/psyq -e include/include_asm.h)
	@# clang-format never touches comment text (ReflowComments: Never), so a tab there survives it
	@! grep -nP '\t' $$(find src include -name '*.c' -o -name '*.h' -o -name '*_tables.inc' -o -path 'src/ovl/*.inc' | grep -v -e include/psyq -e include/include_asm.h) \
		|| { echo "format: tab characters above (clang-format cannot fix tabs inside comments)"; exit 1; }

expected: check
	rm -rf expected/build && mkdir -p expected && cp -r $(BUILD_DIR) expected/

progress:
	$(PYTHON) tools/progress.py

SHELL = /bin/bash -e -o pipefail
