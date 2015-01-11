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

#include "olinuxino_mod_mpu6050.h"

// *NOTE*: taken from i2cdevlib (see also: http://www.i2cdevlib.com/devices/mpu6050#source)
#include "MPU6050.h"

#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/sched.h>
#include <linux/sysfs.h>

// *** sysfs ***
// this function is called when writing to the "store" sysfs file
static
ssize_t
i2c_mpu6050_store_store(struct kobject* kobj_in,
                        struct kobj_attribute* attr_in,
                        const char* buf_in,
                        size_t count_in)
{
  struct i2c_mpu6050_client_data_t* client_data_p = NULL;

  // sanity check(s)
//  struct device* device_p = kobj_to_dev(kobj_in);
////  struct kobj_type* ktype = get_ktype(kobj_in);
////  if (ktype == &device_ktype)
////    device_p = to_dev(kobj_in);
//  if (!device_p) {
//    printk(KERN_ERR "%s: invalid parameter (not a device ?)\n", __func__);
//    return -ENODEV;
//  }
//  struct i2c_client* client_p = to_i2c_client(device_p);
  struct i2c_client* client_p = kobj_to_i2c_client(kobj_in);
  if (!client_p) {
    printk(KERN_ERR "%s: invalid parameter (not a I2C device ?)\n", __func__);
    return -ENODEV;
  }
  client_data_p = (struct i2c_mpu6050_client_data_t*)i2c_get_clientdata(client_p);
  if (!client_data_p) {
    printk(KERN_ERR "unable to retrieve client state\n");
    return -ENODEV;
  }
  if (count_in > FIFOSTOREDATASIZE) {
    printk(KERN_ERR "can't store data because it's too big.");
    return -ENODEV;
  }
  if (client_data_p->fifostorepos >= FIFOSTORESIZE) {
    printk(KERN_ERR "can't store data because FIFO is full.");
    return -ENODEV;
  }
  printk(KERN_DEBUG "store_store(): storing %d bytes to store pos 0x%.2x\n", (int)count_in, client_data_p->fifostorepos);

  memcpy(client_data_p->fifostore[client_data_p->fifostorepos].data, buf_in, count_in);
  client_data_p->fifostore[client_data_p->fifostorepos].size = count_in;
  client_data_p->fifostorepos++;

  printk(KERN_DEBUG "queueing work PROCESSFIFOSTORE\n");
  queue_work(client_data_p->workqueue, &client_data_p->work_processfifostore.work);

  return count_in;
}
// this function is called when reading from the "store" sysfs file
static
ssize_t
i2c_mpu6050_store_show(struct kobject* kobj_in,
                       struct kobj_attribute* attr_in,
                       char* buf_in)
{
  struct i2c_mpu6050_client_data_t* client_data_p = NULL;
  int i, currentbufsize = 0;

  // sanity check(s)
//  struct device* device_p = NULL;
//  struct kobj_type* ktype = get_ktype(kobj_in);
//  if (ktype == &device_ktype)
//    device_p = to_dev(kobj_in);
//  if (!device_p) {
//    printk(KERN_ERR "%s: invalid parameter (not a device ?)\n", __func__);
//    return -ENODEV;
//  }
//  struct i2c_client* client_p = to_i2c_client(device_p);
  struct i2c_client* client_p = kobj_to_i2c_client(kobj_in);
  if (!client_p) {
    printk(KERN_ERR "%s: invalid parameter (not a I2C device ?)\n", __func__);
    return -ENODEV;
  }
  client_data_p = (struct i2c_mpu6050_client_data_t*)i2c_get_clientdata(client_p);
  if (!client_data_p) {
    printk(KERN_ERR "unable to retrieve client state\n");
    return -ENODEV;
  }

  i = client_data_p->ringbufferpos+1;
  if (i == RINGBUFFERSIZE)
    i = 0;

  while (i != client_data_p->ringbufferpos) {
    if (client_data_p->ringbuffer[i].completed) {
      currentbufsize = client_data_p->ringbuffer[i].size;
      // we found a used & completed slot, outputting
      printk(KERN_DEBUG "store_show(): outputting ringbuf %.2x, %d bytes\n", i, currentbufsize);
      memcpy(buf_in, client_data_p->ringbuffer[i].data, currentbufsize);
      client_data_p->ringbuffer[i].completed = client_data_p->ringbuffer[i].used = 0;
      return currentbufsize;
    }

    i++;
    if (i == RINGBUFFERSIZE)
      i = 0;
  }

  return 0;
}

