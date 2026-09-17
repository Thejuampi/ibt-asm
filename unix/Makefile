NASM ?= nasm
CC ?= cc
STRIP ?= strip
UPX ?= upx

TARGET := IntelBurnTest
UNPACKED := IntelBurnTest.unpacked
OBJECT := ibt_unix.o
MAX_SIZE := 23552

.PHONY: all clean check size

all: $(TARGET) check

$(OBJECT): ibt_unix.asm ../ibt_lpk.inc ../bench_lib.inc \
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

size: $(TARGET)
	@wc -c $(TARGET)

clean:
	rm -f $(OBJECT) $(UNPACKED) $(TARGET) IntelBurnTest.upx results.log
