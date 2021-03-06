#ifndef  __MESON_GPIO_H__
#define	 __MESON_GPIO_H__

#include "io.h"
//#include "gpio_name.h"
typedef enum gpio_bank {
    PREG_PAD_GPIO0 = 0,
    PREG_PAD_GPIO1,
    PREG_PAD_GPIO2,
    PREG_PAD_GPIO3,
    PREG_PAD_GPIO4,
    PREG_PAD_GPIO5,
	PREG_PAD_GPIOAO,
	PREG_JTAG_GPIO,
} gpio_bank_t;


typedef enum gpio_mode {
    GPIO_OUTPUT_MODE,
    GPIO_INPUT_MODE,
} gpio_mode_t;

int set_gpio_mode(gpio_bank_t bank, int bit, gpio_mode_t mode);
gpio_mode_t get_gpio_mode(gpio_bank_t bank, int bit);

int set_gpio_val(gpio_bank_t bank, int bit, unsigned long val);
unsigned long  get_gpio_val(gpio_bank_t bank, int bit);

#define GPIOA_bank_bit0_27(bit)     (PREG_PAD_GPIO0)
#define GPIOA_bit_bit0_27(bit)      (bit)

#define GPIOB_bank_bit0_23(bit)     (PREG_PAD_GPIO1)
#define GPIOB_bit_bit0_23(bit)      (bit)

#define GPIOC_bank_bit0_15(bit)     (PREG_PAD_GPIO2)
#define GPIOC_bit_bit0_15(bit)      (bit)

#define GPIOAO_bank_bit0_11(bit)    (PREG_PAD_GPIOAO)
#define GPIOAO_bit_bit0_11(bit)     (bit)

#define GPIOD_bank_bit0_9(bit)      (PREG_PAD_GPIO2)
#define GPIOD_bit_bit0_9(bit)       (bit+16)

#define GPIOCARD_bank_bit0_8(bit)   (PREG_PAD_GPIO5)
#define GPIOCARD_bit_bit0_8(bit)    (bit+23)

#define GPIOBOOT_bank_bit0_17(bit)  (PREG_PAD_GPIO3)
#define GPIOBOOT_bit_bit0_17(bit)   (bit)

#define GPIOX_bank_bit0_31(bit)     (PREG_PAD_GPIO4)
#define GPIOX_bit_bit0_31(bit)      (bit)

#define GPIOX_bank_bit32_35(bit)    (PREG_PAD_GPIO3)
#define GPIOX_bit_bit32_35(bit)     (bit+20)

#define GPIOY_bank_bit0_22(bit)     (PREG_PAD_GPIO5)
#define GPIOY_bit_bit0_22(bit)      (bit)

/**
 * enable gpio edge interrupt
 *	
 * @param [in] pin  index number of the chip, start with 0 up to 255 
 * @param [in] flag rising(0) or falling(1) edge 
 * @param [in] group  this interrupt belong to which interrupt group  from 0 to 7
 */
extern void gpio_enable_edge_int(int pin , int flag, int group);
/**
 * enable gpio level interrupt
 *	
 * @param [in] pin  index number of the chip, start with 0 up to 255 
 * @param [in] flag high(0) or low(1) level 
 * @param [in] group  this interrupt belong to which interrupt group  from 0 to 7
 */
extern void gpio_enable_level_int(int pin , int flag, int group);

/**
 * enable gpio interrupt filter
 *
 * @param [in] filter from 0~7(*222ns)
 * @param [in] group  this interrupt belong to which interrupt group  from 0 to 7
 */
extern void gpio_enable_int_filter(int filter, int group);

extern int gpio_is_valid(int number);
extern int gpio_request(unsigned gpio, const char *label);
extern void gpio_free(unsigned gpio);
extern int gpio_direction_input(unsigned gpio);
extern int gpio_direction_output(unsigned gpio, int value);
extern void gpio_set_value(unsigned gpio, int value);
extern int gpio_get_value(unsigned gpio);

