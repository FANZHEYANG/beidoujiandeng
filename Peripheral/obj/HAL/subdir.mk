################################################################################
# MRS Version: 1.9.2
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
E:/postgraduate/project/beidou_light/05\ 传递的资料/后续资料/CH584_SW/BLE_all0818/HAL/AUDIO.c \
E:/postgraduate/project/beidou_light/05\ 传递的资料/后续资料/CH584_SW/BLE_all0818/HAL/KEY.c \
E:/postgraduate/project/beidou_light/05\ 传递的资料/后续资料/CH584_SW/BLE_all0818/HAL/LED.c \
E:/postgraduate/project/beidou_light/05\ 传递的资料/后续资料/CH584_SW/BLE_all0818/HAL/MCU.c \
E:/postgraduate/project/beidou_light/05\ 传递的资料/后续资料/CH584_SW/BLE_all0818/HAL/PWR.c \
E:/postgraduate/project/beidou_light/05\ 传递的资料/后续资料/CH584_SW/BLE_all0818/HAL/RTC.c \
E:/postgraduate/project/beidou_light/05\ 传递的资料/后续资料/CH584_SW/BLE_all0818/HAL/SLEEP.c \
E:/postgraduate/project/beidou_light/05\ 传递的资料/后续资料/CH584_SW/BLE_all0818/HAL/app_i2c.c \
E:/postgraduate/project/beidou_light/05\ 传递的资料/后续资料/CH584_SW/BLE_all0818/HAL/bq25601.c \
E:/postgraduate/project/beidou_light/05\ 传递的资料/后续资料/CH584_SW/BLE_all0818/HAL/decode.c 

OBJS += \
./HAL/AUDIO.o \
./HAL/KEY.o \
./HAL/LED.o \
./HAL/MCU.o \
./HAL/PWR.o \
./HAL/RTC.o \
./HAL/SLEEP.o \
./HAL/app_i2c.o \
./HAL/bq25601.o \
./HAL/decode.o 

C_DEPS += \
./HAL/AUDIO.d \
./HAL/KEY.d \
./HAL/LED.d \
./HAL/MCU.d \
./HAL/PWR.d \
./HAL/RTC.d \
./HAL/SLEEP.d \
./HAL/app_i2c.d \
./HAL/bq25601.d \
./HAL/decode.d 


# Each subdirectory must supply rules for building sources it contributes
HAL/AUDIO.o: E:/postgraduate/project/beidou_light/05\ 传递的资料/后续资料/CH584_SW/BLE_all0818/HAL/AUDIO.c
	@	@	riscv-none-elf-gcc -march=rv32imc_zba_zbb_zbc_zbs_xw -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common --param=highcode-gen-section-name=1  -g -DDEBUG=2 -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\Startup" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\Peripheral\APP\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\Peripheral\Profile\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\StdPeriphDriver\inc" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\HAL\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\Ld" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\LIB" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@
HAL/KEY.o: E:/postgraduate/project/beidou_light/05\ 传递的资料/后续资料/CH584_SW/BLE_all0818/HAL/KEY.c
	@	@	riscv-none-elf-gcc -march=rv32imc_zba_zbb_zbc_zbs_xw -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common --param=highcode-gen-section-name=1  -g -DDEBUG=2 -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\Startup" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\Peripheral\APP\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\Peripheral\Profile\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\StdPeriphDriver\inc" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\HAL\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\Ld" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\LIB" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@
HAL/LED.o: E:/postgraduate/project/beidou_light/05\ 传递的资料/后续资料/CH584_SW/BLE_all0818/HAL/LED.c
	@	@	riscv-none-elf-gcc -march=rv32imc_zba_zbb_zbc_zbs_xw -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common --param=highcode-gen-section-name=1  -g -DDEBUG=2 -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\Startup" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\Peripheral\APP\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\Peripheral\Profile\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\StdPeriphDriver\inc" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\HAL\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\Ld" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\LIB" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@
