/* Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

/*
 * Created by weilanying on 20131014
 */

#include <linux/module.h>
#include <linux/export.h>
#include "msm_led_flash.h"
#define CONFIG_MSMB_CAMERA_DEBUG
#undef CDBG
#ifdef CONFIG_MSMB_CAMERA_DEBUG
#define CDBG(fmt, args...) pr_err(fmt, ##args)
#else
#define CDBG(fmt, args...) do { } while (0)
#endif
#define FLASH_NAME "qcom,lm3642"
/*#define STROBE_HARDWARE_GPIO_CONTROL*/
static struct msm_led_flash_ctrl_t fctrl;
static struct i2c_driver lm3642_i2c_driver;

extern int msm_flash_i2c_probe(struct i2c_client *client,
		const struct i2c_device_id *id);
extern int32_t msm_led_i2c_trigger_get_subdev_id(struct msm_led_flash_ctrl_t *fctrl, void *arg);
extern int32_t msm_led_i2c_trigger_config(struct msm_led_flash_ctrl_t *fctrl, void *data);
extern int msm_camera_request_gpio_table(struct gpio *gpio_tbl, uint8_t size, int gpio_en);
extern void gpio_set_value_cansleep(unsigned gpio, int value);
#ifdef STROBE_HARDWARE_GPIO_CONTROL
static struct msm_camera_i2c_reg_array lm3642_init_array[] = {
	{ 0x0A, 0x00 }, // standby mode
	{ 0x01, 0x00 },
};

static struct msm_camera_i2c_reg_array lm3642_low_array[] = {
	{ 0x09, 0x28 }, // current control torch=140.63mA
	{ 0x0A, 0x02 }, // torch mode
	{ 0x01, 0x00 },
};

static struct msm_camera_i2c_reg_array lm3642_high_array[] = {
	{ 0x08, 0x57 }, // IVM-D threshold voltage=3.4v default=0x52
	{ 0x09, 0x29 }, // current control 0x28->843.75mA 0x29>937.5mA
	{ 0x0A, 0xA3 }, // flash mode
	{ 0x01, 0x00 },
};

static struct msm_camera_i2c_reg_array lm3642_off_array[] = {
	{ 0x0A, 0x00 },
	{ 0x01, 0x00 },
};

static struct msm_camera_i2c_reg_array lm3642_release_array[] = {
	{ 0x0A, 0x00 },
	{ 0x01, 0x00 },
};
#else
static struct msm_camera_i2c_reg_array lm3642_init_array[] = {
	{ 0x0A, 0x00 }, // standby mode
	{ 0x01, 0x00 },
};

static struct msm_camera_i2c_reg_array lm3642_low_array[] = {
	{ 0x06, 0x00 }, // 
	{ 0x09, 0x28 }, // current control torch=140.63mA
	{ 0x0A, 0x02 }, // torch mode
	{ 0x01, 0x00 },
};

static struct msm_camera_i2c_reg_array lm3642_high_array[] = {
	{ 0x08, 0x57 }, // IVM-D threshold voltage=3.4v default=0x52
	{ 0x09, 0x29 }, // current control 0x28->843.75mA 0x29>937.5mA
	{ 0x0A, 0x83 }, // flash mode
	{ 0x01, 0x00 },
};

static struct msm_camera_i2c_reg_array lm3642_off_array[] = {
	{ 0x0A, 0x00 },
	{ 0x01, 0x00 },
};

static struct msm_camera_i2c_reg_array lm3642_release_array[] = {
	{ 0x0A, 0x00 },
	{ 0x01, 0x00 },
};
#endif

static void __exit msm_flash_lm3642_i2c_remove(void)
{
	i2c_del_driver(&lm3642_i2c_driver);
	return;
}

static const struct of_device_id lm3642_i2c_trigger_dt_match[] = {
	{.compatible = "qcom,lm3642"},
	{}
};

MODULE_DEVICE_TABLE(of, lm3642_i2c_trigger_dt_match);

static const struct i2c_device_id flash_i2c_id[] = {
	{"qcom,lm3642", (kernel_ulong_t)&fctrl},
	{ }
};

static const struct i2c_device_id lm3642_i2c_id[] = {
	{FLASH_NAME, (kernel_ulong_t)&fctrl},
	{ }
};

static int msm_flash_lm3642_i2c_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	if (!id) {
		pr_err("msm_flash_lm3642_i2c_probe: id is NULL");
		id = lm3642_i2c_id;
	}

	return msm_flash_i2c_probe(client, id);
}

