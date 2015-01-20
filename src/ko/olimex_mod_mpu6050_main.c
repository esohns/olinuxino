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

#include "olimex_mod_mpu6050.h"

//#include <linux/device.h>
//#include <linux/gpio.h>
//#include <linux/irq.h>
//#include <linux/kernel.h>
//#include <linux/pinctrl/consumer.h>
//#include <linux/printk.h>
//#include <linux/regmap.h>
//#include <linux/sched.h>
//#include <linux/slab.h>
//#include <linux/sysfs.h>

//// *NOTE*: taken from i2cdevlib (see also: http://www.i2cdevlib.com/devices/mpu6050#source)
//#include "MPU6050.h"

// *** irq handlers ***
static
irqreturn_t
i2c_mpu6050_interrupt_handler(int irq_in,
                              void* dev_id_in)
{
  struct device* device_p = NULL;
  struct i2c_client* client_p = NULL;
  struct i2c_mpu6050_client_data_t* client_data_p = NULL;
//  enum { falling, rising } type;
  u32 type;

  // sanity check(s)
  device_p = (struct device*)dev_id_in;
  if (!device_p) {
    printk(KERN_ERR "unable to retrieve client state\n");
    return IRQ_NONE;
  }
  client_p = to_i2c_client(device_p);
  if (!client_p) {
    printk(KERN_ERR "unable to retrieve client state\n");
    return IRQ_NONE;
  }
  client_data_p = (struct i2c_mpu6050_client_data_t*)i2c_get_clientdata(client_p);
  if (!client_data_p) {
    printk(KERN_ERR "unable to retrieve client state\n");
    return IRQ_NONE;
  }

  printk(KERN_DEBUG "interrupt received (irq: %d)\n", irq_in);

  if (irq_in == client_p->irq) {
    type = irqd_get_trigger_type(irq_get_irq_data(irq_in));
//    type = (gpio_get_value(irq_to_gpio(irq_in)) ? rising : falling);
//    if (type == rising) {
    if ((type & IRQ_TYPE_SENSE_MASK) == IRQ_TYPE_EDGE_RISING) {
      printk(KERN_DEBUG "rising GPIO interrupt received, queueing work READ\n");

      gpio_write_one_pin_value(client_data_p->gpio_led_handle, 1, GPIO_LED_PH02_LABEL);
      queue_work(client_data_p->workqueue, &client_data_p->work_read.work);
      gpio_write_one_pin_value(client_data_p->gpio_led_handle, 0, GPIO_LED_PH02_LABEL);
      return IRQ_HANDLED;
    }
  }

  return IRQ_NONE;
}

// *** driver ***
static
int
i2c_mpu6050_attach_adapter(struct i2c_adapter* adapter_in)
{
  printk(KERN_DEBUG "i2c_mpu6050_attach_adapter(%d) called.\n",
         i2c_adapter_id(adapter_in));

//  return i2c_detect(adapter_in, &addr_data,
//                    chip_detect);
  return 0;
}

static
int
i2c_mpu6050_detach_adapter(struct i2c_adapter* adapter_in)
{
  printk(KERN_DEBUG "i2c_mpu6050_detach_adapter(%d) called.\n",
         i2c_adapter_id(adapter_in));

  return 0;
}