HAL/MCU.o: E:/postgraduate/project/beidou_light/05\ 传递的资料/后续资料/CH584_SW/BLE_all0818/HAL/MCU.c
	@	@	riscv-none-elf-gcc -march=rv32imc_zba_zbb_zbc_zbs_xw -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common --param=highcode-gen-section-name=1  -g -DDEBUG=2 -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\Startup" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\Peripheral\APP\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\Peripheral\Profile\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\StdPeriphDriver\inc" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\HAL\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\Ld" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\LIB" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@
HAL/PWR.o: E:/postgraduate/project/beidou_light/05\ 传递的资料/后续资料/CH584_SW/BLE_all0818/HAL/PWR.c
	@	@	riscv-none-elf-gcc -march=rv32imc_zba_zbb_zbc_zbs_xw -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common --param=highcode-gen-section-name=1  -g -DDEBUG=2 -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\Startup" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\Peripheral\APP\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\Peripheral\Profile\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\StdPeriphDriver\inc" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\HAL\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\Ld" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\LIB" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@
HAL/RTC.o: E:/postgraduate/project/beidou_light/05\ 传递的资料/后续资料/CH584_SW/BLE_all0818/HAL/RTC.c
	@	@	riscv-none-elf-gcc -march=rv32imc_zba_zbb_zbc_zbs_xw -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common --param=highcode-gen-section-name=1  -g -DDEBUG=2 -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\Startup" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\Peripheral\APP\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\Peripheral\Profile\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\StdPeriphDriver\inc" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\HAL\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\Ld" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\LIB" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@
HAL/SLEEP.o: E:/postgraduate/project/beidou_light/05\ 传递的资料/后续资料/CH584_SW/BLE_all0818/HAL/SLEEP.c
	@	@	riscv-none-elf-gcc -march=rv32imc_zba_zbb_zbc_zbs_xw -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common --param=highcode-gen-section-name=1  -g -DDEBUG=2 -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\Startup" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\Peripheral\APP\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\Peripheral\Profile\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\StdPeriphDriver\inc" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\HAL\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\Ld" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\LIB" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@
HAL/app_i2c.o: E:/postgraduate/project/beidou_light/05\ 传递的资料/后续资料/CH584_SW/BLE_all0818/HAL/app_i2c.c
	@	@	riscv-none-elf-gcc -march=rv32imc_zba_zbb_zbc_zbs_xw -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common --param=highcode-gen-section-name=1  -g -DDEBUG=2 -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\Startup" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\Peripheral\APP\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\Peripheral\Profile\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\StdPeriphDriver\inc" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\HAL\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\Ld" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\LIB" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@
HAL/bq25601.o: E:/postgraduate/project/beidou_light/05\ 传递的资料/后续资料/CH584_SW/BLE_all0818/HAL/bq25601.c
	@	@	riscv-none-elf-gcc -march=rv32imc_zba_zbb_zbc_zbs_xw -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common --param=highcode-gen-section-name=1  -g -DDEBUG=2 -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\Startup" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\Peripheral\APP\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\Peripheral\Profile\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\StdPeriphDriver\inc" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\HAL\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\Ld" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\LIB" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@
HAL/decode.o: E:/postgraduate/project/beidou_light/05\ 传递的资料/后续资料/CH584_SW/BLE_all0818/HAL/decode.c
	@	@	riscv-none-elf-gcc -march=rv32imc_zba_zbb_zbc_zbs_xw -mabi=ilp32 -mcmodel=medany -msmall-data-limit=8 -mno-save-restore -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -fno-common --param=highcode-gen-section-name=1  -g -DDEBUG=2 -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\Startup" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\Peripheral\APP\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\Peripheral\Profile\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\StdPeriphDriver\inc" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\HAL\include" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\Ld" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\BLE_all0818\LIB" -I"E:\postgraduate\project\beidou_light\05 传递的资料\后续资料\CH584_SW\SRC\RVMSIS" -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@	@