static
void
i2c_mpu6050_workqueue_fifo_handler(struct work_struct* work_in) {
  struct fifo_work_t* work_p = NULL;
  struct i2c_mpu6050_client_data_t* client_data_p = NULL;
  int i, j;

  // sanity check(s)
  work_p = (struct fifo_work_t*)work_in;
  if (!work_p) {
    printk(KERN_ERR "unable to retrieve client state\n");
    return;
  }
  client_data_p = (struct i2c_mpu6050_client_data_t*)i2c_get_clientdata(work_p->client);
  if (!client_data_p) {
    printk(KERN_ERR "unable to retrieve client state\n");
    return;
  }

  printk(KERN_DEBUG "work PROCESSFIFOSTORE called\n");

  while (client_data_p->fifostorepos > 0) { // processing all items in the fifo store
    printk(KERN_DEBUG "%d entries in fifo store\n", client_data_p->fifostorepos);

    printk(KERN_DEBUG "sending %d bytes\n", client_data_p->fifostore[0].size);
    //      BEAGLEBONE_LED3ON;
    //      spi_write_reg_burst(SPIDEVDATAREG, fifostore[0].data, fifostore[0].size);
    //      BEAGLEBONE_LED3OFF;

    // left shifting the FIFO store
    for (i = 1; i < FIFOSTORESIZE; i++) {
      for (j = 0; j < client_data_p->fifostore[i].size; j++)
        client_data_p->fifostore[i-1].data[j] = client_data_p->fifostore[i].data[j];
      client_data_p->fifostore[i-1].size = client_data_p->fifostore[i].size;
    }
    client_data_p->fifostorepos--;
  }

  printk(KERN_DEBUG "work exit\n");
}
static
void
i2c_mpu6050_workqueue_read_handler(struct work_struct* work_in) {
  int err;
  struct fifo_work_t* work_p = NULL;
  struct i2c_mpu6050_client_data_t* client_data_p = NULL;

  // sanity check(s)
  work_p = (struct fifo_work_t*)work_in;
  if (!work_p) {
    printk(KERN_ERR "unable to retrieve client state\n");
    return;
  }
  client_data_p = (struct i2c_mpu6050_client_data_t*)i2c_get_clientdata(work_p->client);
  if (!client_data_p) {
    printk(KERN_ERR "unable to retrieve client state\n");
    return;
  }

  printk(KERN_DEBUG "work READ called, ringbuf %.2x\n",
         client_data_p->ringbufferpos);

  memset(&client_data_p->ringbuffer[client_data_p->ringbufferpos].data,
         0,
         RINGBUFFERDATASIZE);
  client_data_p->ringbuffer[client_data_p->ringbufferpos].used = 1;

//  gpio_write_one_pin_value(client_data_p->gpio_led_handle, 1, GPIO_LED_PH02_LABEL);
  err = i2c_master_recv(client_data_p->client,
                        client_data_p->ringbuffer[client_data_p->ringbufferpos].data,
                        RINGBUFFERDATASIZE);
  if (err < 0) {
    printk(KERN_ERR "unable to recieve device data: %d\n", -err);
    return;
  }
//  gpio_write_one_pin_value(client_data_p->gpio_led_handle, 0, GPIO_LED_PH02_LABEL);
  client_data_p->ringbuffer[client_data_p->ringbufferpos].size = RINGBUFFERDATASIZE;
  client_data_p->ringbuffer[client_data_p->ringbufferpos].completed = 1;

  printk(KERN_DEBUG "read stopped, ringbuf %.2x\n", client_data_p->ringbufferpos);

  client_data_p->ringbufferpos++;
  if (client_data_p->ringbufferpos == RINGBUFFERSIZE)
    client_data_p->ringbufferpos = 0;

  printk(KERN_DEBUG "work exit\n");
}

// this function gets called when an interrupt happens for the registered GPIO pins
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