static
int
__devinit i2c_mpu6050_probe(struct i2c_client* client_in,
                            const struct i2c_device_id* id_in)
{
  int err, gpio_used;
  struct i2c_client* client_p;
  struct i2c_mpu6050_client_data_t* client_data_p;
//  struct gpio_chip* gpio_chip_p;

  printk(KERN_DEBUG "i2c_mpu6050_probe() called.\n");

  // sanity check(s)
  if (!client_in)
  {
    pr_err("%s: invalid argument\n", __FUNCTION__);
    return -ENOSYS;
  }
  if (!i2c_check_functionality(client_in->adapter,
                               (I2C_FUNC_SMBUS_BYTE_DATA |
                                I2C_FUNC_SMBUS_WORD_DATA |
                                I2C_FUNC_SMBUS_I2C_BLOCK))) {
    pr_err("%s: needed i2c functionality is not supported\n", __FUNCTION__);
    return -ENODEV;
  }
  client_p = i2c_new_probed_device(client_in->adapter,
                                   &i2c_mpu6050_board_infos[0],
                                   normal_i2c,
                                   NULL);
  if (IS_ERR(client_p)) {
    pr_err("%s: i2c_new_probed_device() failed\n", __FUNCTION__);
    return PTR_ERR(client_p);
  }

  client_data_p = kzalloc(sizeof(struct i2c_mpu6050_client_data_t), GFP_KERNEL);
  if (IS_ERR(client_data_p)) {
    pr_err("%s: kzalloc() failed\n", __FUNCTION__);
    return PTR_ERR(client_data_p);
  }
  client_data_p->client = client_in;
  i2c_set_clientdata(client_in, client_data_p);

  err = script_parser_fetch("gpio_para",
                            "gpio_used",
                            &gpio_used,
                            sizeof(gpio_used)/sizeof(int));
  if (err) {
    pr_err("%s: script_parser_fetch(\"gpio_para\",\"gpio_used\") failed\n", __FUNCTION__);
    goto error1;
  }
  err = script_parser_fetch("gpio_para",
                            GPIO_UEXT4_UART4RX_PG11_LABEL,
                            (int*)&client_data_p->gpio_int_data,
                            sizeof(script_gpio_set_t));
  if (err) {
    pr_err("%s: script_parser_fetch(\"gpio_para\",\"%s\") failed\n", __FUNCTION__,
           GPIO_UEXT4_UART4RX_PG11_LABEL);
    goto error1;
  }
  err = script_parser_fetch("gpio_para",
                            GPIO_LED_PH02_LABEL,
                            (int*)&client_data_p->gpio_led_data,
                            sizeof(script_gpio_set_t));
  if (err) {
    pr_err("%s: script_parser_fetch(\"gpio_para\",\"%s\") failed\n", __FUNCTION__,
           GPIO_LED_PH02_LABEL);
    goto error1;
  }

  if (!i2c_mpu6050_sysfs_init(client_data_p)) {
    pr_err("%s: i2c_mpu6050_sysfs_init() failed\n", __FUNCTION__);
    goto error1;
  }

  if (!i2c_mpu6050_wq_init(client_data_p)) {
    pr_err("%s: i2c_mpu6050_wq_init() failed\n", __FUNCTION__);
    goto error2;
  }

  i2c_mpu6050_clearringbuffer(client_data_p);
  spin_lock_init(&client_data_p->sync_lock);

  // initialize interrupt for GPIO INT line
//  client_data_p->pin_ctrl = devm_pinctrl_get(&client_in->dev);
  client_data_p->pin_ctrl = pinctrl_get(&client_in->dev);
  if (IS_ERR(client_data_p->pin_ctrl)) {
    pr_err("%s: pinctrl_get() failed: %d\n", __FUNCTION__,
           PTR_ERR(client_data_p->pin_ctrl));
    goto error3;
  }
  client_data_p->pin_ctrl_state = pinctrl_lookup_state(client_data_p->pin_ctrl,
                                                       PINCTRL_STATE_DEFAULT);
  if (IS_ERR(client_data_p->pin_ctrl_state)) {
    pr_err("%s: pinctrl_lookup_state() failed: %d\n", __FUNCTION__,
           PTR_ERR(client_data_p->pin_ctrl_state));
    goto error4;
  }
  err = pinctrl_select_state(client_data_p->pin_ctrl,
                             client_data_p->pin_ctrl_state);
  if (err < 0) {
    pr_err("%s: pinctrl_select_state() failed: %d\n", __FUNCTION__,
           err);
    goto error4;
  }
  err = gpio_is_valid(GPIO_UEXT4_UART4RX_PG11_PIN);
  if (err) {
    pr_err("%s: gpio_is_valid(%d) failed\n", __FUNCTION__,
           GPIO_UEXT4_UART4RX_PG11_PIN);
    goto error4;
  }
  err = gpio_is_valid(GPIO_LED_PH02_PIN);
  if (err) {
    pr_err("%s: gpio_is_valid(%d) failed\n", __FUNCTION__,
           GPIO_LED_PH02_PIN);
    goto error4;
  }
  err = devm_gpio_request(&client_in->dev,
                          GPIO_UEXT4_UART4RX_PG11_PIN,
                          GPIO_UEXT4_UART4RX_PG11_LABEL);
  if (err < 0) {
    pr_err("%s: devm_gpio_request(%d) failed\n", __FUNCTION__,
           GPIO_UEXT4_UART4RX_PG11_PIN);
    goto error4;
  }
//  err = gpio_request_one(GPIO_UEXT4_UART4RX_PG11_PIN,
//                         GPIOF_DIR_IN,
//                         GPIO_UEXT4_UART4RX_PG11_LABEL);
//  if (err) {
//    pr_err("%s: gpio_request_one(%d) failed\n", __FUNCTION__,
//           GPIO_UEXT4_UART4RX_PG11_PIN);
//    goto error4;
//  }
  client_data_p->gpio_led_handle = gpio_request_ex("gpio_para",
                                                   GPIO_LED_PH02_LABEL);
  if (client_data_p->gpio_led_handle < 0) {
    pr_err("%s: gpio_request_ex(%s) failed\n", __FUNCTION__,
           GPIO_LED_PH02_LABEL);
    goto error5;
  }
  // export GPIOs to userspace
  err = gpio_export(GPIO_UEXT4_UART4RX_PG11_PIN,
                    false);
  if (err) {
    pr_err("%s: gpio_export(%d) failed\n", __FUNCTION__,
           GPIO_UEXT4_UART4RX_PG11_PIN);
    goto error6;
  }
  err = gpio_export(GPIO_LED_PH02_PIN,
                    false);
  if (err) {
    pr_err("%s: gpio_export(%d) failed\n", __FUNCTION__,
           GPIO_LED_PH02_PIN);
    goto error7;
  }
  err = gpio_export_link(&client_data_p->client->dev,
                         GPIO_UEXT4_UART4RX_PG11_LABEL,
                         GPIO_UEXT4_UART4RX_PG11_PIN);
  if (err) {
    pr_err("%s: gpio_export_link(%s,%d) failed\n", __FUNCTION__,
           GPIO_UEXT4_UART4RX_PG11_LABEL,
           GPIO_UEXT4_UART4RX_PG11_PIN);
    goto error8;
  }
  client_data_p->client->irq = gpio_to_irq(GPIO_UEXT4_UART4RX_PG11_PIN);
  if (client_data_p->client->irq < 0) {
    pr_err("%s: gpio_to_irq(%d) failed: %d\n", __FUNCTION__,
           GPIO_UEXT4_UART4RX_PG11_PIN,
           client_data_p->client->irq);
    goto error8;
  }
  pr_inf("%s: GPIO %s --> PIN %d --> IRQ %d\n", __FUNCTION__,
         GPIO_UEXT4_UART4RX_PG11_LABEL,
         GPIO_UEXT4_UART4RX_PG11_PIN,
         client_data_p->client->irq);
//  gpio_chip_p = gpio_to_chip(GPIO_UEXT4_UART4RX_PG11_PIN);
//  if (!IS_ERR(gpio_chip_p)) {
//    pr_err("%s: gpio_to_chip(%d) failed: %d\n", __FUNCTION__,
//           GPIO_UEXT4_UART4RX_PG11_PIN,
//           PTR_ERR(gpio_chip_p));
//    goto error8;
//  }
//  err = gpio_lock_as_irq(gpio_chip_p,
//                         GPIO_UEXT4_UART4RX_PG11_PIN);
//  if (err) {
//    pr_err("%s: gpio_lock_as_irq(%d) failed\n", __FUNCTION__,
//           GPIO_UEXT4_UART4RX_PG11_PIN);
//    goto error8;
//  }
  err = request_irq(client_data_p->client->irq,
                    i2c_mpu6050_interrupt_handler,
                    (IRQ_TYPE_EDGE_RISING),
                    GPIO_UEXT4_UART4RX_PG11_LABEL,
                    &client_data_p->client->dev);
  if (err) {
    pr_err("%s: request_irq(%d) failed: %d\n", __FUNCTION__,
           client_data_p->client->irq,
           err);
    goto error8;
  }

  // debug info
  dev_info(&client_data_p->client->dev,
           "%s created\n",
           KO_OLIMEX_MOD_MPU6050_DRIVER_NAME);

  return 0;

error8:
  gpio_unexport(GPIO_LED_PH02_PIN);
error7:
  gpio_unexport(GPIO_UEXT4_UART4RX_PG11_PIN);
error6:
  gpio_release(client_data_p->gpio_led_handle, 1);
error5:
  devm_gpio_free(&client_in->dev,
                 GPIO_UEXT4_UART4RX_PG11_PIN);
error4:
//  devm_pinctrl_put(client_data_p->pin_ctrl);
  pinctrl_put(client_data_p->pin_ctrl);
error3:
  i2c_mpu6050_wq_fini(client_data_p);
error2:
  i2c_mpu6050_sysfs_fini(client_data_p);
error1:
  kfree(client_data_p);

  return -ENOSYS;
}

