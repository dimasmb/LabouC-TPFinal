################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../helix/real/bitstream.c \
../helix/real/buffers.c \
../helix/real/dct32.c \
../helix/real/dequant.c \
../helix/real/dqchan.c \
../helix/real/huffman.c \
../helix/real/hufftabs.c \
../helix/real/imdct.c \
../helix/real/polyphase.c \
../helix/real/scalfact.c \
../helix/real/stproc.c \
../helix/real/subband.c \
../helix/real/trigtabs_fixpt.c 

OBJS += \
./helix/real/bitstream.o \
./helix/real/buffers.o \
./helix/real/dct32.o \
./helix/real/dequant.o \
./helix/real/dqchan.o \
./helix/real/huffman.o \
./helix/real/hufftabs.o \
./helix/real/imdct.o \
./helix/real/polyphase.o \
./helix/real/scalfact.o \
./helix/real/stproc.o \
./helix/real/subband.o \
./helix/real/trigtabs_fixpt.o 

C_DEPS += \
./helix/real/bitstream.d \
./helix/real/buffers.d \
./helix/real/dct32.d \
./helix/real/dequant.d \
./helix/real/dqchan.d \
./helix/real/huffman.d \
./helix/real/hufftabs.d \
./helix/real/imdct.d \
./helix/real/polyphase.d \
./helix/real/scalfact.d \
./helix/real/stproc.d \
./helix/real/subband.d \
./helix/real/trigtabs_fixpt.d 


# Each subdirectory must supply rules for building sources it contributes
helix/real/%.o: ../helix/real/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -D__REDLIB__ -DPRINTF_FLOAT_ENABLE=0 -D__USE_CMSIS -DCR_INTEGER_PRINTF -DSDK_DEBUGCONSOLE=1 -D__MCUXPRESSO -DNDEBUG -DFSL_RTOS_BM -DSDK_OS_BAREMETAL -DCPU_MK64FN1M0VLL12 -DCPU_MK64FN1M0VLL12_cm4 -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master\board" -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master\source" -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master" -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master\drivers" -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master\utilities" -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master\startup" -I"F:\Materias\2022Q2\Labo de Micros\TPs\GITs\LabouC-TPFinal\frdmk64f_mp3_player-master\CMSIS" -Os -fno-common -g -Wall -c -fmessage-length=0 -fno-builtin -ffunction-sections -fdata-sections -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -D__REDLIB__ -specs=redlib.specs -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