static
void
i2c_mpu6050_clearringbuffer(void* data_in)
{
  struct i2c_mpu6050_client_data_t* client_data_p = NULL;
  int i;

  // sanity check(s)
  client_data_p = (struct i2c_mpu6050_client_data_t*)data_in;
  if (!client_data_p) {
    printk(KERN_ERR "unable to retrieve client state\n");
    return;
  }

  for (i = 0; i < RINGBUFFERSIZE; i++)
    client_data_p->ringbuffer[i].completed = client_data_p->ringbuffer[i].used = 0;
}

// this functions gets called when the user reads the sysfs file "somereg"
static
ssize_t
i2c_mpu6050_reg_show(struct kobject* kobj_in,
                     struct kobj_attribute* attr_in,
                     char* buf_in)
{
  unsigned int reg;
  struct i2c_client* client_p = kobj_to_i2c_client(kobj_in);
  if (!client_p) {
    printk(KERN_ERR "%s: invalid parameter (not a I2C device ?)\n", __func__);
    return -ENODEV;
  }

  sscanf(buf_in, "%x", &reg);
  return sprintf(buf_in, "%x\n", i2c_smbus_read_byte_data(client_p, (u8)reg));
}

// this function gets called when the user writes the sysfs file "somereg"
static
ssize_t
i2c_mpu6050_reg_store(struct kobject* kobj_in,
                      struct kobj_attribute* attr_in,
                      const char* buf_in,
                      size_t count_in)
{
  unsigned int reg, val;
  struct i2c_client* client_p = kobj_to_i2c_client(kobj_in);
  if (!client_p) {
    printk(KERN_ERR "%s: invalid parameter (not a I2C device ?)\n", __func__);
    return -ENODEV;
  }

  sscanf(buf_in, "%x", &reg);
  sscanf(buf_in + 1, "%x", &val);
  i2c_smbus_write_byte_data(client_p, (u8)reg, (u8)val);
  printk(KERN_DEBUG "stored %.2x to register %.2x\n", (u8)reg, (u8)val);
  return count_in;
}

// this function gets called when the user writes the sysfs file "clearringbuffer"
static
ssize_t
i2c_mpu6050_clearringbuffer_store(struct kobject* kobj_in,
                                  struct kobj_attribute* attr_in,
                                  const char* buf_in,
                                  size_t count_in)
{
  struct i2c_mpu6050_client_data_t* client_data_p = NULL;

  // sanity check(s)
//  struct device* device_p = kobj_to_dev(kobj_in);
////  struct kobj_type* ktype = get_ktype(kobj_in);
////  if (ktype == &device_ktype)
////    device_p = to_dev(kobj_in);
//  if (!device_p) {
//    printk(KERN_ERR "%s: invalid parameter (not a device ?)\n", __func__);
//    return -ENODEV;
//  }
//  struct i2c_client* client_p = to_i2c_client(device_p);
  struct i2c_client* client_p = kobj_to_i2c_client(kobj_in);
  if (!client_p) {
    printk(KERN_ERR "%s: invalid parameter (not a I2C device ?)\n", __func__);
    return -ENODEV;
  }
  client_data_p = (struct i2c_mpu6050_client_data_t*)i2c_get_clientdata(client_p);
  if (!client_data_p) {
    printk(KERN_ERR "unable to retrieve client state\n");
    return -ENODEV;
  }

  i2c_mpu6050_clearringbuffer(client_data_p);

  printk(KERN_DEBUG "ringbuffer cleared.\n");

  return count_in;
}

