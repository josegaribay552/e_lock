################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/RTOS-Project.c \
../source/gpioint.c \
../source/ndef_helper.c \
../source/nfc_task.c \
../source/semihost_hardfault.c \
../source/servo.c \
../source/tcp_webclient.c \
../source/tcpipserver_task.c 

OBJS += \
./source/RTOS-Project.o \
./source/gpioint.o \
./source/ndef_helper.o \
./source/nfc_task.o \
./source/semihost_hardfault.o \
./source/servo.o \
./source/tcp_webclient.o \
./source/tcpipserver_task.o 

C_DEPS += \
./source/RTOS-Project.d \
./source/gpioint.d \
./source/ndef_helper.d \
./source/nfc_task.d \
./source/semihost_hardfault.d \
./source/servo.d \
./source/tcp_webclient.d \
./source/tcpipserver_task.d 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DRW_SUPPORT -DCPU_MK64FN1M0VLL12 -DUSE_RTOS=1 -DPRINTF_ADVANCED_ENABLE=1 -DFRDM_K64F -DFREEDOM -DFSL_RTOS_FREE_RTOS -DCPU_MK64FN1M0VLL12_cm4 -DCR_INTEGER_PRINTF -D__MCUXPRESSO -D__USE_CMSIS -DDEBUG -DSDK_OS_FREE_RTOS -DFSL_RTOS_BM -DSDK_OS_BAREMETAL -DSDK_DEBUGCONSOLE=0 -I../drivers -I../drivers/freertos -I../component/serial_manager -I../amazon-freertos/freertos/portable -I../amazon-freertos/include -I../utilities -I../CMSIS -I../source -I../lwip/port -I../board -I../lwip/contrib/apps/tcpecho -I../component/lists -I../device -I../lwip/port/arch -I../lwip/src/include/compat/posix/arpa -I../lwip/src/include/compat/posix/net -I../lwip/src/include/compat/posix -I../lwip/src/include/compat/posix/sys -I../lwip/src/include/compat/stdc -I../lwip/src/include/lwip -I../lwip/src/include/lwip/priv -I../lwip/src/include/lwip/prot -I../lwip/src/include/netif -I../lwip/src/include/netif/ppp -I../lwip/src/include/netif/ppp/polarssl -I../component/uart -I../lwip/src -I../lwip/src/include -I"C:\workspace\frdmk64f_freertos_RTOS-Project_Step6\NfcLibrary" -I"C:\workspace\frdmk64f_freertos_RTOS-Project_Step6\source\TML" -I"C:\workspace\frdmk64f_freertos_RTOS-Project_Step6\source\tool" -I"C:\workspace\frdmk64f_freertos_RTOS-Project_Step6\segger_sysview" -I"C:\workspace\frdmk64f_freertos_RTOS-Project_Step6\NfcLibrary\inc" -I"C:\workspace\frdmk64f_freertos_RTOS-Project_Step6\NfcLibrary\NdefLibrary\inc" -I"C:\workspace\frdmk64f_freertos_RTOS-Project_Step6\NfcLibrary\NxpNci\inc" -I../ -O0 -fno-common -g3 -Wall -c  -ffunction-sections  -fdata-sections  -ffreestanding  -fno-builtin -fmerge-constants -fmacro-prefix-map="../$(@D)/"=. -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -fstack-usage -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


