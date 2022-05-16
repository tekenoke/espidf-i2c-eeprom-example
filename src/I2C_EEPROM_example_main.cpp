#include <stdio.h>
#include "driver/i2c.h"

#define I2C_MASTER_SCL_IO           22		// I2C master SCK
#define I2C_MASTER_SDA_IO           21      // I2C master SDA
#define I2C_MASTER_NUM              0		// I2C module Number
#define I2C_MASTER_FREQ_HZ          400000	// I2C master freq
#define I2C_MASTER_TX_BUF_DISABLE   0		// I2C master doesn't need buffer
#define I2C_MASTER_RX_BUF_DISABLE   0		// I2C master doesn't need buffer

#define	E24AA01_ADDR	0xA0				// 0xA0
#define ACK_CHECK_EN 0x1					// I2C master will check ack from slave
#define ACK_CHECK_DIS 0x0					// I2C master will not check ack from slave

// E2P random write 1byte
void	i2c_eep_write(int addr, unsigned char dt) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, E24AA01_ADDR, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, addr, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, dt, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000);	// 1000ms timeout
    i2c_cmd_link_delete(cmd);
	vTaskDelay(5);
}

// E2P random read 1byte
unsigned char i2c_eep_read(int addr) {
	unsigned char ch;

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, E24AA01_ADDR, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, addr, ACK_CHECK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, E24AA01_ADDR | 0x1, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, &ch, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000);
    i2c_cmd_link_delete(cmd);
	return ch;
}

// I2C initial
void	i2c_master_init(void) {
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = I2C_MASTER_SDA_IO;
    conf.scl_io_num = I2C_MASTER_SCL_IO;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = (uint32_t)I2C_MASTER_FREQ_HZ;  // select frequency specific to your project
	conf.clk_flags = 0;
    i2c_param_config(i2c_master_port, &conf);
    i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

// E2PROM marching test
extern "C" void app_main() {
	printf("\r\n");
    i2c_master_init();
	for(int i=0;i<128;i++) {		// 0xaaを書き込む
		i2c_eep_write(i,0xaa);
	} 
	for(int i=0;i<128;i++) {
		if(i2c_eep_read(i)==0xaa) {	// 読出し0xaaならそのアドレスに0x55を書く
			i2c_eep_write(i,0x55);
		} else {					// 読出し≠0xaaならエラー
			printf("NG %d\r\n",i);
			for(;;) vTaskDelay(100);
		}
	}
	printf("OK\n\r");
	for(;;) vTaskDelay(100);
}
