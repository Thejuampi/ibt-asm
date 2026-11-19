NASM ?= nasm
NASM_FLAGS := -Werror=zeroing -w-pp-macro-redef-multi -f bin -I ../ -DIBT_COMPACT_PIPELINE=1
LOADER ?= /lib64/ld-linux-x86-64.so.2
TARGET := IntelBurnTest
CANDIDATE := IntelBurnTest.build
MUTATION := IntelBurnTest.verify-test
LATE_MUTATION := IntelBurnTest.verify-late-test
EQUIVALENT := IntelBurnTest.equivalent-mov IntelBurnTest.equivalent-nop IntelBurnTest.equivalent-branch IntelBurnTest.equivalent-address
COMPARE_TEST := IntelBurnTest.compare-test
DEPFILE := .ibt-deps.mk
COMPARE_DEPFILE := .ibt-compare-deps.mk
REFERENCE := ../reference/numerical/linux.bin
APP_FILE_LIMIT := 43344
SIZE_CHECK_FILE ?= $(TARGET)
SIZE_CHECK_OUTPUT := .ibt-size-check
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

.DELETE_ON_ERROR:
.PHONY: all check check-gui check-size check-verifier clean

all: $(TARGET)
	@echo "Build ready: unix/$(TARGET)"

$(COMPARE_TEST): ../native/kernel_compare_test.asm Makefile
	$(NASM) $(NASM_FLAGS) -DKERNEL_TEST_ELF=1 -MD "$(COMPARE_DEPFILE)" -MP -MT "$(COMPARE_TEST)" -o "$@" $<

$(TARGET): elf.asm $(REFERENCE) Makefile $(COMPARE_TEST)
	$(NASM) $(NASM_FLAGS) -DAPP_FILE_LIMIT=$(APP_FILE_LIMIT) -MD "$(DEPFILE)" -MP -MT "$(TARGET)" -o "$(CANDIDATE)" elf.asm
	$(LOADER) ./$(CANDIDATE) --prepare-test "$(COMPARE_TEST)"
	@echo "Checking 29 numerical routines"
	./$(COMPARE_TEST) ./$(CANDIDATE)
	@echo "Verified 29 numerical routines"
	$(LOADER) ./$(CANDIDATE) --build-test ./$(TARGET)

check: check-size
	@echo "Checking 29 numerical routines"
	./$(COMPARE_TEST) ./$(TARGET)
	@echo "Verified 29 numerical routines"
	./$(TARGET) --selftest

check-size: $(TARGET)
	@echo "Checking $(SIZE_CHECK_FILE) against the $(APP_FILE_LIMIT)-byte desktop limit"
	$(NASM) -f bin "-DSIZE_CHECK_FILE=$(SIZE_CHECK_FILE)" -DAPP_FILE_LIMIT=$(APP_FILE_LIMIT) -o "$(SIZE_CHECK_OUTPUT)" ../native/desktop_size_check.asm

check-gui: $(TARGET)
	./$(TARGET) --gui-test

check-verifier: $(TARGET)
	./$(COMPARE_TEST)
	./$(COMPARE_TEST) ./$(TARGET)
	@./$(COMPARE_TEST) ./$(COMPARE_TEST); status=$$?; if [ $$status -ne 2 ]; then echo "ERROR: verifier did not distinguish invalid input"; exit 1; fi
	$(NASM) $(NASM_FLAGS) -DKERNEL_EQUIVALENT_ELF=1 -DKERNEL_EQUIVALENT_MOV=1 -o IntelBurnTest.equivalent-mov ../native/kernel_equivalence_test.asm
	./$(COMPARE_TEST) ./IntelBurnTest.equivalent-mov
	$(NASM) $(NASM_FLAGS) -DKERNEL_EQUIVALENT_ELF=1 -DKERNEL_EQUIVALENT_NOP=1 -o IntelBurnTest.equivalent-nop ../native/kernel_equivalence_test.asm
	./$(COMPARE_TEST) ./IntelBurnTest.equivalent-nop
	$(NASM) $(NASM_FLAGS) -DKERNEL_EQUIVALENT_ELF=1 -DKERNEL_EQUIVALENT_BRANCH=1 -o IntelBurnTest.equivalent-branch ../native/kernel_equivalence_test.asm
	./$(COMPARE_TEST) ./IntelBurnTest.equivalent-branch
	$(NASM) $(NASM_FLAGS) -DKERNEL_EQUIVALENT_ELF=1 -DKERNEL_EQUIVALENT_ADDRESS=1 -o IntelBurnTest.equivalent-address ../native/kernel_equivalence_test.asm
	./$(COMPARE_TEST) ./IntelBurnTest.equivalent-address
	@echo "Verifier accepted the four equivalent encodings and address variants"
	$(NASM) $(NASM_FLAGS) -DKERNEL_MUTATION_ELF=1 -o "$(MUTATION)" ../native/kernel_verification_mutation.asm
	@./$(COMPARE_TEST) ./$(MUTATION); status=$$?; if [ $$status -eq 1 ]; then echo "Verifier rejected the modified numerical routines"; else echo "ERROR: verifier did not reject the mutation"; exit 1; fi
	$(NASM) $(NASM_FLAGS) -DKERNEL_MUTATION_ELF=1 -o "$(LATE_MUTATION)" ../native/kernel_late_mutation.asm
	@./$(COMPARE_TEST) ./$(LATE_MUTATION); status=$$?; if [ $$status -eq 1 ]; then echo "Verifier rejected the modified final helper"; else echo "ERROR: verifier did not reject the final helper mutation"; exit 1; fi

clean:
	rm -f $(TARGET) $(CANDIDATE) $(MUTATION) $(LATE_MUTATION) $(EQUIVALENT) $(COMPARE_TEST) $(DEPFILE) $(COMPARE_DEPFILE) $(SIZE_CHECK_OUTPUT)