/*
  * Added close the led-flash when powerdown
  *
  * by ZTE_YCM_20140421 yi.changming
  */
// --->
static void zte_flash_shutdown(struct i2c_client *client)
{
	pr_err("%s: E\n", __func__);

	fctrl.func_tbl->flash_led_off(&fctrl);

}
// <---

static struct i2c_driver lm3642_i2c_driver = {
	.id_table = lm3642_i2c_id,
	.probe  = msm_flash_lm3642_i2c_probe,
	.remove = __exit_p(msm_flash_lm3642_i2c_remove),
	.driver = {
		.name = FLASH_NAME,
		.owner = THIS_MODULE,
		.of_match_table = lm3642_i2c_trigger_dt_match,
	},
/*
  * Added close the led-flash when powerdown
  *
  * by ZTE_YCM_20140421 yi.changming
  */
// --->
	.shutdown = zte_flash_shutdown,
// <---
};

static int __init msm_flash_lm3642_i2c_add_driver(void)
{
	CDBG("%s called\n", __func__);
	return i2c_add_driver(&lm3642_i2c_driver);
}

static struct msm_camera_i2c_client lm3642_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
};

static struct msm_camera_i2c_reg_setting lm3642_init_setting = {
	.reg_setting = lm3642_init_array,
	.size = ARRAY_SIZE(lm3642_init_array),
	.addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.delay = 0,
};

static struct msm_camera_i2c_reg_setting lm3642_off_setting = {
	.reg_setting = lm3642_off_array,
	.size = ARRAY_SIZE(lm3642_off_array),
	.addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.delay = 0,
};

static struct msm_camera_i2c_reg_setting lm3642_release_setting = {
	.reg_setting = lm3642_release_array,
	.size = ARRAY_SIZE(lm3642_release_array),
	.addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.delay = 0,
};

static struct msm_camera_i2c_reg_setting lm3642_low_setting = {
	.reg_setting = lm3642_low_array,
	.size = ARRAY_SIZE(lm3642_low_array),
	.addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.delay = 0,
};

static struct msm_camera_i2c_reg_setting lm3642_high_setting = {
	.reg_setting = lm3642_high_array,
	.size = ARRAY_SIZE(lm3642_high_array),
	.addr_type = MSM_CAMERA_I2C_BYTE_ADDR,
	.data_type = MSM_CAMERA_I2C_BYTE_DATA,
	.delay = 0,
};

static struct msm_led_flash_reg_t lm3642_regs = {
	.init_setting = &lm3642_init_setting,
	.off_setting = &lm3642_off_setting,
	.low_setting = &lm3642_low_setting,
	.high_setting = &lm3642_high_setting,
	.release_setting = &lm3642_release_setting,
};
static int32_t lm3642_led_trigger_i2c_write_table	(struct msm_led_flash_ctrl_t *fctrl,
	struct msm_camera_i2c_reg_setting *write_setting);
static int32_t lm3642_led_trigger_init(struct msm_led_flash_ctrl_t *fctrl);
static int32_t lm3642_led_trigger_low(struct msm_led_flash_ctrl_t *fctrl);
static int32_t lm3642_led_trigger_high(struct msm_led_flash_ctrl_t *fctrl);
static int32_t lm3642_led_trigger_off(struct msm_led_flash_ctrl_t *fctrl);
static int32_t lm3642_led_trigger_release(struct msm_led_flash_ctrl_t *fctrl);

static struct msm_flash_fn_t lm3642_func_tbl = {
	.flash_get_subdev_id = msm_led_i2c_trigger_get_subdev_id,
	.flash_led_config = msm_led_i2c_trigger_config,
	.flash_led_init = lm3642_led_trigger_init,
	.flash_led_release = lm3642_led_trigger_release,
	.flash_led_off = lm3642_led_trigger_off,
	.flash_led_low = lm3642_led_trigger_low,
	.flash_led_high = lm3642_led_trigger_high,
	
	
};

static int32_t lm3642_led_trigger_i2c_write_table	(struct msm_led_flash_ctrl_t *fctrl,
	struct msm_camera_i2c_reg_setting *write_setting)
{
	int32_t  rc = 0;

		rc = fctrl->flash_i2c_client->i2c_func_tbl->i2c_write_table(fctrl->flash_i2c_client, 
			write_setting);		
		if (rc < 0) {
		pr_err("%s: i2c write table failed\n", __func__);
		}