static
int
i2c_mpu6050_remove(struct i2c_client* client_in)
{
  struct i2c_mpu6050_client_data_t* client_data_p = NULL;
//  struct gpio_chip* gpio_chip_p;

  printk(KERN_DEBUG "i2c_mpu6050_remove() called.\n");

  // sanity check(s)
  if (!client_in) {
    pr_err("%s: invalid argument\n", __FUNCTION__);
    return -ENOSYS;
  }
  client_data_p = (struct i2c_mpu6050_client_data_t*)i2c_get_clientdata(client_in);
  if (IS_ERR(client_data_p)) {
    pr_err("%s: i2c_get_clientdata() failed: %d\n", __FUNCTION__,
           PTR_ERR(client_data_p));
    return -ENOSYS;
  }
//  gpio_chip_p = gpio_to_chip(GPIO_UEXT4_UART4RX_PG11_PIN);
//  if (IS_ERR(gpio_chip_p)) {
//    pr_err("%s: gpio_to_chip(%d) failed: %d\n", __FUNCTION__,
//           GPIO_UEXT4_UART4RX_PG11_PIN,
//           PTR_ERR(gpio_chip_p));
//    return -ENOSYS;
//  }

//  gpiochip_unlock_as_irq(gpio_chip_p,
//                         GPIO_UEXT4_UART4RX_PG11_PIN);
  free_irq(client_data_p->client->irq, NULL);
  gpio_unexport(GPIO_LED_PH02_PIN);
  gpio_unexport(GPIO_UEXT4_UART4RX_PG11_PIN);
  gpio_release(client_data_p->gpio_led_handle, 1);
  devm_gpio_free(&client_in->dev,
                 GPIO_UEXT4_UART4RX_PG11_PIN);
//  devm_pinctrl_put(client_data_p->pin_ctrl);
  pinctrl_put(client_data_p->pin_ctrl);
  i2c_mpu6050_wq_fini(client_data_p);
  i2c_mpu6050_sysfs_fini(client_data_p);
  kfree(client_data_p);

  return 0;
}

