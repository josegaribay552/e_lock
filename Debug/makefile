################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include utilities/subdir.mk
-include startup/subdir.mk
-include source/tool/subdir.mk
-include source/TML/subdir.mk
-include source/subdir.mk
-include segger_sysview/subdir.mk
-include lwip/src/netif/ppp/subdir.mk
-include lwip/src/netif/subdir.mk
-include lwip/src/core/ipv6/subdir.mk
-include lwip/src/core/ipv4/subdir.mk
-include lwip/src/core/subdir.mk
-include lwip/src/api/subdir.mk
-include lwip/port/subdir.mk
-include lwip/contrib/apps/tcpecho/subdir.mk
-include drivers/freertos/subdir.mk
-include drivers/subdir.mk
-include device/subdir.mk
-include component/uart/subdir.mk
-include component/serial_manager/subdir.mk
-include component/lists/subdir.mk
-include board/subdir.mk
-include amazon-freertos/freertos/portable/subdir.mk
-include amazon-freertos/freertos/subdir.mk
-include NfcLibrary/NxpNci/src/subdir.mk
-include NfcLibrary/NdefLibrary/src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: frdmk64f_freertos_RTOS-Project_Step6.axf

# Tool invocations
frdmk64f_freertos_RTOS-Project_Step6.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -Xlinker -Map="frdmk64f_freertos_RTOS-Project_Step6.map" -Xlinker --gc-sections -Xlinker -print-memory-usage -Xlinker --sort-section=alignment -Xlinker --cref -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -T "frdmk64f_freertos_RTOS-Project_Step3_Debug.ld" -o "frdmk64f_freertos_RTOS-Project_Step6.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) frdmk64f_freertos_RTOS-Project_Step6.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size "frdmk64f_freertos_RTOS-Project_Step6.axf"; # arm-none-eabi-objcopy -v -O binary "frdmk64f_freertos_RTOS-Project_Step6.axf" "frdmk64f_freertos_RTOS-Project_Step6.bin" ; # checksum -p MK64FN1M0xxx12 -d "frdmk64f_freertos_RTOS-Project_Step6.bin";
	-@echo ' '

.PHONY: all clean dependents post-build

-include ../makefile.targets