	return rc;
}

static int32_t lm3642_led_trigger_init(struct msm_led_flash_ctrl_t *fctrl)
{
	int32_t rc = 0;

	CDBG("%s: E\n", __func__);

	rc = lm3642_led_trigger_i2c_write_table(fctrl, fctrl->reg_setting->init_setting);
	if (rc < 0) {
		pr_err("%s: i2c write table failed\n", __func__);
		return rc;
	}
	rc = msm_camera_request_gpio_table(
		fctrl->flashdata->gpio_conf->cam_gpio_req_tbl,
		fctrl->flashdata->gpio_conf->cam_gpio_req_tbl_size, 1);
	if (rc < 0) {
		pr_err("%s: request gpio failed\n", __func__);
		return rc;
	}
#ifdef STROBE_HARDWARE_GPIO_CONTROL
	gpio_set_value_cansleep(fctrl->flashdata->gpio_conf->gpio_num_info->gpio_num[1], GPIO_OUT_LOW);
#endif
	CDBG("%s: X\n", __func__);

	return 0;
}

static int32_t lm3642_led_trigger_low(struct msm_led_flash_ctrl_t *fctrl)
{
	int32_t rc = 0;

	CDBG("%s: E\n", __func__);

	rc = lm3642_led_trigger_i2c_write_table(fctrl, fctrl->reg_setting->low_setting);
	if (rc < 0) {
		pr_err("%s: i2c write table failed\n", __func__);
		return rc;
	}

	CDBG("%s: X\n", __func__);

	return 0;
}

static int32_t lm3642_led_trigger_high(struct msm_led_flash_ctrl_t *fctrl)
{
	int32_t rc = 0;

	CDBG("%s: E\n", __func__);

	rc = lm3642_led_trigger_i2c_write_table(fctrl, fctrl->reg_setting->high_setting);
	if (rc < 0) {
		pr_err("%s: i2c write table failed\n", __func__);
		return rc;
	}
#ifdef STROBE_HARDWARE_GPIO_CONTROL
	gpio_set_value_cansleep(fctrl->flashdata->gpio_conf->gpio_num_info->gpio_num[1], GPIO_OUT_HIGH);
#endif
	CDBG("%s: X\n", __func__);

	return 0;
}

static int32_t lm3642_led_trigger_off(struct msm_led_flash_ctrl_t *fctrl)
{
	int32_t rc = 0;

	CDBG("%s: E\n", __func__);

	rc = lm3642_led_trigger_i2c_write_table(fctrl, fctrl->reg_setting->off_setting);
	if (rc < 0) {
		pr_err("%s: i2c write table failed\n", __func__);
		return rc;
	}
#ifdef STROBE_HARDWARE_GPIO_CONTROL
	gpio_set_value_cansleep(fctrl->flashdata->gpio_conf->gpio_num_info->gpio_num[1], GPIO_OUT_LOW);
#endif
	CDBG("%s: X\n", __func__);

	return 0;
}

static int32_t lm3642_led_trigger_release(struct msm_led_flash_ctrl_t *fctrl)
{
	int32_t rc = 0;

	CDBG("%s: E\n", __func__);

	rc = lm3642_led_trigger_i2c_write_table(fctrl, fctrl->reg_setting->release_setting);
	if (rc < 0) {
		pr_err("%s: i2c write table failed\n", __func__);
		return rc;
	}

	gpio_set_value_cansleep(fctrl->flashdata->gpio_conf->gpio_num_info->gpio_num[1], GPIO_OUT_LOW);

	(void)msm_camera_request_gpio_table(fctrl->flashdata->gpio_conf->cam_gpio_req_tbl,
										fctrl->flashdata->gpio_conf->cam_gpio_req_tbl_size, 0);

	CDBG("%s: X\n", __func__);

	return 0;
}

static struct msm_led_flash_ctrl_t fctrl = {
	.flash_i2c_client = &lm3642_i2c_client,
	.reg_setting = &lm3642_regs,
	.func_tbl = &lm3642_func_tbl,
};

/*subsys_initcall(msm_flash_i2c_add_driver);*/
module_init(msm_flash_lm3642_i2c_add_driver);
module_exit(msm_flash_lm3642_i2c_remove);
MODULE_DESCRIPTION("lm3642 FLASH");
MODULE_LICENSE("GPL v2");


