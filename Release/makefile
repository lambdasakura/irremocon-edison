################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: pre-build main-build

# Main-build Target
main-build: libIntelligentRemocon.a

# Tool invocations
libIntelligentRemocon.a: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Archiver'
	i686-elf-ar -r  "libIntelligentRemocon.a" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) intel_mcu.bin
	-$(RM) $(OBJS)$(ARCHIVES)$(C_DEPS) libIntelligentRemocon.a
	-@echo ' '

pre-build:
	-@echo 'Clean project before build'
	-make clean
	-@echo ' '

post-build:
	-@echo 'Generate binary'
	-../internal_tools/generate_mcu_bin.sh  libIntelligentRemocon.a
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: main-build pre-build post-build

-include ../makefile.targets