// these two functions are called when the user reads the sysfs files "gpio19state" and "gpio21state"
static
ssize_t
i2c_mpu6050_intstate_show(struct kobject* kobj_in,
                          struct kobj_attribute* attr_in,
                          char* buf_in)
{
  struct i2c_client* client_p = kobj_to_i2c_client(kobj_in);
  if (!client_p) {
    printk(KERN_ERR "%s: invalid parameter (not a I2C device ?)\n", __func__);
    return -ENODEV;
  }

  return sprintf(buf_in, "%d\n", gpio_get_value(irq_to_gpio(client_p->irq)));
}
static
ssize_t
i2c_mpu6050_ledstate_show(struct kobject* kobj_in,
                          struct kobj_attribute* attr_in,
                          char* buf_in)
{
  struct i2c_mpu6050_client_data_t* client_data_p = NULL;
  struct i2c_client* client_p = kobj_to_i2c_client(kobj_in);
  if (!client_p) {
    printk(KERN_ERR "%s: invalid parameter (not a I2C device ?)\n", __func__);
    return -ENODEV;
  }
  client_data_p = (struct i2c_mpu6050_client_data_t*)i2c_get_clientdata(client_p);
  if (!client_data_p) {
    printk(KERN_ERR "unable to retrieve client state\n");
    return -ENODEV;
  }

  return sprintf(buf_in, "%d\n", gpio_read_one_pin_value(client_data_p->gpio_led_handle, GPIO_LED_PH02_LABEL));
}

static struct kobj_attribute store_attribute = __ATTR(data, 0666, i2c_mpu6050_store_show, i2c_mpu6050_store_store);
static struct kobj_attribute somereg_attribute = __ATTR(addr, 0666, i2c_mpu6050_somereg_show, i2c_mpu6050_somereg_store);
static struct kobj_attribute clearringbuffer_attribute = __ATTR(clearringbuffer, 0666, NULL, i2c_mpu6050_clearringbuffer_store);
static struct kobj_attribute gpio19state_attribute = __ATTR(gpio19state, 0666, i2c_mpu6050_gpio19state_show, NULL);
static struct kobj_attribute gpio21state_attribute = __ATTR(gpio21state, 0666, i2c_mpu6050_gpio21state_show, NULL);
// a group of attributes so that we can create and destroy them all at once
static struct attribute* attrs[] = {
  &store_attribute.attr,
  &somereg_attribute.attr,
  &clearringbuffer_attribute.attr,
  &gpio19state_attribute.attr,
  &gpio21state_attribute.attr,
  NULL, // need to NULL terminate the list of attributes
};
/* An unnamed attribute group will put all of the attributes directly in
 * the kobject directory.  If we specify a name, a subdirectory will be
 * created for the attributes with the directory being the name of the
 * attribute group.
 */
static struct attribute_group attr_group = { .attrs = attrs };

// driver structure
// *NOTE*: see https://www.kernel.org/doc/Documentation/i2c/writing-clients
static struct i2c_device_id i2c_mpu6050_idtable[] = {
  { "mpu6050", 0 },
  { }
};
MODULE_DEVICE_TABLE(i2c, i2c_mpu6050_idtable);

static struct dev_pm_ops i2c_mpu6050_pm_ops = {
  .prepare = i2c_mpu6050_pm_prepare,
  .complete = i2c_mpu6050_pm_complete,
  .suspend = i2c_mpu6050_pm_suspend,
  .resume = i2c_mpu6050_pm_resume,
  .freeze = i2c_mpu6050_pm_freeze,
  .thaw = i2c_mpu6050_pm_thaw,
  .poweroff = i2c_mpu6050_pm_poweroff,
  .restore = i2c_mpu6050_pm_restore,
  .suspend_late = i2c_mpu6050_pm_suspend_late,
  .resume_early = i2c_mpu6050_pm_resume_early,
  .freeze_late = i2c_mpu6050_pm_freeze_late,
  .thaw_early = i2c_mpu6050_pm_thaw_early,
  .poweroff_late = i2c_mpu6050_pm_poweroff_late,
  .restore_early = i2c_mpu6050_pm_restore_early,
  .suspend_noirq = i2c_mpu6050_pm_suspend_noirq,
  .resume_noirq = i2c_mpu6050_pm_resume_noirq,
  .freeze_noirq = i2c_mpu6050_pm_freeze_noirq,
  .thaw_noirq = i2c_mpu6050_pm_thaw_noirq,
  .poweroff_noirq = i2c_mpu6050_pm_poweroff_noirq,
  .restore_noirq = i2c_mpu6050_pm_restore_noirq,
  .runtime_suspend = i2c_mpu6050_pm_runtime_suspend,
  .runtime_resume = i2c_mpu6050_pm_runtime_resume,
  .runtime_idle = i2c_mpu6050_pm_runtime_idle
};
static struct i2c_driver i2c_mpu6050_driver = {
  .class = I2C_CLASS_HWMON,

