/*
 * READ ME:
 * This file is in charge of power manager. It need match different power control, PMU(if any) and wakeup condition.
 * We need special this file in customer/board dir to replace the default pwr_op.c.
 *
*/

/*
 * This pwr_op.c is supply for axp202.
*/
#include <i2c.h>
#include <asm/arch/ao_reg.h>
#include <arc_pwr.h>



#define CONFIG_IR_REMOTE_WAKEUP 1//for M8 MBox

#ifdef CONFIG_IR_REMOTE_WAKEUP
#include "irremote2arc.c"
#endif

/*
 * i2c clock speed define for 32K and 24M mode
 */
#define I2C_SUSPEND_SPEED    6                  // speed = 8KHz / I2C_SUSPEND_SPEED
#define I2C_RESUME_SPEED    60                  // speed = 6MHz / I2C_RESUME_SPEED

#define I2C_RN5T618_ADDR   (0x64 >> 1)
#define i2c_pmu_write_b(reg, val)       i2c_pmu_write_12(reg, 1, val)
#define i2c_pmu_write_w(reg, val)       i2c_pmu_write_12(reg, 2, val)
#define i2c_pmu_read_b(reg)             (unsigned  char)i2c_pmu_read_12(reg, 1)
#define i2c_pmu_read_w(reg)             (unsigned short)i2c_pmu_read_12(reg, 2)
static unsigned char vbus_status;

void printf_arc(const char *str)
{
    f_serial_puts(str);
    wait_uart_empty();
}

void i2c_pmu_write_12(unsigned int reg, int size, unsigned short val)
{
	unsigned char buff[3];
	buff[0] = reg & 0xff;
	buff[1] = val & 0xff;

    if (size == 2) {
    	buff[2] = (val >> 8) & 0xff;
    }

	struct i2c_msg msg[] = {
        {
            .addr  = I2C_RN5T618_ADDR,
            .flags = 0,
            .len   = 1 + size,
            .buf   = buff,
        }
    };

    if (do_msgs(msg, 1) < 0) {
        printf_arc("i2c write failed\n");
    }
}

unsigned short i2c_pmu_read_12(unsigned int reg, int size)
{
	unsigned char buff[1];
    unsigned short val = 0;
	buff[0] = reg & 0xff;
    
    struct i2c_msg msgs[] = {
        {
            .addr = I2C_RN5T618_ADDR,
            .flags = 0,
            .len = 1,
            .buf = buff,
        },
        {
            .addr = I2C_RN5T618_ADDR,
            .flags = I2C_M_RD,
            .len = size,
            .buf = &val,
        }
    };

    if (do_msgs(msgs, 2)< 0) {
       printf_arc("i2c read error\n");
    }
//	printf_arc("i2c read \n");

    return val;
}

extern void delay_ms(int ms);

void init_I2C()
{
	unsigned v,speed,reg;
	struct aml_i2c_reg_ctrl* ctrl;

	f_serial_puts("i2c init\n");


	//1. set pinmux
	v = readl(P_AO_RTI_PIN_MUX_REG);
	//master
	v |= ((1<<5)|(1<<6));
	v &= ~((1<<2)|(1<<24)|(1<<1)|(1<<23));
	//slave
	// v |= ((1<<1)|(1<<2)|(1<<3)|(1<<4));
	writel(v,P_AO_RTI_PIN_MUX_REG);
	udelay__(10000);


	reg = readl(P_AO_I2C_M_0_CONTROL_REG);
	reg &= 0xFFC00FFF;
	reg |= (I2C_RESUME_SPEED <<12);             // at 24MHz, i2c speed to 100KHz
	writel(reg,P_AO_I2C_M_0_CONTROL_REG);
//	delay_ms(20);
//	delay_ms(1);
	udelay__(1000);

    v = i2c_pmu_read_b(0x0000);                 // read version
    if (v == 0x00 || v == 0xff)
	{
        serial_put_hex(v, 8);
        f_serial_puts(" Error: I2C init failed!\n");
    }
	else
	{
		serial_put_hex(v, 8);
		f_serial_puts("Success.\n");
	}
}

void rn5t618_set_bits(unsigned char addr, unsigned char bit, unsigned char mask)
{
    unsigned char val;
    val = i2c_pmu_read_b(addr);
    val &= ~(mask);
    val |=  (bit & mask);
    i2c_pmu_write_b(addr, val);
}