typedef enum {
	GPIOZ_0=0,
	GPIOZ_1=1,
	GPIOZ_2=2,
	GPIOZ_3=3,
	GPIOZ_4=4,
	GPIOZ_5=5,
	GPIOZ_6=6,
	GPIOZ_7=7,
	GPIOZ_8=8,
	GPIOZ_9=9,
	GPIOZ_10=10,
	GPIOZ_11=11,
	GPIOZ_12=12,
	GPIOE_0=13,
	GPIOE_1=14,
	GPIOE_2=15,
	GPIOE_3=16,
	GPIOE_4=17,
	GPIOE_5=18,
	GPIOE_6=19,
	GPIOE_7=20,
	GPIOE_8=21,
	GPIOE_9=22,
	GPIOE_10=23,
	GPIOE_11=24,
	GPIOY_0=25,
	GPIOY_1=26,
	GPIOY_2=27,
	GPIOY_3=28,
	GPIOY_4=29,
	GPIOY_5=30,
	GPIOY_6=31,
	GPIOY_7=32,
	GPIOY_8=33,
	GPIOY_9=34,
	GPIOY_10=35,
	GPIOY_11=36,
	GPIOY_12=37,
	GPIOY_13=38,
	GPIOY_14=39,
	GPIOY_15=40,
	GPIOX_0=41,
	GPIOX_1=42,
	GPIOX_2=43,
	GPIOX_3=44,
	GPIOX_4=45,
	GPIOX_5=46,
	GPIOX_6=47,
	GPIOX_7=48,
	GPIOX_8=49,
	GPIOX_9=50,
	GPIOX_10=51,
	GPIOX_11=52,
	GPIOX_12=53,
	GPIOX_13=54,
	GPIOX_14=55,
	GPIOX_15=56,
	GPIOX_16=57,
	GPIOX_17=58,
	GPIOX_18=59,
	GPIOX_19=60,
	GPIOX_20=61,
	GPIOX_21=62,
	GPIOX_22=63,
	GPIOX_23=64,
	GPIOX_24=65,
	GPIOX_25=66,
	GPIOX_26=67,
	GPIOX_27=68,
	GPIOX_28=69,
	GPIOX_29=70,
	GPIOX_30=71,
	GPIOX_31=72,
	GPIOX_32=73,
	GPIOX_33=74,
	GPIOX_34=75,
	GPIOX_35=76,
	BOOT_0=77,
	BOOT_1=78,
	BOOT_2=79,
	BOOT_3=80,
	BOOT_4=81,
	BOOT_5=82,
	BOOT_6=83,
	BOOT_7=84,
	BOOT_8=85,
	BOOT_9=86,
	BOOT_10=87,
	BOOT_11=88,
	BOOT_12=89,
	BOOT_13=90,
	BOOT_14=91,
	BOOT_15=92,
	BOOT_16=93,
	BOOT_17=94,
	GPIOD_0=95,
	GPIOD_1=96,
	GPIOD_2=97,
	GPIOD_3=98,
	GPIOD_4=99,
	GPIOD_5=100,
	GPIOD_6=101,
	GPIOD_7=102,
	GPIOD_8=103,
	GPIOD_9=104,
	GPIOC_0=105,
	GPIOC_1=106,
	GPIOC_2=107,
	GPIOC_3=108,
	GPIOC_4=109,
	GPIOC_5=110,
	GPIOC_6=111,
	GPIOC_7=112,
	GPIOC_8=113,
	GPIOC_9=114,
	GPIOC_10=115,
	GPIOC_11=116,
	GPIOC_12=117,
	GPIOC_13=118,
	GPIOC_14=119,
	GPIOC_15=120,
	CARD_0=121,
	CARD_1=122,
	CARD_2=123,
	CARD_3=124,
	CARD_4=125,
	CARD_5=126,
	CARD_6=127,
	CARD_7=128,
	CARD_8=129,
	GPIOB_0=130,
	GPIOB_1=131,
	GPIOB_2=132,
	GPIOB_3=133,
	GPIOB_4=134,
	GPIOB_5=135,
	GPIOB_6=136,
	GPIOB_7=137,
	GPIOB_8=138,
	GPIOB_9=139,
	GPIOB_10=140,
	GPIOB_11=141,
	GPIOB_12=142,
	GPIOB_13=143,
	GPIOB_14=144,
	GPIOB_15=145,
	GPIOB_16=146,
	GPIOB_17=147,
	GPIOB_18=148,
	GPIOB_19=149,
	GPIOB_20=150,
	GPIOB_21=151,
	GPIOB_22=152,
	GPIOB_23=153,
	GPIOA_0=154,
	GPIOA_1=155,
	GPIOA_2=156,
	GPIOA_3=157,
	GPIOA_4=158,
	GPIOA_5=159,
	GPIOA_6=160,
	GPIOA_7=161,
	GPIOA_8=162,
	GPIOA_9=163,
	GPIOA_10=164,
	GPIOA_11=165,
	GPIOA_12=166,
	GPIOA_13=167,
	GPIOA_14=168,
	GPIOA_15=169,
	GPIOA_16=170,
	GPIOA_17=171,
	GPIOA_18=172,
	GPIOA_19=173,
	GPIOA_20=174,
	GPIOA_21=175,
	GPIOA_22=176,
	GPIOA_23=177,
	GPIOA_24=178,
	GPIOA_25=179,
	GPIOA_26=180,
	GPIOA_27=181,
	GPIOAO_0=182,
	GPIOAO_1=183,
	GPIOAO_2=184,
	GPIOAO_3=185,
	GPIOAO_4=186,
	GPIOAO_5=187,
	GPIOAO_6=188,
	GPIOAO_7=189,
	GPIOAO_8=190,
	GPIOAO_9=191,
	GPIOAO_10=192,
	GPIOAO_11=193,
	TEST_N=194,
	GPIO_MAX=195,
}gpio_t;

#endif
