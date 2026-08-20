################################################################################
# MRS Version: 1.9.2
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
E:/postgraduate/project/beidou_light/05\ 传递的资料/后续资料/CH584_SW/BLE_all0818/LIB/ble_task_scheduler.S 

OBJS += \
./LIB/ble_task_scheduler.o 

S_UPPER_DEPS += \
./LIB/ble_task_scheduler.d 


# Each subdirectory must supply rules for building sources it contributes
LIB/ble_task_scheduler.o: E:/postgraduate/project/beidou_light/05\ 传递的资料/后续资料/CH584_SW/BLE_all0818/LIB/ble_task_scheduler.S
	@	@	riscv-none-elf-gcc -march=rv32imc_zba_zbb_zbc_zbs_xw -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common --param=highcode-gen-section-name=1  -g -x assembler-with-cpp -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

