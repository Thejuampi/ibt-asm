NASM ?= nasm
CC ?= cc
STRIP ?= strip
UPX ?= upx
NM ?= nm
SIZE ?= size

TARGET := IntelBurnTest
UNPACKED := IntelBurnTest.unpacked
OBJECT := ibt_unix.o
MAX_SIZE := 23552
# Keep the calculation span below half of a typical 32 KiB L1 instruction cache.
CORE_BUDGET := 16384

.PHONY: all clean check size

all: $(TARGET) check

$(OBJECT): ibt_unix.asm ibt_draw.inc ../ibt_lpk.inc ../bench_lib.inc \
	../res/flame4.bmp ../res/coffee4.bmp
	$(NASM) -f elf64 -O9 -o $@ $<

$(UNPACKED): $(OBJECT)
	$(CC) -nostartfiles -no-pie -Wl,-z,noseparate-code -Wl,--build-id=none \
		-Wl,--gc-sections -o $@ $< -lXft -lX11 -lpthread -lc
	$(STRIP) -s -R .comment -R .note.gnu.property $@

$(TARGET): $(UNPACKED)
	cp $< $@
	$(UPX) --best --lzma -q $@

check: $(TARGET)
	./$(TARGET) --selftest
	@test $$(wc -c < $(TARGET)) -le $(MAX_SIZE) || \
		(echo "size limit exceeded: $$(wc -c < $(TARGET)) > $(MAX_SIZE) bytes"; exit 1)
	@core_start=$$($(NM) -n $(OBJECT) | awk '$$3=="lp_isa_probe" {print "0x" $$1; exit}'); \
		text_size=$$($(SIZE) -A $(OBJECT) | awk '$$1==".text" {print $$2; exit}'); \
		{ test -n "$$core_start" && test -n "$$text_size"; } || \
			{ echo "cannot measure numerical code span"; exit 1; }; \
		core_bytes=$$((text_size - core_start)); \
		echo "numerical code span: $$core_bytes / $(CORE_BUDGET) bytes"; \
		test $$core_bytes -le $(CORE_BUDGET) || \
			(echo "numerical code exceeds L1i budget"; exit 1)

size: $(TARGET)
	@wc -c $(TARGET)

clean:
	rm -f $(OBJECT) $(UNPACKED) $(TARGET) IntelBurnTest.upx results.log
