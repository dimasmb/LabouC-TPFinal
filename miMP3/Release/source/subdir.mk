################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/DAC.c \
../source/PIT.c \
../source/dac_out.c \
../source/fatfs_sdcard.c \
../source/filter.c \
../source/hardware.c \
../source/semihost_hardfault.c 

OBJS += \
./source/DAC.o \
./source/PIT.o \
./source/dac_out.o \
./source/fatfs_sdcard.o \
./source/filter.o \
./source/hardware.o \
./source/semihost_hardfault.o 

C_DEPS += \
./source/DAC.d \
./source/PIT.d \
./source/dac_out.d \
./source/fatfs_sdcard.d \
./source/filter.d \
./source/hardware.d \
./source/semihost_hardfault.d 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DCPU_MK64FN1M0VLL12 -DCPU_MK64FN1M0VLL12_cm4 -DSDK_DEBUGCONSOLE=0 -DCR_INTEGER_PRINTF -DFRDM_K64F -DFREEDOM -D__MCUXPRESSO -D__USE_CMSIS -DNDEBUG -I../board -I../source -I../ -I../drivers -I../CMSIS -I../utilities -I../startup -I../sdmmc/inc -I../fatfs/fatfs_include -I../board/src -I../sdmmc/port -Os -fno-common -g -Wall -c  -ffunction-sections  -fdata-sections  -ffreestanding  -fno-builtin -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


