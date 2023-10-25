################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../board/board.c \
../board/clock_config.c \
../board/peripherals.c \
../board/pin_mux.c 

OBJS += \
./board/board.o \
./board/clock_config.o \
./board/peripherals.o \
./board/pin_mux.o 

C_DEPS += \
./board/board.d \
./board/clock_config.d \
./board/peripherals.d \
./board/pin_mux.d 


# Each subdirectory must supply rules for building sources it contributes
board/%.o: ../board/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DPRINTF_FLOAT_ENABLE=0 -D__USE_CMSIS -DCR_INTEGER_PRINTF -DSDK_DEBUGCONSOLE=1 -D__MCUXPRESSO -DNDEBUG -DFSL_RTOS_BM -DSDK_OS_BAREMETAL -DCPU_MK64FN1M0VLL12 -DCPU_MK64FN1M0VLL12_cm4 -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master\board" -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master\source" -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master" -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master\drivers" -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master\utilities" -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master\startup" -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master\CMSIS" -Os -fno-common -g -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


