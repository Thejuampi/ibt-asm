ifeq ($(OS),Windows_NT)

SHELL := cmd.exe
.SHELLFLAGS := /C

NASM := nasm.exe
NASM_FLAGS := -Werror=zeroing -w-pp-macro-redef-multi -f bin -DIBT_COMPACT_PIPELINE=1

BIN_DIR := bin
TARGET := $(BIN_DIR)/IntelBurnTest.exe
CANDIDATE := $(BIN_DIR)/IntelBurnTest.build.exe
SELFTEST_APP := $(BIN_DIR)/IntelBurnTest.selftest.exe
DEPFILE := $(BIN_DIR)/ibt-deps.mk
COMPARE_TEST := $(BIN_DIR)/ibt-kernel-compare-test.exe
COMPARE_DEPFILE := $(BIN_DIR)/ibt-kernel-deps.mk
REFERENCE := reference/numerical/windows.bin
APP_FILE_LIMIT := 54784
CLEAN_FILES := $(TARGET) $(CANDIDATE) $(SELFTEST_APP) $(DEPFILE) $(COMPARE_TEST) $(COMPARE_DEPFILE) \
	$(BIN_DIR)/lpk_test.txt $(BIN_DIR)/lpk_prof.txt \
	$(addprefix $(BIN_DIR)/ibt-kernel-,equivalent-mov.exe \
	equivalent-nop.exe equivalent-branch.exe equivalent-address.exe mutation.exe late-mutation.exe)
.DEFAULT_GOAL := all

ifneq ($(wildcard $(DEPFILE)),)
include $(DEPFILE)
else
.PHONY: missing-depfile
$(TARGET): missing-depfile
endif
ifneq ($(wildcard $(COMPARE_DEPFILE)),)
include $(COMPARE_DEPFILE)
else
.PHONY: missing-compare-depfile
$(COMPARE_TEST): missing-compare-depfile
endif

run_program = cmd /v:on /c "start "" /wait "$(1)" $(2) & exit /b !ERRORLEVEL!"
run_checker = $(call run_program,$(COMPARE_TEST),$(1))
require_checker_status = cmd /v:on /c "start "" /wait "$(COMPARE_TEST)" $(1) & if !ERRORLEVEL! EQU $(2) (exit /b 0) else (exit /b 1)"
build_selftest = $(NASM) $(NASM_FLAGS) -DSELFTEST=1 -o "$(SELFTEST_APP)" native/pe.asm

.DELETE_ON_ERROR:
.PHONY: all clean check check-gui check-verifier selftest

all: $(TARGET)
	@echo Build ready: $(TARGET)

$(BIN_DIR):
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"

$(COMPARE_TEST): native/kernel_compare_test.asm Makefile | $(BIN_DIR)
	$(NASM) $(NASM_FLAGS) -MD "$(COMPARE_DEPFILE)" -MP -MT "$(COMPARE_TEST)" -o "$@" $<

$(TARGET): native/pe.asm $(REFERENCE) Makefile $(COMPARE_TEST) | $(BIN_DIR)
	$(NASM) $(NASM_FLAGS) -DAPP_FILE_LIMIT=$(APP_FILE_LIMIT) -MD "$(DEPFILE)" -MP -MT "$(TARGET)" -o "$(CANDIDATE)" native/pe.asm
	@echo Checking 29 numerical routines
	$(call run_checker,"$(CANDIDATE)")
	@echo Verified 29 numerical routines
	$(call run_program,$(CANDIDATE),-t)
	$(build_selftest)
	$(call run_program,$(SELFTEST_APP),-t)
	move /Y "$(subst /,\,$(CANDIDATE))" "$(subst /,\,$(TARGET))" >nul

$(SELFTEST_APP): $(TARGET)
	$(build_selftest)

check selftest: $(TARGET) $(SELFTEST_APP)
	@echo Checking 29 numerical routines
	$(call run_checker,"$(TARGET)")
	@echo Verified 29 numerical routines
	$(call run_program,$(TARGET),-t)
	$(call run_program,$(SELFTEST_APP),-t)

check-gui: $(TARGET) $(SELFTEST_APP)
	$(call run_program,$(TARGET),-g)
	$(call run_program,$(SELFTEST_APP),-g)

check-verifier: $(TARGET)
	$(call run_checker)
	$(call run_checker,"$(TARGET)")
	$(call require_checker_status,"$(COMPARE_TEST)",2)
	$(NASM) $(NASM_FLAGS) -DKERNEL_EQUIVALENT_MOV=1 -o "$(BIN_DIR)/ibt-kernel-equivalent-mov.exe" native/kernel_equivalence_test.asm
	$(call run_checker,"$(BIN_DIR)/ibt-kernel-equivalent-mov.exe")
	$(NASM) $(NASM_FLAGS) -DKERNEL_EQUIVALENT_NOP=1 -o "$(BIN_DIR)/ibt-kernel-equivalent-nop.exe" native/kernel_equivalence_test.asm
	$(call run_checker,"$(BIN_DIR)/ibt-kernel-equivalent-nop.exe")
	$(NASM) $(NASM_FLAGS) -DKERNEL_EQUIVALENT_BRANCH=1 -o "$(BIN_DIR)/ibt-kernel-equivalent-branch.exe" native/kernel_equivalence_test.asm
	$(call run_checker,"$(BIN_DIR)/ibt-kernel-equivalent-branch.exe")
	$(NASM) $(NASM_FLAGS) -DKERNEL_EQUIVALENT_ADDRESS=1 -o "$(BIN_DIR)/ibt-kernel-equivalent-address.exe" native/kernel_equivalence_test.asm
	$(call run_checker,"$(BIN_DIR)/ibt-kernel-equivalent-address.exe")
	@echo Verifier accepted the four equivalent encodings and address variants
	$(NASM) $(NASM_FLAGS) -o "$(BIN_DIR)/ibt-kernel-mutation.exe" native/kernel_verification_mutation.asm
	$(call require_checker_status,"$(BIN_DIR)/ibt-kernel-mutation.exe",1)
	@echo Verifier rejected the modified numerical routines
	$(NASM) $(NASM_FLAGS) -o "$(BIN_DIR)/ibt-kernel-late-mutation.exe" native/kernel_late_mutation.asm
	$(call require_checker_status,"$(BIN_DIR)/ibt-kernel-late-mutation.exe",1)
	@echo Verifier rejected the modified final helper

clean:
	@$(if $(wildcard $(CLEAN_FILES)),del /Q $(foreach file,$(wildcard $(CLEAN_FILES)),"$(subst /,\,$(file))"))

else

.PHONY: all clean check check-gui check-verifier

all clean check check-gui check-verifier:
	$(MAKE) -C unix $@

endif
