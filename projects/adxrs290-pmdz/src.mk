#See No-OS/tool/scripts/src_model.mk for variable description

SRC_DIRS += $(PROJECT)/src
SRC_DIRS += $(DRIVERS)/gyro/adxrs290 
SRC_DIRS += $(NO-OS)/iio/iio_adxrs290 

# For the moment there is support only for aducm for iio with network backend
ifeq (aducm3029,$(strip $(PLATFORM)))
ifeq '$(USE_TCP_SOCKET)' 'y'
ENABLE_IIO_NETWORK ?= y
CFLAGS += -DUSE_TCP_SOCKET
else
ENABLE_IIO_NETWORK ?= n
endif
endif

ifeq (y,$(strip $(ENABLE_IIO_NETWORK)))
DISABLE_SECURE_SOCKET ?= y
SRC_DIRS += $(NO-OS)/network
SRCS	 += $(NO-OS)/util/circular_buffer.c
SRCS	 += $(PLATFORM_DRIVERS)/delay.c
SRCS	 += $(PLATFORM_DRIVERS)/timer.c
INCS	 += $(INCLUDE)/delay.h
INCS	 += $(INCLUDE)/timer.h
INCS	 += $(INCLUDE)/circular_buffer.h
INCS	 += $(PLATFORM_DRIVERS)/timer_extra.h
endif


LIBRARIES += iio

SRCS += $(PLATFORM_DRIVERS)/uart.c					\
	$(PLATFORM_DRIVERS)/irq.c					\
	$(PLATFORM_DRIVERS)/gpio.c					\
	$(PLATFORM_DRIVERS)/spi.c					\
	$(NO-OS)/util/xml.c						\
	$(NO-OS)/util/list.c						\
	$(NO-OS)/util/fifo.c						\
	$(NO-OS)/util/util.c						\

INCS += $(INCLUDE)/xml.h						\
	$(INCLUDE)/fifo.h						\
	$(INCLUDE)/irq.h						\
	$(INCLUDE)/uart.h						\
	$(INCLUDE)/list.h						\
	$(INCLUDE)/util.h						\
	$(INCLUDE)/error.h						\
	$(INCLUDE)/gpio.h						\
	$(INCLUDE)/rtc.h						\
	$(INCLUDE)/spi.h						\
	$(PLATFORM_DRIVERS)/spi_extra.h					\
	$(PLATFORM_DRIVERS)/irq_extra.h					\
	$(PLATFORM_DRIVERS)/rtc_extra.h					\
	$(PLATFORM_DRIVERS)/uart_extra.h				