#define power_off_vddio_ao28()  rn5t618_set_bits(0x0044, 0x00, 0x01)    // LDO1
#define power_on_vddio_ao28()   rn5t618_set_bits(0x0044, 0x01, 0x01)
#define power_off_vddio_ao18()  rn5t618_set_bits(0x0044, 0x00, 0x02)    // LDO2
#define power_on_vddio_ao18()   rn5t618_set_bits(0x0044, 0x02, 0x02)
#define power_off_vcc18()       rn5t618_set_bits(0x0044, 0x00, 0x04)    // LDO3
#define power_on_vcc18()        rn5t618_set_bits(0x0044, 0x04, 0x04)
#define power_off_vcc28()       rn5t618_set_bits(0x0044, 0x00, 0x08)    // LDO4
#define power_on_vcc28()        rn5t618_set_bits(0x0044, 0x08, 0x08)
#define power_off_avdd18()      rn5t618_set_bits(0x0044, 0x00, 0x10)    // LDO5
#define power_on_avdd18()       rn5t618_set_bits(0x0044, 0x10, 0x10)

#define LDO2_BIT                0x02
#define LDO3_BIT                0x04
#define LDO4_BIT                0x08
#define LDO5_BIT                0x10

#define MODE_PWM                1
#define MODE_PSM                2
#define MODE_AUTO               0

inline void power_off_ddr15() 
{
    rn5t618_set_bits(0x0030, 0x00, 0x01);    // DCDC3
}
inline void power_on_ddr15() 
{
    rn5t618_set_bits(0x0030, 0x01, 0x01);
}

int get_charging_state()
{
    unsigned char status;
    status = i2c_pmu_read_b(0x00bd);
    return (status & 0xc0) ? 1 : 0;
}

void rn5t618_shut_down()
{
    rn5t618_set_bits(0x000f, 0x00, 0x01);
    rn5t618_set_bits(0x000E, 0x01, 0x01);
    while (1);
}

int find_idx(int start, int target, int step, int size)
{
    int i = 0;  
    do { 
        if (start >= target) {
            break;    
        }    
        start += step;
        i++; 
    } while (i < size);
    return i;
}

void rn5t618_set_dcdc_voltage(int dcdc, int voltage)
{
    int addr;
    int idx_to;
    addr = 0x35 + dcdc;
    idx_to = find_idx(6000, voltage * 10, 125, 256);            // step is 12.5mV
    i2c_pmu_write_b(addr, idx_to);
#if 1
    printf_arc("set dcdc");
    serial_put_hex(addr-36, 4);
    wait_uart_empty();
    printf_arc(" to 0x");
    serial_put_hex(idx_to, 8);
    wait_uart_empty();
    printf_arc("\n");
#endif
}

void rn5t618_set_dcdc_mode(int dcdc, int mode)
{
    int addr = 0x2C + (dcdc - 1) * 2;
    unsigned char bits = (mode << 4) & 0xff; 

    rn5t618_set_bits(addr, bits, 0x30);
    udelay__(100);
}

void rn5t618_set_gpio(int gpio, int output)
{
    int val = output ? 1 : 0;
    if (gpio < 0 || gpio > 3) {
        return ;
    }
    rn5t618_set_bits(0x0091, val << gpio, 1 << gpio);       // set out put value
    rn5t618_set_bits(0x0090, 1 << gpio, 1 << gpio);         // set pin to output mode
    udelay__(100);
}

void rn5t618_get_gpio(int gpio, unsigned char *val)
{
    int value;
    if (gpio < 0 || gpio > 3) {
        return ;
    }
    value = i2c_pmu_read_b(0x0097);                         // read status
    *val = (value & (1 << gpio)) ? 1 : 0;
}

int pmu_get_battery_voltage(void)
{
    unsigned char val[2];
    int result;

    rn5t618_set_bits(0x0066, 0x01, 0x07);                   // select vbat channel
    udelay__(200);
    result = i2c_pmu_read_w(0x006A);
    val[0] = result & 0xff;
    val[1] = (result >> 8) & 0xff;
    result = (val[0] << 4) | (val[1] & 0x0f);
    result = (result * 5000) / 4096;                        // resolution: 1.221mV

    return result;
}

#ifdef CONFIG_RESET_TO_SYSTEM
void pmu_feed_watchdog(unsigned int flags)
{
    i2c_pmu_read_b(0x0b);
    i2c_pmu_write_b(0x0013, 0x00);
}
#endif /* CONFIG_RESET_TO_SYSTEM */

/*
 * use globle virable to fast i2c speed
 */
static unsigned char reg_ldo     = 0;
static unsigned char reg_ldo_rtc = 0;
static unsigned char charge_timeout = 0;

