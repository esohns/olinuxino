/*  I2C kernel module driver for the Olimex MOD-MPU6050 UEXT module
    (see https://www.olimex.com/Products/Modules/Sensors/MOD-MPU6050/open-source-hardware,
         http://www.invensense.com/mems/gyro/mpu6050.html)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/kobject.h>
#include <linux/pinctrl/consumer.h>
#include <linux/spinlock.h>
#include <linux/workqueue.h>

#include <plat/sys_config.h>

// defines
#define KO_OLIMEX_MOD_MPU6050_LICENSE         "GPL"
#define KO_OLIMEX_MOD_MPU6050_AUTHOR          "Erik Sohns"
#define KO_OLIMEX_MOD_MPU6050_VERSION         "0.1"
#define KO_OLIMEX_MOD_MPU6050_DRIVER_NAME     "olimex_mod_mpu6050"
#define KO_OLIMEX_MOD_MPU6050_DESCRIPTION     "I2C kernel module driver for the Olimex MOD-MPU6050 UEXT module"

// defaults
#define BUFSIZ                                256

// macros
#define ARRAY_AND_SIZE(x)	                    (x), ARRAY_SIZE(x)

// *NOTE*: check the .fex file (bin2fex of script.bin) in the device boot partition
#define GPIO_UEXT4_UART4RX_PG11_PIN           10
#define GPIO_UEXT4_UART4RX_PG11_LABEL         "gpio_pin_10"
#define GPIO_LED_PH02_PIN                     20
#define GPIO_LED_PH02_LABEL                   "gpio_pin_20"

#define FIFOSTORESIZE                         20
#define FIFOSTOREDATASIZE                     64
#define RINGBUFFERSIZE                        20
#define RINGBUFFERDATASIZE                    64

// types
struct fifostoreentry_t {
  uint8_t data[FIFOSTOREDATASIZE];
  int size;
};

struct ringbufferentry_t {
  int used;
  int completed;
  uint8_t data[RINGBUFFERDATASIZE];
  int size;
};

struct read_work_t {
  struct work_struct work;
  struct i2c_client* client;
};
struct fifo_work_t {
  struct work_struct work;
  struct i2c_client* client;
};

struct i2c_mpu6050_client_data_t {
  struct pinctrl* pin_ctrl;
  struct pinctrl_state* pin_ctrl_state;
  script_gpio_set_t gpio_int_data;
  script_gpio_set_t gpio_led_data;
  unsigned gpio_led_handle;
  struct i2c_client* client;
  struct kobject* object; // used for the sysfs entries
  struct workqueue_struct* workqueue;
  struct fifo_work_t work_processfifostore;
  struct read_work_t work_read;
  struct fifostoreentry_t fifostore[FIFOSTORESIZE];
  int fifostorepos;
  struct ringbufferentry_t ringbuffer[RINGBUFFERSIZE];
  int ringbufferpos;
  spinlock_t sync_lock; // disable interrupts while i2c_sync() is running
  unsigned long sync_lock_flags;
};

// function declarations
// sysfs callbacks
static ssize_t i2c_mpu6050_store_store(struct kobject*, struct kobj_attribute*, const char*, size_t);
static ssize_t i2c_mpu6050_store_show(struct kobject*, struct kobj_attribute*, char*);

// workqueue callbacks
static void i2c_mpu6050_workqueue_fifo_handler(struct work_struct*);
static void i2c_mpu6050_workqueue_read_handler(struct work_struct*);

static irqreturn_t i2c_mpu6050_interrupt_handler(int, void*);

static void i2c_mpu6050_clearringbuffer(void*);

static ssize_t i2c_mpu6050_reg_show(struct kobject*, struct kobj_attribute*, char*);
static ssize_t i2c_mpu6050_reg_store(struct kobject*, struct kobj_attribute*, const char*, size_t);
static ssize_t i2c_mpu6050_clearringbuffer_store(struct kobject*, struct kobj_attribute*, const char*, size_t);

static int i2c_mpu6050_pm_prepare(struct device*);
static void i2c_mpu6050_pm_complete(struct device*);
static int i2c_mpu6050_pm_suspend(struct device*);
static int i2c_mpu6050_pm_suspend_late(struct device*);
static int i2c_mpu6050_pm_resume(struct device*);
static int i2c_mpu6050_pm_resume_early(struct device*);
static int i2c_mpu6050_pm_freeze(struct device*);
static int i2c_mpu6050_pm_freeze_late(struct device*);
static int i2c_mpu6050_pm_thaw(struct device*);
static int i2c_mpu6050_pm_thaw_early(struct device*);
static int i2c_mpu6050_pm_poweroff(struct device*);
static int i2c_mpu6050_pm_poweroff_late(struct device*);
static int i2c_mpu6050_pm_restore(struct device*);
static int i2c_mpu6050_pm_restore_early(struct device*);
//
static int i2c_mpu6050_pm_suspend_noirq(struct device*);
static int i2c_mpu6050_pm_resume_noirq(struct device*);
static int i2c_mpu6050_pm_freeze_noirq(struct device*);
static int i2c_mpu6050_pm_thaw_noirq(struct device*);
static int i2c_mpu6050_pm_poweroff_noirq(struct device*);
static int i2c_mpu6050_pm_restore_noirq(struct device*);
//
static int i2c_mpu6050_pm_runtime_suspend(struct device*);
static int i2c_mpu6050_pm_runtime_resume(struct device*);
static int i2c_mpu6050_pm_runtime_idle(struct device*);

static int i2c_mpu6050_attach_adapter(struct i2c_adapter*);
static int i2c_mpu6050_detach_adapter(struct i2c_adapter*);
static int i2c_mpu6050_probe(struct i2c_client*, const struct i2c_device_id*);
static int i2c_mpu6050_remove(struct i2c_client*);
static void i2c_mpu6050_shutdown(struct i2c_client*);
static int i2c_mpu6050_suspend(struct i2c_client*, pm_message_t);
static int i2c_mpu6050_resume(struct i2c_client*);

static void i2c_mpu6050_alert(struct i2c_client*, unsigned int);

static int i2c_mpu6050_command(struct i2c_client*, unsigned int, void*);

static int i2c_mpu6050_detect(struct i2c_client*, struct i2c_board_info*);

//static int chip_detect(struct i2c_adapter*, int, int);

static int i2c_mpu6050_init(void);
static void i2c_mpu6050_exit(void);
