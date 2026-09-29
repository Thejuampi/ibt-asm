.DEFAULT_GOAL := all

ifeq ($(OS),Windows_NT)
SHELL := cmd.exe
.SHELLFLAGS := /C
HOST_PLATFORM := windows
NASM ?= nasm.exe
PYTHON ?= python
UPX ?= upx.exe
else
SHELL := /bin/sh
HOST_PLATFORM := linux
NASM ?= nasm
PYTHON ?= python3
UPX ?= upx
endif

RC = rc.exe
LINKER ?= link.exe
UPX_EXE ?= $(UPX)
CC ?= cc
STRIP ?= strip
NM ?= nm
SIZE ?= size

BIN_DIR := bin
DIST_DIR := dist
TARGET := $(BIN_DIR)/IntelBurnTest.exe
OBJECT := $(BIN_DIR)/ibt.obj
RESOURCE := $(BIN_DIR)/ibt.res
LISTING := $(BIN_DIR)/ibt.lst

SELFTEST_OBJECT := $(BIN_DIR)/ibt-selftest.obj
SELFTEST_EXE := $(BIN_DIR)/IntelBurnTest-selftest.exe
PACKED_RESOURCE := $(BIN_DIR)/ibt-packed.res
PACK_SOURCE := $(BIN_DIR)/IntelBurnTest-packsource.exe
PACKED_EXE := $(DIST_DIR)/IntelBurnTest.exe

LINUX_TARGET := $(BIN_DIR)/IntelBurnTest-linux-x64
LINUX_UNPACKED := $(BIN_DIR)/IntelBurnTest-linux-x64.unpacked
LINUX_OBJECT := $(BIN_DIR)/ibt_unix.o
LINUX_MAX_SIZE := 23552
# Keep the calculation span below half of a typical 32 KiB L1 instruction cache.
LINUX_CORE_BUDGET := 16384

ASM_SOURCES := ibt.asm ibt_macros.inc ibt_ui.inc ibt_theme.inc ibt_lpk.inc bench_lib.inc
RESOURCE_SOURCES := ibt.rc app.manifest res/app.ico res/coffee4.bmp \
                    res/flame4-rle.bmp res/flame4.bmp
LIBS := kernel32.lib user32.lib gdi32.lib comctl32.lib \
        synchronization.lib shell32.lib uxtheme.lib dwmapi.lib
LINK_FLAGS := /nologo /SUBSYSTEM:WINDOWS /NODEFAULTLIB /ENTRY:start \
              /MACHINE:X64 /DYNAMICBASE /NXCOMPAT /LARGEADDRESSAWARE

LINUX_SOURCES := unix/ibt_unix.asm unix/ibt_state.inc unix/ibt_app.inc \
                 unix/ibt_window.inc unix/ibt_events.inc unix/ibt_layout.inc \
                 unix/ibt_draw_primitives.inc unix/ibt_bridge.inc unix/ibt_draw.inc \
                 ibt_lpk.inc bench_lib.inc res/flame4.bmp res/coffee4.bmp
BUILD_OUTPUTS := $(TARGET) $(OBJECT) $(RESOURCE) $(LISTING) \
                 $(SELFTEST_OBJECT) $(SELFTEST_EXE) $(BIN_DIR)/ibt-selftest.lst \
                 $(PACKED_RESOURCE) $(PACK_SOURCE) $(PACKED_EXE) \
                 $(LINUX_TARGET) $(LINUX_UNPACKED) $(LINUX_OBJECT)

.PHONY: all windows linux clean selftest packed assets visual size \
        windows-selftest windows-packed windows-size \
        linux-selftest linux-packed linux-size

all: $(HOST_PLATFORM)

windows: $(TARGET)

linux: linux-selftest

selftest: $(HOST_PLATFORM)-selftest

packed: $(HOST_PLATFORM)-packed

size: $(HOST_PLATFORM)-size

$(BIN_DIR):
ifeq ($(OS),Windows_NT)
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"
else
	mkdir -p "$(BIN_DIR)"
endif

$(DIST_DIR):
ifeq ($(OS),Windows_NT)
	@if not exist "$(DIST_DIR)" mkdir "$(DIST_DIR)"
else
	mkdir -p "$(DIST_DIR)"
endif

$(OBJECT): $(ASM_SOURCES) tools/asmcheck.py | $(BIN_DIR)
	$(NASM) -f win64 -l "$(LISTING)" -o "$@" ibt.asm
	$(PYTHON) tools/asmcheck.py "$(LISTING)"

