#include <stdio.h>
#include "soc/reg_base.h" // DR_REG_GPIO_BASE, DR_REG_IO_MUX_BASE
#include "driver/rtc_io.h" // rtc_gpio_*
#include "pin.h"

// TODO: GPIO Matrix Registers - GPIO_OUT_REG, GPIO_OUT_W1TS_REG, ...
// NOTE: Remember to enclose the macro values in parenthesis, as below
#define GPIO_OUT_REG          			(DR_REG_GPIO_BASE+0x04)
#define GPIO_OUT1_REG					(DR_REG_GPIO_BASE+0x0010)
#define GPIO_FUNC_OUT_SEL_CFG_REG(n)  	(DR_REG_GPIO_BASE+0x530+0x4*(n))
#define GPIO_PIN_REG(n)  				(DR_REG_GPIO_BASE+0x88+0x4*(n))
#define GPIO_ENABLE_REG  				(DR_REG_GPIO_BASE+0x0020)
#define GPIO_ENABLE1_REG				(DR_REG_GPIO_BASE+0x002c)
#define GPIO_IN_REG  					(DR_REG_GPIO_BASE+0x003c)
#define GPIO_IN1_REG  					(DR_REG_GPIO_BASE+0x0040)

// GPIO PIN Register Fields
#define GPIO_PIN_PAD_DRIVER 2

// TODO: IO MUX Registers
// HINT: Add DR_REG_IO_MUX_BASE with PIN_MUX_REG_OFFSET[n]
#define IO_MUX_REG(n) (DR_REG_IO_MUX_BASE+PIN_MUX_REG_OFFSET[n])
// TODO: Finish this macro

// TODO: IO MUX Register Fields - FUN_WPD, FUN_WPU, ...
#define MCU_IE   4
#define MCU_DRV  5
#define FUN_WPD  7
#define FUN_WPU  8
#define FUN_IE   9
#define FUN_DRV  10
#define MCU_SEL  12


#define REG(r) (*(volatile uint32_t *)(r))
#define REG_BITS 32
// TODO: Finish these macros. HINT: Use the REG() macro.
#define REG_SET_BIT(r,b) REG(r)=(REG(r)|(0x01<<(b)))
#define REG_CLR_BIT(r,b) REG(r)=(REG(r)&(~(0x01<<(b))))
#define REG_GET_BIT(r,b) ((REG(r)&(0x01<<(b)))>>(b))

// Gives byte offset of IO_MUX Configuration Register
// from base address DR_REG_IO_MUX_BASE
static const uint8_t PIN_MUX_REG_OFFSET[] = {
    0x44, 0x88, 0x40, 0x84, 0x48, 0x6c, 0x60, 0x64, // pin  0- 7
    0x68, 0x54, 0x58, 0x5c, 0x34, 0x38, 0x30, 0x3c, // pin  8-15
    0x4c, 0x50, 0x70, 0x74, 0x78, 0x7c, 0x80, 0x8c, // pin 16-23
    0x90, 0x24, 0x28, 0x2c, 0xFF, 0xFF, 0xFF, 0xFF, // pin 24-31
    0x1c, 0x20, 0x14, 0x18, 0x04, 0x08, 0x0c, 0x10, // pin 32-39
};




#define GPIO_FUNC_OUT_RESET_VAL 0x100
// Reset the configuration of a pin to not be an input or an output.
// Pull-up is enabled so the pin does not float.
// Return zero if successful, or non-zero otherwise.
int32_t pin_reset(pin_num_t pin)
{
	if (rtc_gpio_is_valid_gpio(pin)) { // hand-off work to RTC subsystem
		rtc_gpio_deinit(pin);
		rtc_gpio_pullup_en(pin);
		rtc_gpio_pulldown_dis(pin);
	}
	// TODO: Reset GPIO_PINn_REG: All fields zero
	REG(GPIO_PIN_REG(pin)) = 0x00;

	// TODO: Reset GPIO_FUNCn_OUT_SEL_CFG_REG: GPIO_FUNCn_OUT_SEL=0x100
	REG(GPIO_FUNC_OUT_SEL_CFG_REG(pin)) = GPIO_FUNC_OUT_RESET_VAL;

	// TODO: Reset IO_MUX_x_REG: MCU_SEL=2, FUN_DRV=2, FUN_WPU=1
	REG(IO_MUX_REG(pin)) = 0x0;
	REG_SET_BIT(IO_MUX_REG(pin), MCU_SEL+1);
	REG_SET_BIT(IO_MUX_REG(pin), FUN_DRV+1);
	REG_SET_BIT(IO_MUX_REG(pin), FUN_WPU);

	// NOTE: By default, pin should not float, save power with FUN_WPU=1
	printf("DR_REG_GPIO_BASE: 0x%x\n", DR_REG_GPIO_BASE);
	printf("DR_REG_IO_MUX_BASE: 0x%x\n", DR_REG_IO_MUX_BASE);

	// Now that the pin is reset, set the output level to zero
	return pin_set_level(pin, 0);
}

