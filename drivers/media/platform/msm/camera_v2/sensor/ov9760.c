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
#include "msm_sensor.h"
#define OV9760_SENSOR_NAME "ov9760"
DEFINE_MSM_MUTEX(ov9760_mut);

static struct msm_sensor_ctrl_t ov9760_s_ctrl;

static struct msm_sensor_power_setting ov9760_power_setting[] = {
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_STANDBY,
		.config_val = GPIO_OUT_LOW,
		.sleep_val = GPIO_OUT_LOW,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_LOW,
		.sleep_val = GPIO_OUT_LOW,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_VIO,
		.config_val = GPIO_OUT_HIGH,
		.sleep_val = GPIO_OUT_HIGH,
		.delay = 0,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_STANDBY,
		.config_val = GPIO_OUT_HIGH,
		.sleep_val = GPIO_OUT_LOW,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_VANA,
		.config_val = GPIO_OUT_HIGH,
		.sleep_val = GPIO_OUT_HIGH,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_VDIG,
		.config_val = GPIO_OUT_HIGH,
		.sleep_val = GPIO_OUT_HIGH,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_GPIO,
		.seq_val = SENSOR_GPIO_RESET,
		.config_val = GPIO_OUT_HIGH,
		.sleep_val = GPIO_OUT_HIGH,
		.delay = 5,
	},
	{
		.seq_type = SENSOR_CLK,
		.seq_val = SENSOR_CAM_MCLK,
		.config_val = 0,
		.delay = 10,
	},
	{
		.seq_type = SENSOR_I2C_MUX,
		.seq_val = 0,
		.config_val = 0,
		.delay = 0,
	},
};

static struct v4l2_subdev_info ov9760_subdev_info[] = {
	{
		.code   = V4L2_MBUS_FMT_SBGGR10_1X10,
		.colorspace = V4L2_COLORSPACE_JPEG,
		.fmt    = 1,
		.order    = 0,
	},
};

static const struct i2c_device_id ov9760_i2c_id[] = {
	{OV9760_SENSOR_NAME, (kernel_ulong_t)&ov9760_s_ctrl},
	{ }
};

static int32_t msm_ov9760_i2c_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	return msm_sensor_i2c_probe(client, id, &ov9760_s_ctrl);
}

static struct i2c_driver ov9760_i2c_driver = {
	.id_table = ov9760_i2c_id,
	.probe  = msm_ov9760_i2c_probe,
	.driver = {
		.name = OV9760_SENSOR_NAME,
	},
};

static struct msm_camera_i2c_client ov9760_sensor_i2c_client = {
	.addr_type = MSM_CAMERA_I2C_WORD_ADDR,
};

static const struct of_device_id ov9760_dt_match[] = {
	{.compatible = "qcom,ov9760", .data = &ov9760_s_ctrl},
	{}
};

MODULE_DEVICE_TABLE(of, ov9760_dt_match);

static struct platform_driver ov9760_platform_driver = {
	.driver = {
		.name = "qcom,ov9760",
		.owner = THIS_MODULE,
		.of_match_table = ov9760_dt_match,
	},
};

static int32_t ov9760_platform_probe(struct platform_device *pdev)
{
	int32_t rc = 0;
	const struct of_device_id *match;
	match = of_match_device(ov9760_dt_match, &pdev->dev);
	rc = msm_sensor_platform_probe(pdev, match->data);
	return rc;
}

static int __init ov9760_init_module(void)
{
	int32_t rc = 0;
	pr_err("%s:%d\n", __func__, __LINE__);
	rc = platform_driver_probe(&ov9760_platform_driver,
		ov9760_platform_probe);
	if (!rc)
		return rc;
	pr_err("%s:%d rc %d\n", __func__, __LINE__, rc);
	return i2c_add_driver_async(&ov9760_i2c_driver);
}

static void __exit ov9760_exit_module(void)
{
	pr_err("%s:%d\n", __func__, __LINE__);
	if (ov9760_s_ctrl.pdev) {
		msm_sensor_free_sensor_data(&ov9760_s_ctrl);
		platform_driver_unregister(&ov9760_platform_driver);
	} else
		i2c_del_driver(&ov9760_i2c_driver);
	return;
}

static struct msm_sensor_ctrl_t ov9760_s_ctrl = {
	.sensor_i2c_client = &ov9760_sensor_i2c_client,
	.power_setting_array.power_setting = ov9760_power_setting,
	.power_setting_array.size = ARRAY_SIZE(ov9760_power_setting),
	.msm_sensor_mutex = &ov9760_mut,
	.sensor_v4l2_subdev_info = ov9760_subdev_info,
	.sensor_v4l2_subdev_info_size = ARRAY_SIZE(ov9760_subdev_info),
	.msm_sensor_reg_default_data_type=MSM_CAMERA_I2C_BYTE_DATA,
};

module_init(ov9760_init_module);
module_exit(ov9760_exit_module);
MODULE_DESCRIPTION("ov9760");
MODULE_LICENSE("GPL v2");