static
void
i2c_mpu6050_shutdown(struct i2c_client* client_in)
{
  printk(KERN_DEBUG "i2c_mpu6050_shutdown() called.\n");

}
static
int
i2c_mpu6050_suspend(struct i2c_client* client_in, pm_message_t message_in)
{
  printk(KERN_DEBUG "i2c_mpu6050_suspend() called.\n");

  return 0;
}
static
int
i2c_mpu6050_resume(struct i2c_client* client_in)
{
  printk(KERN_DEBUG "i2c_mpu6050_resume() called.\n");

  return 0;
}

static
void
i2c_mpu6050_alert(struct i2c_client* client_in, unsigned int data_in)
{
  printk(KERN_DEBUG "i2c_mpu6050_alert() called.\n");

}

static
int
i2c_mpu6050_command(struct i2c_client* client_in, unsigned int command_in, void* data_in)
{
  printk(KERN_DEBUG "i2c_mpu6050_command() called.\n");

  return 0;
}

static
int
i2c_mpu6050_detect(struct i2c_client* client_in, struct i2c_board_info* info_in)
{
  printk(KERN_DEBUG "i2c_mpu6050_detect(%s) called.\n",
         info_in->type);

  // *NOTE*: return 0 for supported, -ENODEV for unsupported devices
  return  ((strcmp(info_in->type,
                   KO_OLIMEX_MOD_MPU6050_DRIVER_NAME) == 0) ? 0
                                                            : -ENODEV);
}

