################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../glcd-master/glcd.c \
../glcd-master/graphics.c \
../glcd-master/graphs.c \
../glcd-master/text.c \
../glcd-master/text_tiny.c \
../glcd-master/unit_tests.c 

OBJS += \
./glcd-master/glcd.o \
./glcd-master/graphics.o \
./glcd-master/graphs.o \
./glcd-master/text.o \
./glcd-master/text_tiny.o \
./glcd-master/unit_tests.o 

C_DEPS += \
./glcd-master/glcd.d \
./glcd-master/graphics.d \
./glcd-master/graphs.d \
./glcd-master/text.d \
./glcd-master/text_tiny.d \
./glcd-master/unit_tests.d 


# Each subdirectory must supply rules for building sources it contributes
glcd-master/%.o: ../glcd-master/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DPRINTF_FLOAT_ENABLE=0 -D__USE_CMSIS -DCR_INTEGER_PRINTF -DSDK_DEBUGCONSOLE=1 -D__MCUXPRESSO -DNDEBUG -DFSL_RTOS_BM -DSDK_OS_BAREMETAL -DCPU_MK64FN1M0VLL12 -DCPU_MK64FN1M0VLL12_cm4 -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master\board" -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master\source" -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master" -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master\drivers" -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master\utilities" -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master\startup" -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master\CMSIS" -Os -fno-common -g -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


