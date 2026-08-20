################################################################################
# MRS Version: 1.9.2
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Profile/devinfoservice.c \
../Profile/gattprofile.c 

OBJS += \
./Profile/devinfoservice.o \
./Profile/gattprofile.o 

C_DEPS += \
./Profile/devinfoservice.d \
./Profile/gattprofile.d 


# Each subdirectory must supply rules for building sources it contributes
Profile/%.o: ../Profile/%.c
	@	@	riscv-none-elf-gcc -march=rv32imc_zba_zbb_zbc_zbs_xw -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common --param=highcode-gen-section-name=1  -g -DDEBUG=2 -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\Startup" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\Peripheral\APP\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\Peripheral\Profile\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\StdPeriphDriver\inc" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\HAL\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\Ld" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\LIB" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