  /* notifies the driver that a new bus has appeared.
     *NOTE*: avoid using this, it will be removed in a
     near future. */
  .attach_adapter = i2c_mpu6050_attach_adapter,

  /* standard driver model interfaces */
  .probe		= i2c_mpu6050_probe,
  .remove		= __devexit_p(i2c_mpu6050_remove),

  /* driver model interfaces that don't relate to enumeration */
  .shutdown	= i2c_mpu6050_shutdown,
  .suspend	= i2c_mpu6050_suspend,
  .resume	= i2c_mpu6050_resume,

  /* alert callback, for example for the SMBus alert protocol.
     The format and meaning of the data value depends on the protocol.
     For the SMBus alert protocol, there is a single bit of data passed
     as the alert response's low bit ("event flag"). */
  .alert = i2c_mpu6050_alert,

  /* a ioctl like command that can be used to perform specific functions
     with the device. */
  .command = i2c_mpu6050_command,

  .driver = {
    .name	= "mpu6050", // be sure to match this with any aliased 'i2c_board_info's
//    .bus = ,
    .owner = THIS_MODULE,
//    .mod_name = ,
//    .suppress_bind_attrs = ,
//    .of_device_id = ,
//    .acpi_device_id = ,
//    .probe = ,
//    .remove = ,
//    .shutdown = ,
//    .suspend = ,
//    .resume = ,
    .attribute_group = attr_group,
    .pm	= &i2c_mpu6050_pm_ops,
//    .p = ,
  },
  .id_table	= i2c_mpu6050_idtable,

  /* device detection callback for automatic device creation */
  .detect		= i2c_mpu6050_detect,
  .address_list	= normal_i2c,
  .clients =
}
//module_i2c_driver(i2c_mpu6050_driver);