$(RESOURCE): $(RESOURCE_SOURCES) | $(BIN_DIR)
	$(RC) /nologo /fo "$@" ibt.rc

$(TARGET): $(OBJECT) $(RESOURCE) tools/benchcheck.py
	$(LINKER) $(LINK_FLAGS) /OUT:"$@" $(OBJECT) $(RESOURCE) $(LIBS)
	$(PYTHON) tools/benchcheck.py "$@"

$(SELFTEST_OBJECT): $(ASM_SOURCES) | $(BIN_DIR)
	$(NASM) -DSELFTEST=1 -f win64 -l "$(BIN_DIR)/ibt-selftest.lst" -o "$@" ibt.asm

$(SELFTEST_EXE): $(SELFTEST_OBJECT) $(RESOURCE)
	$(LINKER) $(LINK_FLAGS) /OUT:"$@" $(SELFTEST_OBJECT) $(RESOURCE) $(LIBS)

windows-selftest: $(SELFTEST_EXE)
	"$(SELFTEST_EXE)" -t

$(PACKED_RESOURCE): $(RESOURCE_SOURCES) | $(BIN_DIR)
	$(RC) /nologo /d PACKED_RESOURCES /fo "$@" ibt.rc

$(PACK_SOURCE): $(OBJECT) $(PACKED_RESOURCE) tools/benchcheck.py
	$(LINKER) $(LINK_FLAGS) /OUT:"$@" $(OBJECT) $(PACKED_RESOURCE) $(LIBS)
	$(PYTHON) tools/benchcheck.py "$@"

$(PACKED_EXE): $(PACK_SOURCE) | $(DIST_DIR)
	copy /Y "$(subst /,\,$<)" "$(subst /,\,$@)" >nul
	$(UPX_EXE) --ultra-brute --lzma "$@"

windows-packed: $(PACKED_EXE)

$(LINUX_OBJECT): $(LINUX_SOURCES) | $(BIN_DIR)
	cd unix && $(NASM) -f elf64 -O9 -o "$(abspath $@)" ibt_unix.asm

$(LINUX_UNPACKED): $(LINUX_OBJECT)
	$(CC) -nostartfiles -no-pie -Wl,-z,noseparate-code -Wl,--build-id=none \
		-Wl,--gc-sections -o "$@" "$<" -lXft -lX11 -lpthread -lc
	$(STRIP) -s -R .comment -R .note.gnu.property "$@"

$(LINUX_TARGET): $(LINUX_UNPACKED)
	cp "$<" "$@"
	$(UPX) --best --lzma -q "$@"

linux-selftest: $(LINUX_TARGET)
	"./$(LINUX_TARGET)" --selftest
	@test $$(wc -c < "$(LINUX_TARGET)") -le $(LINUX_MAX_SIZE) || \
		(echo "size limit exceeded: $$(wc -c < '$(LINUX_TARGET)') > $(LINUX_MAX_SIZE) bytes"; exit 1)
	@core_start=$$($(NM) -n "$(LINUX_OBJECT)" | awk '$$3=="lp_isa_probe" {print "0x" $$1; exit}'); \
		text_size=$$($(SIZE) -A "$(LINUX_OBJECT)" | awk '$$1==".text" {print $$2; exit}'); \
		{ test -n "$$core_start" && test -n "$$text_size"; } || \
			{ echo "cannot measure numerical code span"; exit 1; }; \
		core_bytes=$$((text_size - core_start)); \
		echo "numerical code span: $$core_bytes / $(LINUX_CORE_BUDGET) bytes"; \
		test $$core_bytes -le $(LINUX_CORE_BUDGET) || \
			(echo "numerical code exceeds L1i budget"; exit 1)

linux-packed: linux

linux-size: $(LINUX_TARGET)
	@wc -c "$(LINUX_TARGET)"

assets:
	$(PYTHON) tools/make_assets.py

visual: $(TARGET)
	$(PYTHON) tools/visual_qa.py

windows-size: $(TARGET)
	$(PYTHON) tools/size_report.py "$(LISTING)"

clean:
ifeq ($(OS),Windows_NT)
	$(PYTHON) -c "from pathlib import Path; import sys; [Path(p).unlink(missing_ok=True) for p in sys.argv[1:]]" $(foreach file,$(BUILD_OUTPUTS),"$(file)")
else
	rm -f $(foreach file,$(BUILD_OUTPUTS),"$(file)")
endif
