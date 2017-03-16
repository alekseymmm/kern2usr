################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../kern/fops_chdev.c \
../kern/kern_main.c 

OBJS += \
./kern/fops_chdev.o \
./kern/kern_main.o 

C_DEPS += \
./kern/fops_chdev.d \
./kern/kern_main.d 


# Each subdirectory must supply rules for building sources it contributes
kern/%.o: ../kern/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/root/workspace/KernUsr" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