// this function gets called when a matching modalias and driver name found
static int __devinit i2c_mpu6050_probe(struct i2c_client* client_in,
                                       const struct i2c_device_id* id_in)
{
  int err, gpio_used;
  struct i2c_mpu6050_client_data* client_data_p;

  printk(KERN_DEBUG "i2c_mpu6050_probe() called.\n");

  // sanity check(s)
  if (!i2c_check_functionality(client_in->adapter,
                               (I2C_FUNC_SMBUS_BYTE_DATA |
                                I2C_FUNC_SMBUS_WORD_DATA |
                                I2C_FUNC_SMBUS_I2C_BLOCK))) {
    printk(KERN_ERR "%s: needed i2c functionality is not supported\n", __func__);
    return -ENODEV;
  }

  client_data_p = kzalloc(sizeof(struct i2c_mpu6050_client_data), GFP_KERNEL);
  if (IS_ERR(client_data_p)) {
    printk(KERN_ERR "%s: no memory\n", __func__);
    return PTR_ERR(client_data_p);
  }
  i2c_set_clientdata(client_in, client_data_p);

  err = script_parser_fetch("gpio_para",
                            "gpio_used",
                            &gpio_used,
                            sizeof(gpio_used)/sizeof(int));
  if (err) {
    pr_err("%s script_parser_fetch \"gpio_para\" \"gpio_used\" error\n", __FUNCTION__);

    // clean up
    kfree(client_data_p);

    return -ENODEV;
  }
  err = script_parser_fetch("gpio_para",
                            GPIO_UEXT4_UART4RX_PG11_LABEL,
                            (int*)&client_data_p->gpio_int_data,
                            sizeof(struct script_gpio_set_t));
  if (err) {
    pr_err("%s script_parser_fetch \"gpio_para\" \"%s\" error\n",
           __FUNCTION__,
           GPIO_UEXT4_UART4RX_PG11_LABEL);

    // clean up
    kfree(client_data_p);

    return -ENODEV;
  }
  err = script_parser_fetch("gpio_para",
                            GPIO_LED_PH02_LABEL,
                            (int*)&client_data_p->gpio_led_data,
                            sizeof(struct script_gpio_set_t));
  if (err) {
    pr_err("%s script_parser_fetch \"gpio_para\" \"%s\" error\n",
           __FUNCTION__,
           GPIO_LED_PH02_LABEL);

    // clean up
    kfree(client_data_p);

    return -ENODEV;
  }

  client_data_p->client = client_in;
  client_data_p->object = kobject_create_and_add("i2c_mpu6050", kernel_kobj);
  if (IS_ERR(client_data_p->object)) {
    printk(KERN_ERR "unable to create sysfs object\n");

    // clean up
    kfree(client_data_p);

    return PTR_ERR(client_data_p->object);
  }
  // create the files associated with this kobject
  err = sysfs_create_group(client_data_p->object, &attr_group);
  if (err) {
    printk(KERN_ERR "unable to create sysfs files\n");

    // clean up
    kobject_unregister(client_data_p->object);
    kfree(client_data_p);

    return -ENOMEM;
  }
  // *TODO*: announce new sysfs object
//  err = kobject_uevent(client_data_p->object, enum kobject_action action);
//  if (err) {
//    printk(KERN_ERR "unable to announce sysfs files\n");

//    // clean up
//    kobject_unregister(client_data_p->object);
//    kfree(client_data_p);

//    return -ENOMEM;
//  }
  client_data_p->workqueue = create_singlethread_workqueue("i2c_mpu6050_workqueue");
  if (IS_ERR(client_data_p->workqueue)) {
    printk(KERN_ERR "unable to create workqueue\n");

    // clean up
    kobject_unregister(client_data_p->object);
    kfree(client_data_p);

    return PTR_ERR(client_data_p->workqueue);
  }
  INIT_WORK(&client_data_p->work_processfifostore.work, i2c_mpu6050_workqueue_fifo_handler);
  client_data_p->work_fifo.client = client_data_p->client;
  INIT_WORK(&client_data_p->work_read.work, i2c_mpu6050_workqueue_read_handler);
  client_data_p->work_read.client = client_data_p->client;
  i2c_mpu6050_clearringbuffer(client_data_p);
  __SPIN_LOCK_INITIALIZER(client_data_p->sync_lock);

  // initialize interrupt for GPIO INT line
  client_data_p->pin_ctrl = devm_pinctrl_get(client_in->dev);
  if (IS_ERR(client_data_p->pin_ctrl)) {
    printk(KERN_ERR "unable to retrieve pinctrl configuration\n");

    // clean up
    destroy_workqueue(client_data_p->workqueue);
    kobject_unregister(client_data_p->object);
    kfree(client_data_p);

    return PTR_ERR(client_data_p->pin_ctrl);
  }
  client_data_p->pin_ctrl_state = pinctrl_lookup_state(client_data_p->pin_ctrl,
                                                       PINCTRL_STATE_DEFAULT);
  if (IS_ERR(client_data_p->pin_ctrl_state)) {
    printk(KERN_ERR "unable to retrieve pinctrl state\n");

    // clean up
    devm_pinctrl_put(client_data_p->pin_ctrl);
    destroy_workqueue(client_data_p->workqueue);
    kobject_unregister(client_data_p->object);
    kfree(client_data_p);

    return PTR_ERR(client_data_p->pin_ctrl_state);
  }
  err = pinctrl_select_state(client_data_p->pin_ctrl_state);
  if (err < 0) {
    printk(KERN_ERR "unable to set pinctrl state\n");

    // clean up
    devm_pinctrl_put(client_data_p->pin_ctrl);
    destroy_workqueue(client_data_p->workqueue);
    kobject_unregister(client_data_p->object);
    kfree(client_data_p);

    return -ENOSYS;
  }
  err = gpio_is_valid(GPIO_UEXT4_UART4RX_PG11_PIN);
  if (err) {
    printk(KERN_ERR "%s: invalid GPIO\n", GPIO_UEXT4_UART4RX_PG11_LABEL);

    // clean up
    devm_pinctrl_put(client_data_p->pin_ctrl);
    destroy_workqueue(client_data_p->workqueue);
    kobject_unregister(client_data_p->object);
    kfree(client_data_p);

    return -ENOSYS;
  }
  err = gpio_is_valid(GPIO_LED_PH02_PIN);
  if (err) {
    printk(KERN_ERR "%s: invalid GPIO\n", GPIO_LED_PH02_LABEL);

    // clean up
    devm_pinctrl_put(client_data_p->pin_ctrl);
    destroy_workqueue(client_data_p->workqueue);
    kobject_unregister(client_data_p->object);
    kfree(client_data_p);

    return -ENOSYS;
  }
  err = devm_gpio_request(client_in->dev,
                          GPIO_UEXT4_UART4RX_PG11_PIN,
                          GPIO_UEXT4_UART4RX_PG11_LABEL);
  if (err < 0) {
    printk(KERN_ERR "%s: unable to request GPIO\n",
           GPIO_UEXT4_UART4RX_PG11_LABEL);

    // clean up
    devm_pinctrl_put(client_data_p->pin_ctrl);
    destroy_workqueue(client_data_p->workqueue);
    kobject_unregister(client_data_p->object);
    kfree(client_data_p);

    return -ENOSYS;
  }
//  err = gpio_request_one(GPIO_UEXT4_UART4RX_PG11_PIN,
//                         GPIOF_DIR_IN,
//                         GPIO_UEXT4_UART4RX_PG11_LABEL);
//  if (err) {
//    printk(KERN_ERR "%s: unable to request GPIO\n",
//           GPIO_UEXT4_UART4RX_PG11_LABEL);

//    // clean up
//    destroy_workqueue(client_data_p->workqueue);
//    kobject_unregister(client_data_p->object);
//    kfree(client_data_p);

//    return -EIO;
//  }
  client_data_p->gpio_led_handle = gpio_request_ex("gpio_para",
                                                   GPIO_LED_PH02_LABEL);
  if (client_data_p->gpio_led_handle < 0) {
    printk(KERN_ERR "%s: unable to request GPIO\n",
           GPIO_LED_PH02_LABEL);

    // clean up
    devm_pinctrl_put(client_data_p->pin_ctrl);
    destroy_workqueue(client_data_p->workqueue);
    kobject_unregister(client_data_p->object);
    kfree(client_data_p);

    return -ENOSYS;
  }
  // export GPIO to userspace
  err = gpio_export(GPIO_UEXT4_UART4RX_PG11_PIN,
                    false);
  if (err) {
    printk(KERN_ERR "%s: unable to export GPIO\n",
           GPIO_UEXT4_UART4RX_PG11_LABEL);

    // clean up
    gpio_release(client_data_p->gpio_led_handle, 1);
    devm_gpio_free(GPIO_UEXT4_UART4RX_PG11_PIN);
    devm_pinctrl_put(client_data_p->pin_ctrl);
    destroy_workqueue(client_data_p->workqueue);
    kobject_unregister(client_data_p->object);
    kfree(client_data_p);

    return -EIO;
  }
  err = gpio_export_link(client_data_p->client.dev,
                         GPIO_UEXT4_UART4RX_PG11_LABEL,
                         GPIO_UEXT4_UART4RX_PG11_PIN);
  if (err) {
    printk(KERN_ERR "%s: unable to export GPIO\n",
           GPIO_UEXT4_UART4RX_PG11_10_LABEL);

    // clean up
    gpio_unexport(GPIO_UEXT4_UART4RX_PG11_PIN);
    gpio_release(client_data_p->gpio_led_handle, 1);
    devm_gpio_free(GPIO_UEXT4_UART4RX_PG11_PIN);
    devm_pinctrl_put(client_data_p->pin_ctrl);
    destroy_workqueue(client_data_p->workqueue);
    kobject_unregister(client_data_p->object);
    kfree(client_data_p);

    return -EIO;
  }
  client_data_p->client.irq = gpio_to_irq(GPIO_UEXT4_UART4RX_PG11_PIN);
  if (client_data_p->client.irq < 0) {
    printk(KERN_ERR "%s: unable to request GPIO IRQ\n",
           GPIO_UEXT4_UART4RX_PG11_10_LABEL);

    // clean up
    gpio_unexport(GPIO_UEXT4_UART4RX_PG11_PIN);
    gpio_release(client_data_p->gpio_led_handle, 1);
    devm_gpio_free(GPIO_UEXT4_UART4RX_PG11_PIN);
    devm_pinctrl_put(client_data_p->pin_ctrl);
    destroy_workqueue(client_data_p->workqueue);
    kobject_unregister(client_data_p->object);
    kfree(client_data_p);

    return -EIO;
  }
  printk(KERN_INFO "%s: GPIO %d --> IRQ %d\n",
         GPIO_UEXT4_UART4RX_PG11_LABEL,
         GPIO_UEXT4_UART4RX_PG11_PIN,
         client_data_p->client.irq);
  gpiochip_lock_as_irq();
  err = request_irq(client_data_p->client.irq,
                    i2c_mpu6050_interrupt_handler,
                    SA_INTERRUPT | IRQF_TRIGGER_RISING,
                    GPIO_UEXT4_UART4RX_PG11_LABEL,
                    &(client_data_p->client.dev));
  if (err) {
    printk(KERN_ERR "unable to request IRQ: %d already claimed or allocation failed\n", client_data_p->client.irq);

    // clean up
    gpio_unexport(GPIO_UEXT4_UART4RX_PG11_PIN);
    gpio_release(client_data_p->gpio_led_handle, 1);
    devm_gpio_free(GPIO_UEXT4_UART4RX_PG11_PIN);
    devm_pinctrl_put(client_data_p->pin_ctrl);
    destroy_workqueue(client_data_p->workqueue);
    kobject_unregister(client_data_p->object);
    kfree(client_data_p);

    return -EIO;
  }

  return 0;
}
// this function gets called when our example SPI driver gets removed with spi_unregister_driver()
static int i2c_mpu6050_remove(struct i2c_client* client_in)
{
  struct i2c_mpu6050_client_data_t* client_data_p = NULL;

  printk(KERN_DEBUG "i2c_mpu6050_remove() called.\n");

  client_data_p = (struct i2c_mpu6050_client_data_t*)i2c_get_clientdata(client_in);
  if (!client_data_p) {
    printk(KERN_ERR "unable to retrieve client state\n");
    return -ENODEV;
  }

  // clean up
  gpiochip_unlock_as_irq();
  free_irq(client_data_p->client->irq, NULL);
  gpio_unexport(SUNXI_GPIO(GPIO_UEXT4_UART4RX_PG11_10_NAME));
  gpio_free(SUNXI_GPIO(GPIO_UEXT4_UART4RX_PG11_10_NAME));
  flush_workqueue(client_data_p->workqueue);
  destroy_workqueue(client_data_p->workqueue);
  kobject_unregister(client_data_p->object);
  kfree(client_data_p);

  return 0;
}

