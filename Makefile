# libs Makefile

include Makefile.common

LIBS+=lib$(series).a
CFLAGSlib+=-c

all: libs

libs: $(LIBS)

$(LIBS):
	@echo -n "Building $@ ..."
	@cd $(STMLIB)/CMSIS/Device/ST/$(SERIES)/Source/Templates && \
			$(CC) $(CFLAGSlib) \
				system_$(series).c
	@cd $(STMLIB)/$(SERIES)_StdPeriph_Driver/src && \
			$(CC) $(CFLAGSlib) \
				-D"assert_param(expr)=((void)0)" \
				-I../../CMSIS/Include \
				-I../../CMSIS/Device/ST/$(SERIES)/Include \
				-I../inc \
				*.c
	@cd $(LIBDIR)/STM32F37x_I2C_CPAL_Driver/src && \
			$(CC) $(CFLAGSlib) \
				-D"assert_param(expr)=((void)0)" \
				-I../../CMSIS/Include \
				-I../../CMSIS/Device/ST/$(SERIES)/Include \
				-I../inc \
				*.c
#	@cd $(STMLIB)/STM32_USB-FS-Device_Driver/src && \
#			$(CC) $(CFLAGSlib) \
# 				-D"assert_param(expr)=((void)0)" \
# 				-I../../CMSIS/Include \
# 				-I../../CMSIS/Device/ST/$(SERIES)/Include \
# 				-I../inc \
# 				*.c
	@$(AR) cr $(LIBDIR)/$@ \
			$(STMLIB)/CMSIS/Device/ST/$(SERIES)/Source/Templates/system_$(series).o \
			$(STMLIB)/$(SERIES)_StdPeriph_Driver/src/*.o \
			$(LIBDIR)/STM32F37x_I2C_CPAL_Driver/src/*.o \
# 			$(STMLIB)/STM32_USB-FS-Device_Driver/src/*.o
	@echo "done."

.PHONY: libs clean tshow

clean:
	rm -f $(STMLIB)/CMSIS/Device/ST/$(SERIES)/Source/Templates/system_$(series).o
	rm -f $(STMLIB)/$(SERIES)_StdPeriph_Driver/src/*.o
	rm -f $(STMLIB)/STM32_USB-FS-Device_Driver/src/*.o
	rm -f $(LIBDIR)/STM32F37x_I2C_CPAL_Driver/src/*.o
	rm -f $(LIBS)

tshow:
	@echo "######################################################################################################"
	@echo "################# optimize settings: $(InfoTextLib), $(InfoTextSrc)"
	@echo "######################################################################################################"