void rn5t618_power_off_at_24M()
{
//    printf_arc("%s\n", __func__);
    rn5t618_get_gpio(1, &vbus_status);
    if (!vbus_status) {
    //  rn5t618_set_gpio(1, 1);                                         // close boost
    }
	rn5t618_set_gpio(1, 1);                                             // close vccx2
    rn5t618_set_gpio(0, 1);                                             // close vccx3
    udelay__(2000);

#if defined(CONFIG_VDDAO_VOLTAGE_CHANGE)
#if CONFIG_VDDAO_VOLTAGE_CHANGE
    rn5t618_set_dcdc_voltage(2, CONFIG_VDDAO_SUSPEND_VOLTAGE);
#endif
#endif
#if defined(CONFIG_DCDC_PFM_PMW_SWITCH)
#if CONFIG_DCDC_PFM_PMW_SWITCH
    rn5t618_set_dcdc_mode(2, MODE_PSM);
    printf_arc("dc2 set to PSM\n");
    rn5t618_set_dcdc_mode(3, MODE_PSM);
    printf_arc("dc3 set to PSM\n");
#endif
#endif
    rn5t618_set_bits(0x0044, ~(LDO3_BIT | LDO4_BIT ), (LDO3_BIT | LDO4_BIT ));

    reg_ldo     = i2c_pmu_read_b(0x0044);
    reg_ldo_rtc = i2c_pmu_read_b(0x0045);
    rn5t618_set_bits(0x002c, 0x00, 0x01);               // close DCDC1, vcck 
    printf_arc("enter 32K\n");
}

void rn5t618_power_on_at_24M()          // need match power sequence of  power_off_at_24M
{
    printf_arc("enter 24MHz.\n");

    rn5t618_set_gpio(3, 1);                             // should open LDO1.2v before open VCCK
    udelay__(6 * 1000);                                // must delay 25 ms before open vcck
    rn5t618_set_bits(0x002c, 0x01, 0x01);               // open DCDC1, vcck 

	rn5t618_set_bits(0x0044, LDO3_BIT | LDO4_BIT, (LDO3_BIT | LDO4_BIT ));
	
#if defined(CONFIG_DCDC_PFM_PMW_SWITCH)
#if CONFIG_DCDC_PFM_PMW_SWITCH
    rn5t618_set_dcdc_mode(3, MODE_PWM);
    printf_arc("dc3 set to pwm\n");
    rn5t618_set_dcdc_mode(2, MODE_PWM);
    printf_arc("dc2 set to pwm\n");
#endif
#endif
#if defined(CONFIG_VDDAO_VOLTAGE_CHANGE)
#if CONFIG_VDDAO_VOLTAGE_CHANGE
    rn5t618_set_dcdc_voltage(2, CONFIG_VDDAO_VOLTAGE);
#endif
#endif
	rn5t618_set_gpio(0, 0);                                     // need to open this in LCD driver?
	rn5t618_set_gpio(1, 0);                                     // close vccx2
    if (!vbus_status) {
        //rn5t618_set_gpio(1, 0);
    }
    if (charge_timeout & 0x20) {
        printf_arc("charge timeout detect, reset charger\n");
        rn5t618_set_bits(0x00C5, 0x00, 0x20);                   // clear flag
        rn5t618_set_bits(0x00B3, 0x00, 0x03);
        udelay__(100 * 1000);
        rn5t618_set_bits(0x00B3, 0x03, 0x03);
    }
    rn5t618_set_gpio(3, 0);                             // close ldo 1.2v when vcck is opened 
}

void rn5t618_power_off_at_32K_1()
{
//return;
    unsigned int reg;                               // change i2c speed to 1KHz under 32KHz cpu clock
    unsigned int sleep_flag = readl(P_AO_RTI_STATUS_REG2);

	reg  = readl(P_AO_I2C_M_0_CONTROL_REG);
	reg &= 0xFFC00FFF;
//    if (sleep_flag == 0x87654321) {                 // suspended in uboot
//        reg |= (12) << 12;
//    } else {
	if(readl(P_AO_RTI_STATUS_REG2) == 0x87654321)
    	reg |= (10 << 12);           // suspended from kernel
    else
		reg |= (5 << 12);           // suspended from kernel
//    }
	writel(reg,P_AO_I2C_M_0_CONTROL_REG);
	udelay__(100);

    reg_ldo &= ~(LDO5_BIT);
    i2c_pmu_write_b(0x0044, reg_ldo);                   // close LDO5, AVDD1.8

    reg_ldo_rtc &= ~(0x10);
    i2c_pmu_write_b(0x0045, reg_ldo_rtc);               // close ext DCDC 3.3v

//	rn5t618_set_bits(0x0044, ~LDO2_BIT, LDO2_BIT);//need power off lastly

#ifdef CONFIG_RESET_TO_SYSTEM
    i2c_pmu_write_b(0x000b, 0x00);                      // disable watchdog 
    i2c_pmu_write_b(0x0012, 0x00);                      // disable watchdog
    i2c_pmu_write_b(0x0012, 0x40);                      // enable watchdog
    i2c_pmu_write_b(0x000b, 0x0c);                      // time out to 1s
#endif
}

