/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.c
  * @brief   This file provides code for the configuration
  *          of the I2C instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "i2c.h"

/* USER CODE BEGIN 0 */
#include "oledfont.h"
/* USER CODE END 0 */

/* I2C1 init function */
void MX_I2C1_Init(void) {
    /* USER CODE BEGIN I2C1_Init 0 */

    /* USER CODE END I2C1_Init 0 */

    LL_I2C_InitTypeDef I2C_InitStruct = {0};

    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOB);
    /**I2C1 GPIO Configuration
    PB6   ------> I2C1_SCL
    PB7   ------> I2C1_SDA
    */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_6 | LL_GPIO_PIN_7;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* Peripheral clock enable */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);

    /* USER CODE BEGIN I2C1_Init 1 */

    /* USER CODE END I2C1_Init 1 */

    /** I2C Initialization
    */
    LL_I2C_DisableOwnAddress2(I2C1);
    LL_I2C_DisableGeneralCall(I2C1);
    LL_I2C_EnableClockStretching(I2C1);
    I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
    I2C_InitStruct.ClockSpeed = 100000;
    I2C_InitStruct.DutyCycle = LL_I2C_DUTYCYCLE_2;
    I2C_InitStruct.OwnAddress1 = 0;
    I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
    I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
    LL_I2C_Init(I2C1, &I2C_InitStruct);
    LL_I2C_SetOwnAddress2(I2C1, 0);
    /* USER CODE BEGIN I2C1_Init 2 */

    /* USER CODE END I2C1_Init 2 */
}

/* USER CODE BEGIN 1 */
void OLED_WriteCmd(uint8_t cmd) {
    // 1. 产生起始信号 (START)
    LL_I2C_GenerateStartCondition(I2C1);
    while (!LL_I2C_IsActiveFlag_SB(I2C1)); // 等待起始信号发送完成

    // 2. 发送设备地址，并等待单片机收到屏幕的应答 (ACK)
    LL_I2C_TransmitData8(I2C1, OLED_I2C_ADDR);
    while (!LL_I2C_IsActiveFlag_ADDR(I2C1)); // 等待地址发送完成标志位
    LL_I2C_ClearFlag_ADDR(I2C1); // 必须手动清除 ADDR 标志位，总线才会继续

    // 3. 发送控制字节 0x00，告诉屏幕：“接下来我要发的是命令”
    LL_I2C_TransmitData8(I2C1, 0x00);
    while (!LL_I2C_IsActiveFlag_TXE(I2C1)); // 等待数据寄存器为空

    // 4. 发送实际的命令字节
    LL_I2C_TransmitData8(I2C1, cmd);
    // 【关键】：发送最后一个字节后，必须等待 BTF (字节传输完成) 标志位
    // 确保数据彻彻底底从移位寄存器发到了 I2C 总线上，防止被 Stop 信号截断
    while (!LL_I2C_IsActiveFlag_BTF(I2C1));

    // 5. 产生停止信号 (STOP)，释放 I2C 总线
    LL_I2C_GenerateStopCondition(I2C1);
}

void OLED_WriteData(uint8_t data) {
    // 1. 产生起始信号 (START)
    LL_I2C_GenerateStartCondition(I2C1);
    while (!LL_I2C_IsActiveFlag_SB(I2C1)); // 等待起始信号发送完成

    // 2. 发送设备地址，并等待单片机收到屏幕的应答 (ACK)
    LL_I2C_TransmitData8(I2C1, OLED_I2C_ADDR);
    while (!LL_I2C_IsActiveFlag_ADDR(I2C1)); // 等待地址发送完成标志位
    LL_I2C_ClearFlag_ADDR(I2C1); // 必须手动清除 ADDR 标志位，总线才会继续

    // 3. 发送控制字节 0x00，告诉屏幕：“接下来我要发的是命令”
    LL_I2C_TransmitData8(I2C1, 0x40);
    while (!LL_I2C_IsActiveFlag_TXE(I2C1)); // 等待数据寄存器为空

    // 4. 发送实际的命令字节
    LL_I2C_TransmitData8(I2C1, data);
    // 【关键】：发送最后一个字节后，必须等待 BTF (字节传输完成) 标志位
    // 确保数据彻彻底底从移位寄存器发到了 I2C 总线上，防止被 Stop 信号截断
    while (!LL_I2C_IsActiveFlag_BTF(I2C1));

    // 5. 产生停止信号 (STOP)，释放 I2C 总线
    LL_I2C_GenerateStopCondition(I2C1);
}

/**
 * @brief  设置 OLED 光标位置 (坐标定位)
 * @param  x: X 轴坐标，范围 0 ~ 127
 * @param  y: Y 轴坐标 (页)，范围 0 ~ 7 (对于 128x64 屏幕)
 */
void OLED_SetCursor(uint8_t x, uint8_t y) {
    // 1. 设置页地址 (Page Address)：0xB0 ~ 0xB7
    OLED_WriteCmd(0xB0 | y);

    // 2. 设置列地址的低 4 位 (Lower Column Address)：0x00 ~ 0x0F
    OLED_WriteCmd(0x00 | (x & 0x0F));

    // 3. 设置列地址的高 4 位 (Higher Column Address)：0x10 ~ 0x1F
    OLED_WriteCmd(0x10 | ((x & 0xF0) >> 4));
}

