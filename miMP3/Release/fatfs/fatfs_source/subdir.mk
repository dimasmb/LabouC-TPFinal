################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../fatfs/fatfs_source/diskio.c \
../fatfs/fatfs_source/ff.c \
../fatfs/fatfs_source/ffsystem.c \
../fatfs/fatfs_source/ffunicode.c \
../fatfs/fatfs_source/fsl_sd_disk.c 

OBJS += \
./fatfs/fatfs_source/diskio.o \
./fatfs/fatfs_source/ff.o \
./fatfs/fatfs_source/ffsystem.o \
./fatfs/fatfs_source/ffunicode.o \
./fatfs/fatfs_source/fsl_sd_disk.o 

C_DEPS += \
./fatfs/fatfs_source/diskio.d \
./fatfs/fatfs_source/ff.d \
./fatfs/fatfs_source/ffsystem.d \
./fatfs/fatfs_source/ffunicode.d \
./fatfs/fatfs_source/fsl_sd_disk.d 


# Each subdirectory must supply rules for building sources it contributes
fatfs/fatfs_source/%.o: ../fatfs/fatfs_source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -std=gnu99 -D__REDLIB__ -DCPU_MK64FN1M0VLL12 -DCPU_MK64FN1M0VLL12_cm4 -DSDK_DEBUGCONSOLE=0 -DCR_INTEGER_PRINTF -DFRDM_K64F -DFREEDOM -D__MCUXPRESSO -D__USE_CMSIS -DNDEBUG -I../board -I../source -I../ -I../drivers -I../CMSIS -I../utilities -I../startup -I../sdmmc/inc -I../fatfs/fatfs_include -I../board/src -I../sdmmc/port -Os -fno-common -g -Wall -c  -ffunction-sections  -fdata-sections  -ffreestanding  -fno-builtin -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