void rn5t618_power_on_at_32K_1()        // need match power sequence of  power_off_at_32K_1
{
//	return;
    unsigned int    reg;

//	rn5t618_set_bits(0x0044, LDO2_BIT, LDO2_BIT);  //need power on firstly
    reg_ldo_rtc |= 0x10;
    i2c_pmu_write_b(0x0045, reg_ldo_rtc);               // open ext DCDC 3.3v

#ifdef CONFIG_RESET_TO_SYSTEM
    pmu_feed_watchdog(0);
    i2c_pmu_write_b(0x000b, 0x01);                      // disable watchdog 
    i2c_pmu_write_b(0x0012, 0x00);                      // disable watchdog
    i2c_pmu_write_b(0x000b, 0x0d);                      // set timeout to 8s 
    i2c_pmu_write_b(0x0012, 0x40);                      // enable watchdog 
#endif

    reg_ldo |= (LDO5_BIT);
    i2c_pmu_write_b(0x0044, reg_ldo);                   // open LDO5, AVDD1.8

	reg  = readl(P_AO_I2C_M_0_CONTROL_REG);
	reg &= 0xFFC00FFF;
	reg |= (I2C_RESUME_SPEED << 12);
	writel(reg,P_AO_I2C_M_0_CONTROL_REG);
	udelay__(100);
	
}

void rn5t618_power_off_at_32K_2()       // If has nothing to do, just let null
{
    // TODO: add code here
}

void rn5t618_power_on_at_32K_2()        // need match power sequence of power_off_at_32k
{
    // TODO: add code here
}

unsigned int rn5t618_detect_key(unsigned int flags)
{
    int delay_cnt   = 0;
    int power_status;
    int prev_status;
    int battery_voltage;
    int ret = 0;

#ifdef CONFIG_IR_REMOTE_WAKEUP
    //backup the remote config (on arm)
    backup_remote_register();
    //set the ir_remote to 32k mode at ARC
    init_custom_trigger();
#endif

    writel(readl(P_AO_GPIO_O_EN_N)|(1 << 3),P_AO_GPIO_O_EN_N);
    writel(readl(P_AO_RTI_PULL_UP_REG)|(1 << 3)|(1<<19),P_AO_RTI_PULL_UP_REG);

	prev_status = get_charging_state();
    do {
        /*
         * when extern power status has changed, we need break 
         * suspend loop and resume system.
         */
	    power_status = get_charging_state();
        if (power_status ^ prev_status) {
            if (flags == 0x87654321) {      // suspend from uboot
                ret = 1;
            }
            break;
        }
        delay_cnt++;

    #ifdef CONFIG_RESET_TO_SYSTEM
        pmu_feed_watchdog(flags);
    #endif
        if (delay_cnt >= 3000) {
            if (flags != 0x87654321 && !power_status) {
                /*
                 * when battery voltage is too low but no power supply and suspended from kernel,
                 * we need to break suspend loop to resume system, then system will shut down
                 */
                battery_voltage = pmu_get_battery_voltage();
                if (battery_voltage < 3480) {
                    break;
                }
            } else if (power_status) {
                charge_timeout = i2c_pmu_read_b(0x00c5);
                if (charge_timeout & 0x20) {
                    break;    
                } else {
                    charge_timeout = 0;    
                }
            }
            delay_cnt = 0;
        }

#ifdef CONFIG_IR_REMOTE_WAKEUP
        if(remote_detect_key()){
        	break;
        }
#endif

	    if((readl(P_AO_RTC_ADDR1) >> 12) & 0x1)
		    break;
	
    } while (readl(P_AO_GPIO_I)&(1<<3));            // power key

#ifdef CONFIG_IR_REMOTE_WAKEUP
	resume_remote_register();
#endif

    return ret;
}

void arc_pwr_register(struct arc_pwr_op *pwr_op)
{
//    printf_arc("%s\n", __func__);
	pwr_op->power_off_at_24M    = rn5t618_power_off_at_24M;
	pwr_op->power_on_at_24M     = rn5t618_power_on_at_24M;

	pwr_op->power_off_at_32K_1  = rn5t618_power_off_at_32K_1;
	pwr_op->power_on_at_32K_1   = rn5t618_power_on_at_32K_1;
	pwr_op->power_off_at_32K_2  = rn5t618_power_off_at_32K_2;
	pwr_op->power_on_at_32K_2   = rn5t618_power_on_at_32K_2;

	pwr_op->power_off_ddr15     = 0;//rn5t618_power_off_ddr15;
	pwr_op->power_on_ddr15      = 0;//rn5t618_power_on_ddr15;

	pwr_op->shut_down           = rn5t618_shut_down;

	pwr_op->detect_key          = rn5t618_detect_key;
}