void OLED_ShowChar(uint8_t x, uint8_t y, char chr) {
    uint8_t i;
    uint8_t c = chr - ' '; // 计算该字符在字库数组中的偏移量 (空格的 ASCII 码是 32)

    OLED_SetCursor(x, y); // 调用坐标定位函数，设置上半部分的绘制位置
    for (i = 0; i < 8; i++) {
        OLED_WriteData(OLED_F8x16[c][i]); // 发送上半部分的 8 列像素数据
    }

    OLED_SetCursor(x, y + 1); // 坐标下移一页，设置下半部分的绘制位置
    for (i = 0; i < 8; i++) {
        OLED_WriteData(OLED_F8x16[c][i + 8]); // 发送下半部分的 8 列像素数据
    }
}
/**
 * @brief  清空屏幕上的所有内容
 */
void OLED_Clear(void)
{
    uint8_t i, n;
    for (i = 0; i < 8; i++)  // 遍历 8 页
    {
        OLED_WriteCmd(0xB0 + i); // 设置当前页
        OLED_WriteCmd(0x00);     // 设置列低地址
        OLED_WriteCmd(0x10);     // 设置列高地址
        for (n = 0; n < 128; n++) // 遍历 128 列
        {
            OLED_WriteData(0x00); // 写入 0x00 熄灭像素
        }
    }
}
/**
 * @brief  OLED 屏幕初始化 (必须在主循环前调用)
 */
void OLED_Init(void)
{
    // 注意：建议在上电后延时 100ms 左右再调用此函数，等待屏幕硬件复位稳定

    OLED_WriteCmd(0xAE); // 关闭显示 (Display OFF)

    OLED_WriteCmd(0xD5); // 设置显示时钟分频比/振荡器频率
    OLED_WriteCmd(0x80); // 默认值

    OLED_WriteCmd(0xA8); // 设置多路复用率 (Multiplex Ratio)
    OLED_WriteCmd(0x3F); // 128x64 屏幕为 0x3F (即 64-1)

    OLED_WriteCmd(0xD3); // 设置显示偏移 (Display Offset)
    OLED_WriteCmd(0x00); // 默认值为 0

    OLED_WriteCmd(0x40); // 设置显示开始行 (Start Line) 0x40~0x7F

    OLED_WriteCmd(0x8D); // 设置电荷泵 (Charge Pump)
    OLED_WriteCmd(0x14); // 开启电荷泵 (0x10 为关闭，0x14 为开启，必须开启才能亮)

    OLED_WriteCmd(0x20); // 设置内存寻址模式 (Memory Addressing Mode)
    OLED_WriteCmd(0x02); // 0x02: 页寻址模式 (Page Addressing Mode)

    OLED_WriteCmd(0xA1); // 设置段重映射 (Segment Remap) -> 左右反转：0xA0正常，0xA1反转
    OLED_WriteCmd(0xC8); // 设置 COM 输出扫描方向 -> 上下反转：0xC0正常，0xC8反转

    OLED_WriteCmd(0xDA); // 设置 COM 引脚硬件配置
    OLED_WriteCmd(0x12);

    OLED_WriteCmd(0x81); // 设置对比度 (Contrast Control)
    OLED_WriteCmd(0xCF); // 范围 0x00~0xFF，越大越亮

    OLED_WriteCmd(0xD9); // 设置预充电周期 (Pre-charge Period)
    OLED_WriteCmd(0xF1);

    OLED_WriteCmd(0xDB); // 设置 VCOMH 取消选择级别
    OLED_WriteCmd(0x40);

    OLED_WriteCmd(0xA4); // 全局显示开启 (0xA4: 恢复 RAM 内容显示，0xA5: 忽略 RAM 强制全亮)
    OLED_WriteCmd(0xA6); // 设置正常/反色显示 (0xA6: 正常，0xA7: 反色)

    OLED_Clear();        // 初始化完成后，先清空显存中的随机乱码

    OLED_WriteCmd(0xAF); // 最后，开启显示 (Display ON)
}

/**
 * @brief  在 OLED 上显示一段 ASCII 字符串
 * @param  x: 起始 X 坐标
 * @param  y: 起始 Y 坐标 (页)
 * @param  str: 要显示的字符串指针
 */
void OLED_ShowString(uint8_t x, uint8_t y, char *str)
{
    // 遍历字符串，直到遇到结束符 '\0'
    while (*str != '\0')
    {
        // 检查是否超出屏幕右边界，如果超出则换行
        if (x > 120)
        {
            x = 0;   // 回到行首
            y += 2;  // 因为 8x16 字体占 2 页，所以 Y 坐标加 2
        }

        // 检查是否超出屏幕下边界，超出则停止打印防止数组越界死机
        if (y > 7)
        {
            break;
        }

        // 调用我们之前讨论过的单个字符显示函数
        OLED_ShowChar(x, y, *str);

        // 光标向右移动，准备打印下一个字符 (8x16 字体占用 8 个像素的宽度)
        x += 8;

        // 指针偏移，指向下一个字符
        str++;
    }
}

/* USER CODE END 1 */