//module_i2c_driver(i2c_mpu6050_driver);

//static
//int
//chip_detect(struct i2c_adapter* adapter_in, int address_in, int kind_in)
//{
//  struct i2c_client* client_p;
//  struct i2c_mpu6050_client_data_t* client_data_p;
//  int err = 0;

//  client_p = kmalloc(sizeof(*client_p), GFP_KERNEL);
//  if (!client_p) {
//    printk(KERN_ERR "%s: failed to kmalloc\n", __func__);
//    return -ENOMEM;
//  }
//  memset(client_p, 0, sizeof(*client_p));
//  client_data_p = kmalloc(sizeof(*client_data_p), GFP_KERNEL);
//  if (!client_data_p) {
//    printk(KERN_ERR "%s: failed to kmalloc\n", __func__);

//    // clean up
//    kfree(client_p);

//    return -ENOMEM;
//  }
//  memset(client_data_p, 0, sizeof(*client_data_p));

//  i2c_set_clientdata(client_p, client_data_p);
//  client_p->addr = address_in;
//  client_p->adapter = adapter_in;
//  client_p->driver = &i2c_mpu6050_i2c_driver;
//  client_p->flags = 0;
//  strncpy(client_p->name, KO_OLIMEX_MOD_MPU6050_DRIVER_NAME,
//          I2C_NAME_SIZE);

//  /* Tell the I2C layer a new client has arrived */
//  err = i2c_attach_client(client_p);
//  if (IS_ERR(err))
//  {
//    printk(KERN_ERR "failed to i2c_attach_client: %d\n", err);

//    // clean up
//    kfree(client_p);
//    kfree(client_data_p);

//    return -ENODEV;
//  }

//  return 0;
//}

static
int
__init i2c_mpu6050_init(void)
{
  char buffer[BUFSIZ];
  int err;

  INIT_LIST_HEAD(&i2c_mpu6050_i2c_driver.clients);

  // registering I2C driver, this will call i2c_mpu6050_probe()
  err = i2c_add_driver(&i2c_mpu6050_i2c_driver);
  if (err < 0) {
    pr_err("%s: i2c_add_driver() failed: %d\n", __FUNCTION__,
           err);
    return -ENOSYS;
  }

//  i2c_mpu6050_board_infos[0].irq = gpio_to_irq(GPIO_UEXT4_UART4RX_PG11_PIN);
//  if (i2c_mpu6050_board_infos[0].irq < 0) {
//    pr_err("%s: gpio_to_irq(%d) failed: %d\n", __FUNCTION__,
//           GPIO_UEXT4_UART4RX_PG11_PIN,
//           err);
//    goto init_error1;
//  }
//  err = i2c_register_board_info(0,
//                                ARRAY_AND_SIZE(i2c_mpu6050_board_infos));
//  if (err < 0) {
//    pr_err("%s: i2c_register_board_info() failed: %d\n", __FUNCTION__,
//           err);
//    goto init_error1;
//  }

  memset(buffer, 0, sizeof(buffer));
  strcpy(buffer, KO_OLIMEX_MOD_MPU6050_DRIVER_NAME);
  strcpy(buffer + sizeof(KO_OLIMEX_MOD_MPU6050_DRIVER_NAME), "...added\n");
  printk(KERN_INFO "%s", buffer);

  return 0;

init_error1:
  i2c_del_driver(&i2c_mpu6050_i2c_driver);

  return -ENOSYS;
}

static
void
__exit i2c_mpu6050_exit(void)
{
  char buffer[BUFSIZ];

  i2c_del_driver(&i2c_mpu6050_i2c_driver);

  memset(buffer, 0, sizeof(buffer));
  strcpy(buffer, KO_OLIMEX_MOD_MPU6050_DRIVER_NAME);
  strcpy(buffer + sizeof(KO_OLIMEX_MOD_MPU6050_DRIVER_NAME), "...removed\n");
  printk(KERN_INFO "%s", buffer);
}

module_init(i2c_mpu6050_init);
module_exit(i2c_mpu6050_exit);