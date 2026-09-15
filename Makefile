SHELL := cmd.exe
.SHELLFLAGS := /C

NASM := nasm.exe
RC := rc.exe
LINKER := link.exe
PYTHON := python
UPX_EXE := upx.exe

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

ASM_SOURCES := ibt.asm ibt_ui.inc ibt_theme.inc ibt_lpk.inc bench_lib.inc
RESOURCE_SOURCES := ibt.rc app.manifest res/app.ico res/coffee4.bmp \
                    res/flame4-rle.bmp res/flame4.bmp
LIBS := kernel32.lib user32.lib gdi32.lib comctl32.lib \
        synchronization.lib shell32.lib uxtheme.lib dwmapi.lib
LINK_FLAGS := /nologo /SUBSYSTEM:WINDOWS /NODEFAULTLIB /ENTRY:start \
              /MACHINE:X64 /DYNAMICBASE /NXCOMPAT /LARGEADDRESSAWARE

.PHONY: all clean selftest packed assets visual size

all: $(TARGET)

$(BIN_DIR):
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"

$(DIST_DIR):
	@if not exist "$(DIST_DIR)" mkdir "$(DIST_DIR)"

$(OBJECT): $(ASM_SOURCES) | $(BIN_DIR)
	$(NASM) -f win64 -l "$(LISTING)" -o "$@" ibt.asm
	$(PYTHON) tools/asmcheck.py

$(RESOURCE): $(RESOURCE_SOURCES) | $(BIN_DIR)
	$(RC) /nologo /fo "$@" ibt.rc

$(TARGET): $(OBJECT) $(RESOURCE)
	$(LINKER) $(LINK_FLAGS) /OUT:"$@" $(OBJECT) $(RESOURCE) $(LIBS)

$(SELFTEST_OBJECT): $(ASM_SOURCES) | $(BIN_DIR)
	$(NASM) -DSELFTEST=1 -f win64 -l "$(BIN_DIR)/ibt-selftest.lst" -o "$@" ibt.asm

$(SELFTEST_EXE): $(SELFTEST_OBJECT) $(RESOURCE)
	$(LINKER) $(LINK_FLAGS) /OUT:"$@" $(SELFTEST_OBJECT) $(RESOURCE) $(LIBS)

selftest: $(SELFTEST_EXE)
	"$(SELFTEST_EXE)" -t

$(PACKED_RESOURCE): $(RESOURCE_SOURCES) | $(BIN_DIR)
	$(RC) /nologo /d PACKED_RESOURCES /fo "$@" ibt.rc

$(PACK_SOURCE): $(OBJECT) $(PACKED_RESOURCE)
	$(LINKER) $(LINK_FLAGS) /OUT:"$@" $(OBJECT) $(PACKED_RESOURCE) $(LIBS)

packed: $(PACK_SOURCE) | $(DIST_DIR)
	copy /Y "bin\IntelBurnTest-packsource.exe" "dist\IntelBurnTest.exe" >nul
	$(UPX_EXE) --ultra-brute --lzma "$(PACKED_EXE)"

assets:
	$(PYTHON) tools/make_assets.py

visual: $(TARGET)
	$(PYTHON) tools/visual_qa.py

size: $(TARGET)
	$(PYTHON) tools/size_report.py

clean:
	@if exist "$(BIN_DIR)" rmdir /S /Q "$(BIN_DIR)"
	@if exist "$(DIST_DIR)" rmdir /S /Q "$(DIST_DIR)"
