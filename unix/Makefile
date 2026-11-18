NASM ?= nasm
NASM_FLAGS := -Werror=zeroing -w-pp-macro-redef-multi -f bin -I ../
LOADER ?= /lib64/ld-linux-x86-64.so.2
TARGET := IntelBurnTest
CANDIDATE := IntelBurnTest.build
MUTATION := IntelBurnTest.verify-test
LATE_MUTATION := IntelBurnTest.verify-late-test
EQUIVALENT := IntelBurnTest.equivalent-mov IntelBurnTest.equivalent-nop IntelBurnTest.equivalent-branch IntelBurnTest.equivalent-address
COMPARE_TEST := IntelBurnTest.compare-test
DEPFILE := .ibt-deps.mk
REFERENCE := ../reference/numerical/linux.bin
.DEFAULT_GOAL := all

ifneq ($(wildcard $(DEPFILE)),)
include $(DEPFILE)
else
.PHONY: missing-depfile
$(TARGET): missing-depfile
endif

.DELETE_ON_ERROR:
.PHONY: all check check-gui check-verifier clean

all: $(TARGET)
	@echo "Build ready: unix/$(TARGET)"

$(TARGET): elf.asm $(REFERENCE) Makefile
	$(NASM) $(NASM_FLAGS) -MD "$(DEPFILE)" -MP -MT "$(TARGET)" -o "$(CANDIDATE)" elf.asm
	@echo "Checking 53 numerical routines"
	$(LOADER) ./$(CANDIDATE) --build-test ./$(TARGET)
	@echo "Verified 53 numerical routines"

check: $(TARGET)
	@echo "Checking 53 numerical routines"
	./$(TARGET) --verify-kernels
	@echo "Verified 53 numerical routines"
	./$(TARGET) --selftest

check-gui: $(TARGET)
	./$(TARGET) --gui-test

check-verifier: $(TARGET)
	$(NASM) $(NASM_FLAGS) -DKERNEL_TEST_ELF=1 -o "$(COMPARE_TEST)" ../native/kernel_compare_test.asm
	./$(TARGET) --prepare-test "$(COMPARE_TEST)"
	$(LOADER) ./$(COMPARE_TEST)
	./$(TARGET) --verify-kernels
	$(NASM) $(NASM_FLAGS) -DKERNEL_EQUIVALENT_ELF=1 -DKERNEL_EQUIVALENT_MOV=1 -o IntelBurnTest.equivalent-mov ../native/kernel_equivalence_test.asm
	./$(TARGET) --prepare-test IntelBurnTest.equivalent-mov
	$(LOADER) ./IntelBurnTest.equivalent-mov --verify-kernels
	$(NASM) $(NASM_FLAGS) -DKERNEL_EQUIVALENT_ELF=1 -DKERNEL_EQUIVALENT_NOP=1 -o IntelBurnTest.equivalent-nop ../native/kernel_equivalence_test.asm
	./$(TARGET) --prepare-test IntelBurnTest.equivalent-nop
	$(LOADER) ./IntelBurnTest.equivalent-nop --verify-kernels
	$(NASM) $(NASM_FLAGS) -DKERNEL_EQUIVALENT_ELF=1 -DKERNEL_EQUIVALENT_BRANCH=1 -o IntelBurnTest.equivalent-branch ../native/kernel_equivalence_test.asm
	./$(TARGET) --prepare-test IntelBurnTest.equivalent-branch
	$(LOADER) ./IntelBurnTest.equivalent-branch --verify-kernels
	$(NASM) $(NASM_FLAGS) -DKERNEL_EQUIVALENT_ELF=1 -DKERNEL_EQUIVALENT_ADDRESS=1 -o IntelBurnTest.equivalent-address ../native/kernel_equivalence_test.asm
	./$(TARGET) --prepare-test IntelBurnTest.equivalent-address
	$(LOADER) ./IntelBurnTest.equivalent-address --verify-kernels
	@echo "Verifier accepted the four equivalent encodings and address variants"
	$(NASM) $(NASM_FLAGS) -DKERNEL_MUTATION_ELF=1 -o "$(MUTATION)" ../native/kernel_verification_mutation.asm
	./$(TARGET) --prepare-test "$(MUTATION)"
	@$(LOADER) ./$(MUTATION) --verify-kernels; status=$$?; if [ $$status -eq 1 ]; then echo "Verifier rejected the modified numerical routines"; else echo "ERROR: verifier did not reject the mutation"; exit 1; fi
	$(NASM) $(NASM_FLAGS) -DKERNEL_MUTATION_ELF=1 -o "$(LATE_MUTATION)" ../native/kernel_late_mutation.asm
	./$(TARGET) --prepare-test "$(LATE_MUTATION)"
	@$(LOADER) ./$(LATE_MUTATION) --verify-kernels; status=$$?; if [ $$status -eq 1 ]; then echo "Verifier rejected the modified final helper"; else echo "ERROR: verifier did not reject the final helper mutation"; exit 1; fi

clean:
	rm -f $(TARGET) $(CANDIDATE) $(MUTATION) $(LATE_MUTATION) $(EQUIVALENT) $(COMPARE_TEST) $(DEPFILE)
