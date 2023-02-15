PROJECT_DIR := $(shell pwd)

PROM = factory

all: $(PROM)
.PHONY : all

OBJ = \
	aes.o \
	hmac_sha2.o \
	sha2.o \
	rpmb.o \
	common.o \
	config_parser.o \
	mmcwrapper.o \
	network.o \
	packet.o \
	test_4g.o \
	test_can.o \
	test_flash.o \
	test_gps.o \
	test_net.o \
	test_rtc.o \
	test_stb_mode.o \
	test_sn.o \
	test_uart.o \
	test_usb.o \
	test_version.o \
	udp.o \
	main.o \

CFLAGS += \
	-I$(PROJECT_DIR) \
	-I../vdk/include \
	-I$(SDKTARGETSYSROOT)/usr/include/libxml2

CFLAGS += \
	-L../vdk

CFLAGS += \
	-lc \
	-lz \
	-lpng \
	-ldrm \
	-lpthread \
	-lcrypto \
	-lvdk \
	-lxml2\
	-lstdc++
	#-lcurl \

$(PROM): $(OBJ)
	$(CC) -o $(PROM) $(OBJ) $(CFLAGS)

%.o: %.cpp
	$(CC) -c $< -o $@ $(CFLAGS)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	rm -rf $(OBJ) $(PROM) $(UPDATE_ENGINE)

install:
	mkdir -p $(DESTDIR)/res/images $(DESTDIR)/usr/bin
	cp $(PROJECT_DIR)/res/images/* $(DESTDIR)/res/images/
