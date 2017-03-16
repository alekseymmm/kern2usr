################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../usr/main.c \
../usr/utils.c 

OBJS += \
./usr/main.o \
./usr/utils.o 

C_DEPS += \
./usr/main.d \
./usr/utils.d 


# Each subdirectory must supply rules for building sources it contributes
usr/%.o: ../usr/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -I"/root/workspace/KernUsr" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


