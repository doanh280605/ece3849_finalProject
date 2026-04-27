################################################################################
# Automatically-generated file. Do not edit!
################################################################################

CG_TOOL_ROOT := /Applications/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./Crystalfontz128x128_ST7735.obj" \
"./HAL_EK_TM4C1294XL_Crystalfontz128x128_ST7735.obj" \
"./button.obj" \
"./controller_logic.obj" \
"./debug_uart.obj" \
"./joystick.obj" \
"./main.obj" \
"./motor.obj" \
"./safety.obj" \
"./sensor.obj" \
"./startup_ccs.obj" \
"./system_tasks.obj" \
"./timerLib.obj" \
"./wireless.obj" \
"./FreeRTOS/croutine.obj" \
"./FreeRTOS/event_groups.obj" \
"./FreeRTOS/list.obj" \
"./FreeRTOS/queue.obj" \
"./FreeRTOS/stream_buffer.obj" \
"./FreeRTOS/tasks.obj" \
"./FreeRTOS/timers.obj" \
"./FreeRTOS/portable/CCS/ARM_CM4F/port.obj" \
"./FreeRTOS/portable/CCS/ARM_CM4F/portasm.obj" \
"./FreeRTOS/portable/MemMang/heap_4.obj" \
"../tm4c1294ncpdt.cmd" \
$(GEN_CMDS__FLAG) \
-llibc.a \
-l"/Users/Doanh/ti/TivaWare_C_Series-2.2.0.295/grlib/ccs/Debug/grlib.lib" \
-l"/Users/Doanh/ti/TivaWare_C_Series-2.2.0.295/driverlib/ccs/Debug/driverlib.lib" \

-include ../makefile.init

RM := rm -rf
RMDIR := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include FreeRTOS/subdir_vars.mk
-include FreeRTOS/portable/CCS/ARM_CM4F/subdir_vars.mk
-include FreeRTOS/portable/MemMang/subdir_vars.mk
-include subdir_rules.mk
-include FreeRTOS/subdir_rules.mk
-include FreeRTOS/portable/CCS/ARM_CM4F/subdir_rules.mk
-include FreeRTOS/portable/MemMang/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
ece3849_finalProject_lphung_shtran.out 

EXE_OUTPUTS__QUOTED += \
"ece3849_finalProject_lphung_shtran.out" 


# All Target
all: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "ece3849_finalProject_lphung_shtran.out"

# Tool invocations
ece3849_finalProject_lphung_shtran.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: Arm Linker'
	"/Applications/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --define=ccs="ccs" --define=PART_TM4C1294NCPDT -g --gcc --diag_warning=225 --diag_wrap=off --display_error_number --abi=eabi -z -m"ece3849_finalProject_lphung_shtran.map" --heap_size=0 --stack_size=2048 -i"/Applications/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/lib" -i"/Applications/ti/ccs1281/ccs/tools/compiler/ti-cgt-arm_20.2.7.LTS/include" --reread_libs --diag_wrap=off --display_error_number --warn_sections --xml_link_info="ece3849_finalProject_lphung_shtran_linkInfo.xml" --rom_model -o "ece3849_finalProject_lphung_shtran.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXE_OUTPUTS__QUOTED)
	-$(RM) "Crystalfontz128x128_ST7735.obj" "HAL_EK_TM4C1294XL_Crystalfontz128x128_ST7735.obj" "button.obj" "controller_logic.obj" "debug_uart.obj" "joystick.obj" "main.obj" "motor.obj" "safety.obj" "sensor.obj" "startup_ccs.obj" "system_tasks.obj" "timerLib.obj" "wireless.obj" "FreeRTOS/croutine.obj" "FreeRTOS/event_groups.obj" "FreeRTOS/list.obj" "FreeRTOS/queue.obj" "FreeRTOS/stream_buffer.obj" "FreeRTOS/tasks.obj" "FreeRTOS/timers.obj" "FreeRTOS/portable/CCS/ARM_CM4F/port.obj" "FreeRTOS/portable/CCS/ARM_CM4F/portasm.obj" "FreeRTOS/portable/MemMang/heap_4.obj" 
	-$(RM) "Crystalfontz128x128_ST7735.d" "HAL_EK_TM4C1294XL_Crystalfontz128x128_ST7735.d" "startup_ccs.d" "FreeRTOS/croutine.d" "FreeRTOS/event_groups.d" "FreeRTOS/list.d" "FreeRTOS/queue.d" "FreeRTOS/stream_buffer.d" "FreeRTOS/tasks.d" "FreeRTOS/timers.d" "FreeRTOS/portable/CCS/ARM_CM4F/port.d" "FreeRTOS/portable/MemMang/heap_4.d" 
	-$(RM) "button.d" "controller_logic.d" "debug_uart.d" "joystick.d" "main.d" "motor.d" "safety.d" "sensor.d" "system_tasks.d" "timerLib.d" "wireless.d" 
	-$(RM) "FreeRTOS/portable/CCS/ARM_CM4F/portasm.d" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

