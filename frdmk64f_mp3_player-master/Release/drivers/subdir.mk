################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../drivers/fsl_clock.c \
../drivers/fsl_common.c \
../drivers/fsl_dac.c \
../drivers/fsl_dmamux.c \
../drivers/fsl_edma.c \
../drivers/fsl_flash.c \
../drivers/fsl_ftm.c \
../drivers/fsl_gpio.c \
../drivers/fsl_i2c.c \
../drivers/fsl_i2c_edma.c \
../drivers/fsl_pit.c \
../drivers/fsl_sdhc.c \
../drivers/fsl_smc.c \
../drivers/fsl_sysmpu.c \
../drivers/fsl_uart.c \
../drivers/fsl_uart_edma.c \
../drivers/fsl_vref.c \
../drivers/fsl_wdog.c 

OBJS += \
./drivers/fsl_clock.o \
./drivers/fsl_common.o \
./drivers/fsl_dac.o \
./drivers/fsl_dmamux.o \
./drivers/fsl_edma.o \
./drivers/fsl_flash.o \
./drivers/fsl_ftm.o \
./drivers/fsl_gpio.o \
./drivers/fsl_i2c.o \
./drivers/fsl_i2c_edma.o \
./drivers/fsl_pit.o \
./drivers/fsl_sdhc.o \
./drivers/fsl_smc.o \
./drivers/fsl_sysmpu.o \
./drivers/fsl_uart.o \
./drivers/fsl_uart_edma.o \
./drivers/fsl_vref.o \
./drivers/fsl_wdog.o 

C_DEPS += \
./drivers/fsl_clock.d \
./drivers/fsl_common.d \
./drivers/fsl_dac.d \
./drivers/fsl_dmamux.d \
./drivers/fsl_edma.d \
./drivers/fsl_flash.d \
./drivers/fsl_ftm.d \
./drivers/fsl_gpio.d \
./drivers/fsl_i2c.d \
./drivers/fsl_i2c_edma.d \
./drivers/fsl_pit.d \
./drivers/fsl_sdhc.d \
./drivers/fsl_smc.d \
./drivers/fsl_sysmpu.d \
./drivers/fsl_uart.d \
./drivers/fsl_uart_edma.d \
./drivers/fsl_vref.d \
./drivers/fsl_wdog.d 


# Each subdirectory must supply rules for building sources it contributes
drivers/%.o: ../drivers/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DPRINTF_FLOAT_ENABLE=0 -D__USE_CMSIS -DCR_INTEGER_PRINTF -DSDK_DEBUGCONSOLE=1 -D__MCUXPRESSO -DNDEBUG -DFSL_RTOS_BM -DSDK_OS_BAREMETAL -DCPU_MK64FN1M0VLL12 -DCPU_MK64FN1M0VLL12_cm4 -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master\board" -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master\source" -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master" -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master\drivers" -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master\utilities" -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master\startup" -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master\CMSIS" -Os -fno-common -g -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


