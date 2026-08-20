################################################################################
# MRS Version: 1.9.2
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../APP/peripheral.c \
../APP/peripheral_main.c 

OBJS += \
./APP/peripheral.o \
./APP/peripheral_main.o 

C_DEPS += \
./APP/peripheral.d \
./APP/peripheral_main.d 


# Each subdirectory must supply rules for building sources it contributes
APP/%.o: ../APP/%.c
	@	@	riscv-none-elf-gcc -march=rv32imc_zba_zbb_zbc_zbs_xw -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common --param=highcode-gen-section-name=1  -g -DDEBUG=2 -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\Startup" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0307 (gnss和rdss实现）\Peripheral\APP\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0307 (gnss和rdss实现）\Peripheral\Profile\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\StdPeriphDriver\inc" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0307 (gnss和rdss实现）\HAL\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\Ld" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0307 (gnss和rdss实现）\LIB" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