// this gets called on module init
static int __init i2c_mpu6050_init(void)
{
  char buffer[BUFSIZ];

  // registering I2C driver, this will call i2c_mpu6050_probe()
  int error = i2c_add_driver(&i2c_mpu6050_driver);
  if (error < 0) {
    printk(KERN_ERR "i2c_add_driver() failed %d\n", error);
    return error;
  }

  memset(&buffer, 0, sizeof(buffer));
  strcpy(&buffer, KO_OLIMEX_MOD_MPU6050_DESCRIPTION);
  strcpy(&buffer + sizeof(KO_OLIMEX_MOD_MPU6050_DESCRIPTION), "...added\n");
  printk(KERN_INFO buffer);

  return 0;
}
// this gets called when module is being unloaded
static void __exit i2c_mpu6050_exit(void)
{
  char buffer[BUFSIZ];

  // unregistering I2C driver, this will call i2c_mpu6050_remove()
  i2c_del_driver(&i2c_mpu6050_driver);

  memset(&buffer, 0, sizeof(buffer));
  strcpy(&buffer, KO_OLIMEX_MOD_MPU6050_DESCRIPTION);
  strcpy(&buffer + sizeof(KO_OLIMEX_MOD_MPU6050_DESCRIPTION), "...removed\n");
  printk(KERN_INFO buffer);
}
// setting which function to call on module init and exit
module_init(i2c_mpu6050_init);
module_exit(i2c_mpu6050_exit);

MODULE_LICENSE(KO_OLIMEX_MOD_MPU6050_LICENSE);
MODULE_AUTHOR(KO_OLIMEX_MOD_MPU6050_AUTHOR);
MODULE_VERSION(KO_OLIMEX_MOD_MPU6050_VERSION);
MODULE_DESCRIPTION(KO_OLIMEX_MOD_MPU6050_DESCRIPTION);