// Enable or disable a pull-up on the pin.
// Return zero if successful, or non-zero otherwise.
int32_t pin_pullup(pin_num_t pin, bool enable)
{
	printf("pullup before: %x\n", IO_MUX_REG(pin));
	if (rtc_gpio_is_valid_gpio(pin)) { // hand-off work to RTC subsystem
		if (enable) return rtc_gpio_pullup_en(pin);
		else return rtc_gpio_pullup_dis(pin);
	}
	// TODO: Set or clear the FUN_WPU bit in an IO_MUX register
	if (enable) {
		REG_SET_BIT(IO_MUX_REG(pin), FUN_WPU);
	} else {
		REG_CLR_BIT(IO_MUX_REG(pin), FUN_WPU);
	}
	printf("pullup after: %x\n", IO_MUX_REG(pin));
	return 0;
}

// Enable or disable a pull-down on the pin.
// Return zero if successful, or non-zero otherwise.
int32_t pin_pulldown(pin_num_t pin, bool enable)
{
	printf("pulldown before: %x\n", IO_MUX_REG(pin));
	if (rtc_gpio_is_valid_gpio(pin)) { // hand-off work to RTC subsystem
		if (enable) return rtc_gpio_pulldown_en(pin);
		else return rtc_gpio_pulldown_dis(pin);
	}
	// TODO: Set or clear the FUN_WPD bit in an IO_MUX register
	if (enable) {
		REG_SET_BIT(IO_MUX_REG(pin), FUN_WPD);
	} else {
		REG_CLR_BIT(IO_MUX_REG(pin), FUN_WPD);
	}
	printf("pulldown after: %x\n", IO_MUX_REG(pin));
	return 0;
}

// Enable or disable the pin as an input signal.
// Return zero if successful, or non-zero otherwise.
int32_t pin_input(pin_num_t pin, bool enable)
{
	// TODO: Set or clear the FUN_IE bit in an IO_MUX register
	printf("set input before: %x,  enable: %u\n", IO_MUX_REG(pin), enable);
	if (enable) {
		REG_SET_BIT(IO_MUX_REG(pin), FUN_IE);
	} else {
		REG_CLR_BIT(IO_MUX_REG(pin), FUN_IE);
	}
	printf("set input before: %x, enable: %u\n", IO_MUX_REG(pin), enable);
	return 0;
}

// Enable or disable the pin as an output signal.
// Return zero if successful, or non-zero otherwise.
int32_t pin_output(pin_num_t pin, bool enable)
{
	// TODO: Set or clear the I/O pin bit in the ENABLE or ENABLE1 register
	printf("set output before: %x,  enable: %u\n", IO_MUX_REG(pin), enable);
	if (enable) {
		if (pin < REG_BITS) {
			REG_SET_BIT(GPIO_ENABLE_REG, pin);
		} else {
			REG_SET_BIT(GPIO_ENABLE1_REG, pin-REG_BITS);
		}
	} else {
		if (pin < REG_BITS) {
			REG_CLR_BIT(GPIO_ENABLE_REG, pin);
		} else {
			REG_CLR_BIT(GPIO_ENABLE1_REG, pin-REG_BITS);
		}
	}
	printf("set output after: %x\n", IO_MUX_REG(pin));
	return 0;
}

// Enable or disable the pin as an open-drain signal.
// Return zero if successful, or non-zero otherwise.
int32_t pin_odrain(pin_num_t pin, bool enable)
{
	// TODO: Set or clear the PAD_DRIVER bit in a PIN register
	if (enable) {
		REG_SET_BIT(GPIO_PIN_REG(pin), GPIO_PIN_PAD_DRIVER);
	} else {
		REG_CLR_BIT(GPIO_PIN_REG(pin), GPIO_PIN_PAD_DRIVER);
	}
	return 0;
}

// Sets the output signal level if the pin is configured as an output.
// Return zero if successful, or non-zero otherwise.
int32_t pin_set_level(pin_num_t pin, int32_t level)
{
	// TODO: Set or clear the I/O pin bit in the OUT or OUT1 register
	if (pin < REG_BITS) {
		REG_SET_BIT(GPIO_OUT_REG, pin);
	} else {
		REG_CLR_BIT(GPIO_OUT1_REG, pin-REG_BITS);
	}
	return 0;
}

// Gets the input signal level if the pin is configured as an input.
// Return zero or one if successful, or negative otherwise.
int32_t pin_get_level(pin_num_t pin)
{
	// TODO: Get the I/O pin bit from the IN or IN1 register
	if (pin < REG_BITS) {
		return REG_GET_BIT(GPIO_IN_REG, pin);
	} else {
		return REG_GET_BIT(GPIO_IN1_REG, pin-REG_BITS);
	}
}

// Get the value of the input registers, one pin per bit.
// The two 32-bit input registers are concatenated into a uint64_t.
uint64_t pin_get_in_reg(void)
{
	// TODO: Read the IN and IN1 registers, return the concatenated values
	uint64_t in = REG(GPIO_IN_REG);
	uint64_t in1 = REG(GPIO_IN1_REG);
	return in | in1<<REG_BITS;
}

// Get the value of the output registers, one pin per bit.
// The two 32-bit output registers are concatenated into a uint64_t.
uint64_t pin_get_out_reg(void)
{
	// TODO: Read the OUT and OUT1 registers, return the concatenated values
	uint64_t out = REG(GPIO_OUT_REG);
	uint64_t out1 = REG(GPIO_OUT1_REG);
	return out | out1<<REG_BITS;
}
