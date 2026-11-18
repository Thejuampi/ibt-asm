ifeq ($(OS),Windows_NT)

SHELL := cmd.exe
.SHELLFLAGS := /C

NASM := nasm.exe
NASM_FLAGS := -Werror=zeroing -w-pp-macro-redef-multi -f bin -DSELFTEST=1

BIN_DIR := bin
TARGET := $(BIN_DIR)/IntelBurnTest.exe
CANDIDATE := $(BIN_DIR)/IntelBurnTest.build.exe
DEPFILE := $(BIN_DIR)/ibt-deps.mk
REFERENCE := reference/numerical/windows.bin
CLEAN_FILES := $(TARGET) $(CANDIDATE) $(DEPFILE) \
	$(BIN_DIR)/lpk_test.txt $(BIN_DIR)/lpk_prof.txt \
	$(addprefix $(BIN_DIR)/ibt-kernel-,compare-test.exe equivalent-mov.exe \
	equivalent-nop.exe equivalent-branch.exe equivalent-address.exe mutation.exe late-mutation.exe)
.DEFAULT_GOAL := all

ifneq ($(wildcard $(DEPFILE)),)
include $(DEPFILE)
else
.PHONY: missing-depfile
$(TARGET): missing-depfile
endif

.DELETE_ON_ERROR:
.PHONY: all clean check check-gui check-verifier selftest

all: $(TARGET)
	@echo Build ready: $(TARGET)

$(BIN_DIR):
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"

$(TARGET): native/pe.asm $(REFERENCE) Makefile | $(BIN_DIR)
	$(NASM) $(NASM_FLAGS) -MD "$(DEPFILE)" -MP -MT "$(TARGET)" -o "$(CANDIDATE)" native/pe.asm
	@echo Checking 53 numerical routines
	cmd /v:on /c "start "" /wait "$(CANDIDATE)" -v & exit /b !ERRORLEVEL!"
	@echo Verified 53 numerical routines
	cmd /v:on /c "start "" /wait "$(CANDIDATE)" -t & exit /b !ERRORLEVEL!"
	move /Y "$(subst /,\,$(CANDIDATE))" "$(subst /,\,$(TARGET))" >nul

check selftest: $(TARGET)
	@echo Checking 53 numerical routines
	cmd /v:on /c "start "" /wait "$(TARGET)" -v & exit /b !ERRORLEVEL!"
	@echo Verified 53 numerical routines
	cmd /v:on /c "start "" /wait "$(TARGET)" -t & exit /b !ERRORLEVEL!"

check-gui: $(TARGET)
	cmd /v:on /c "start "" /wait "$(TARGET)" -g & exit /b !ERRORLEVEL!"

check-verifier: $(TARGET)
	$(NASM) -f bin -o "$(BIN_DIR)/ibt-kernel-compare-test.exe" native/kernel_compare_test.asm
	cmd /v:on /c "start "" /wait "$(BIN_DIR)/ibt-kernel-compare-test.exe" & exit /b !ERRORLEVEL!"
	cmd /v:on /c "start "" /wait "$(TARGET)" -v & exit /b !ERRORLEVEL!"
	$(NASM) $(NASM_FLAGS) -DKERNEL_EQUIVALENT_MOV=1 -o "$(BIN_DIR)/ibt-kernel-equivalent-mov.exe" native/kernel_equivalence_test.asm
	cmd /v:on /c "start "" /wait "$(BIN_DIR)/ibt-kernel-equivalent-mov.exe" -v & exit /b !ERRORLEVEL!"
	$(NASM) $(NASM_FLAGS) -DKERNEL_EQUIVALENT_NOP=1 -o "$(BIN_DIR)/ibt-kernel-equivalent-nop.exe" native/kernel_equivalence_test.asm
	cmd /v:on /c "start "" /wait "$(BIN_DIR)/ibt-kernel-equivalent-nop.exe" -v & exit /b !ERRORLEVEL!"
	$(NASM) $(NASM_FLAGS) -DKERNEL_EQUIVALENT_BRANCH=1 -o "$(BIN_DIR)/ibt-kernel-equivalent-branch.exe" native/kernel_equivalence_test.asm
	cmd /v:on /c "start "" /wait "$(BIN_DIR)/ibt-kernel-equivalent-branch.exe" -v & exit /b !ERRORLEVEL!"
	$(NASM) $(NASM_FLAGS) -DKERNEL_EQUIVALENT_ADDRESS=1 -o "$(BIN_DIR)/ibt-kernel-equivalent-address.exe" native/kernel_equivalence_test.asm
	cmd /v:on /c "start "" /wait "$(BIN_DIR)/ibt-kernel-equivalent-address.exe" -v & exit /b !ERRORLEVEL!"
	@echo Verifier accepted the four equivalent encodings and address variants
	$(NASM) $(NASM_FLAGS) -o "$(BIN_DIR)/ibt-kernel-mutation.exe" native/kernel_verification_mutation.asm
	cmd /v:on /c "start "" /wait "$(BIN_DIR)/ibt-kernel-mutation.exe" -v & if !ERRORLEVEL! EQU 1 (exit /b 0) else (exit /b 1)"
	@echo Verifier rejected the modified numerical routines
	$(NASM) $(NASM_FLAGS) -o "$(BIN_DIR)/ibt-kernel-late-mutation.exe" native/kernel_late_mutation.asm
	cmd /v:on /c "start "" /wait "$(BIN_DIR)/ibt-kernel-late-mutation.exe" -v & if !ERRORLEVEL! EQU 1 (exit /b 0) else (exit /b 1)"
	@echo Verifier rejected the modified final helper

clean:
	@$(if $(wildcard $(CLEAN_FILES)),del /Q $(foreach file,$(wildcard $(CLEAN_FILES)),"$(subst /,\,$(file))"))

else

.PHONY: all clean check check-gui check-verifier

all clean check check-gui check-verifier:
	$(MAKE) -C unix $@

endif
