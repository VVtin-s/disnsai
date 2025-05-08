/**
 ****************************************************************************************************
 * @file        sys.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.1
 * @date        2020-04-17
 * @brief       系统初始化代码(包括时钟配置/中断管理/GPIO设置等)
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32F103开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20200417
 * 第一次发布
 *
 * V1.1 20221031
 * 在sys_stm32_clock_init函数添加相关复位/置位代码,关闭非必要外设,避免部分例程异常
 *
 ****************************************************************************************************
 */

#include "sys.h"


/**
 * @brief       设置中断向量表偏移地址
 * @param       baseaddr: 基址
 * @param       offset: 偏移量(必须是0, 或者0X100的倍数)
 * @retval      无
 */
void sys_nvic_set_vector_table(uint32_t baseaddr, uint32_t offset)
{
    /* 设置NVIC的向量表偏移寄存器,VTOR低9位保留,即[8:0]保留 */
    SCB->VTOR = baseaddr | (offset & (uint32_t)0xFFFFFE00);
}

/**
 * @brief       设置NVIC分组
 * @param       group: 0~4,共5组, 详细解释见: sys_nvic_init函数参数说明
 * @retval      无
 */
static void sys_nvic_priority_group_config(uint8_t group)
{
    uint32_t temp, temp1;
    temp1 = (~group) & 0x07;/* 取后三位 */
    temp1 <<= 8;
    temp = SCB->AIRCR;      /* 读取先前的设置 */
    temp &= 0X0000F8FF;     /* 清空先前分组 */
    temp |= 0X05FA0000;     /* 写入钥匙 */
    temp |= temp1;
    SCB->AIRCR = temp;      /* 设置分组 */
}

/**
 * @brief       设置NVIC(包括分组/抢占优先级/子优先级等)
 * @param       pprio: 抢占优先级(PreemptionPriority)
 * @param       sprio: 子优先级(SubPriority)
 * @param       ch: 中断编号(Channel)
 * @param       group: 中断分组
 *   @arg       0, 组0: 0位抢占优先级, 4位子优先级
 *   @arg       1, 组1: 1位抢占优先级, 3位子优先级
 *   @arg       2, 组2: 2位抢占优先级, 2位子优先级
 *   @arg       3, 组3: 3位抢占优先级, 1位子优先级
 *   @arg       4, 组4: 4位抢占优先级, 0位子优先级
 * @note        注意优先级不能超过设定的组的范围! 否则会有意想不到的错误
 * @retval      无
 */
void sys_nvic_init(uint8_t pprio, uint8_t sprio, uint8_t ch, uint8_t group)
{
    uint32_t temp;
    sys_nvic_priority_group_config(group);  /* 设置分组 */
    temp = pprio << (4 - group);
    temp |= sprio & (0x0f >> group);
    temp &= 0xf;                            /* 取低四位 */
    NVIC->ISER[ch / 32] |= 1 << (ch % 32);  /* 使能中断位(要清除的话,设置ICER对应位为1即可) */
    NVIC->IP[ch] |= temp << 4;              /* 设置响应优先级和抢断优先级 */
}

/**
 * @brief       外部中断配置函数, 只针对GPIOA~GPIOG
 * @note        该函数会自动开启对应中断, 以及屏蔽线
 * @param       p_gpiox: GPIOA~GPIOG, GPIO指针
 * @param       pinx: 0X0000~0XFFFF, 引脚位置, 每个位代表一个IO, 第0位代表Px0, 第1位代表Px1, 依次类推. 比如0X0101, 代表同时设置Px0和Px8.
 *   @arg       SYS_GPIO_PIN0~SYS_GPIO_PIN15, 1<<0 ~ 1<<15
 * @param       tmode: 1~3, 触发模式
 *   @arg       SYS_GPIO_FTIR, 1, 下降沿触发
 *   @arg       SYS_GPIO_RTIR, 2, 上升沿触发
 *   @arg       SYS_GPIO_BTIR, 3, 任意电平触发
 * @retval      无
 */
void sys_nvic_ex_config(GPIO_TypeDef *p_gpiox, uint16_t pinx, uint8_t tmode)
{
    uint8_t offset;
    uint32_t gpio_num = 0;      /* gpio编号, 0~10, 代表GPIOA~GPIOG */
    uint32_t pinpos = 0, pos = 0, curpin = 0;

    gpio_num = ((uint32_t)p_gpiox - (uint32_t)GPIOA) / 0X400 ;/* 得到gpio编号 */
    RCC->APB2ENR |= 1 << 0;     /* AFIO = 1,使能AFIO时钟 */

    for (pinpos = 0; pinpos < 16; pinpos++)
    {
        pos = 1 << pinpos;      /* 一个个位检查 */
        curpin = pinx & pos;    /* 检查引脚是否要设置 */

        if (curpin == pos)      /* 需要设置 */
        {
            offset = (pinpos % 4) * 4;
            AFIO->EXTICR[pinpos / 4] &= ~(0x000F << offset);    /* 清除原来设置！！！ */
            AFIO->EXTICR[pinpos / 4] |= gpio_num << offset;     /* EXTI.BITx映射到gpiox.bitx */

            EXTI->IMR |= 1 << pinpos;   /* 开启line BITx上的中断(如果要禁止中断，则反操作即可) */

            if (tmode & 0x01) EXTI->FTSR |= 1 << pinpos;        /* line bitx上事件下降沿触发 */

            if (tmode & 0x02) EXTI->RTSR |= 1 << pinpos;        /* line bitx上事件上升降沿触发 */
        }
    }
}

/**
 * @brief       GPIO重映射功能选择设置
 *   @note      这里仅支持对MAPR寄存器的配置, 不支持对MAPR2寄存器的配置!!!
 * @param       pos: 在AFIO_MAPR寄存器里面的起始位置, 0~24
 *   @arg       [0]    , SPI1_REMAP;         [1]    , I2C1_REMAP;         [2]    , USART1_REMAP;        [3]    , USART2_REMAP;
 *   @arg       [5:4]  , USART3_REMAP;       [7:6]  , TIM1_REMAP;         [9:8]  , TIM2_REMAP;          [11:10], TIM3_REMAP;
 *   @arg       [12]   , TIM4_REMAP;         [14:13], CAN_REMAP;          [15]   , PD01_REMAP;          [16]   , TIM15CH4_REMAP;
 *   @arg       [17]   , ADC1_ETRGINJ_REMAP; [18]   , ADC1_ETRGREG_REMAP; [19]   , ADC2_ETRGINJ_REMAP;  [20]   , ADC2_ETRGREG_REMAP;
 *   @arg       [26:24], SWJ_CFG;
 * @param       bit: 占用多少位, 1 ~ 3, 详见pos参数说明
 * @param       val: 要设置的复用功能, 0 ~ 4, 得根据pos位数决定, 详细的设置值, 参见: <<STM32中文参考手册 V10>> 8.4.2节, 对MAPR寄存器的说明
 *              如: sys_gpio_remap_set(24, 3, 2); 则是设置SWJ_CFG[2:0]    = 2, 选择关闭JTAG, 开启SWD.
 *                  sys_gpio_remap_set(10, 2, 2); 则是设置TIM3_REMAP[1:0] = 2, TIM3选择部分重映射, CH1->PB4, CH2->PB5, CH3->PB0, CH4->PB1
 * @retval      无
 */
void sys_gpio_remap_set(uint8_t pos, uint8_t bit, uint8_t val)
{
    uint32_t temp = 0;
    uint8_t i = 0;
    RCC->APB2ENR |= 1 << 0;     /* 开启辅助时钟 */

    for (i = 0; i < bit; i++)   /* 填充bit个1 */
    {
        temp <<= 1;
        temp += 1;
    }

    AFIO->MAPR &= ~(temp << pos);       /* 清除MAPR对应位置原来的设置 */
    AFIO->MAPR |= (uint32_t)val << pos; /* 设置MAPR对应位置的值 */
}

/**
 * @brief       GPIO通用设置
 * @param       p_gpiox: GPIOA~GPIOG, GPIO指针
 * @param       pinx: 0X0000~0XFFFF, 引脚位置, 每个位代表一个IO, 第0位代表Px0, 第1位代表Px1, 依次类推. 比如0X0101, 代表同时设置Px0和Px8.
 *   @arg       SYS_GPIO_PIN0~SYS_GPIO_PIN15, 1<<0 ~ 1<<15
 *
 * @param       mode: 0~3; 模式选择, 设置如下:
 *   @arg       SYS_GPIO_MODE_IN,  0, 输入模式(系统复位默认状态)
 *   @arg       SYS_GPIO_MODE_OUT, 1, 输出模式
 *   @arg       SYS_GPIO_MODE_AF,  2, 复用功能模式
 *   @arg       SYS_GPIO_MODE_AIN, 3, 模拟输入模式
 *
 * @param       otype: 0 / 1; 输出类型选择, 设置如下:
 *   @arg       SYS_GPIO_OTYPE_PP, 0, 推挽输出
 *   @arg       SYS_GPIO_OTYPE_OD, 1, 开漏输出
 *
 * @param       ospeed: 0~2; 输出速度, 设置如下(注意: 不能为0!!):
 *   @arg       SYS_GPIO_SPEED_LOW,  2, 低速
 *   @arg       SYS_GPIO_SPEED_MID,  1, 中速
 *   @arg       SYS_GPIO_SPEED_HIGH, 3, 高速
 *
 * @param       pupd: 0~3: 上下拉设置, 设置如下:
 *   @arg       SYS_GPIO_PUPD_NONE, 0, 不带上下拉
 *   @arg       SYS_GPIO_PUPD_PU,   1, 上拉
 *   @arg       SYS_GPIO_PUPD_PD,   2, 下拉
 *   @arg       SYS_GPIO_PUPD_RES,  3, 保留
 *
 * @note:       注意:
 *              1, 在输入模式(普通输入/模拟输入)下, otype 和 ospeed 参数无效!!
 *              2, 在输出模式下, pupd 参数无效!!(开漏输出无法使用内部上拉电阻!!)
 * @retval      无
 */
void sys_gpio_set(GPIO_TypeDef *p_gpiox, uint16_t pinx, uint32_t mode, uint32_t otype, uint32_t ospeed, uint32_t pupd)
{
    uint32_t pinpos = 0, pos = 0, curpin = 0;
    uint32_t config = 0;        /* 用于保存某一个IO的设置(CNF[1:0] + MODE[1:0]),只用了其最低4位 */

    for (pinpos = 0; pinpos < 16; pinpos++)
    {
        pos = 1 << pinpos;      /* 一个个位检查 */
        curpin = pinx & pos;    /* 检查引脚是否要设置 */

        if (curpin == pos)      /* 需要设置 */
        {
            config = 0;         /* bit0~3都设置为0, 即CNF[1:0] = 0; MODE[1:0] = 0;  默认是模拟输入模式 */

            if ((mode == 0X01) || (mode == 0X02))   /* 如果是普通输出模式/复用功能模式 */
            {
                config = ospeed & 0X03;             /* 设置bit0/1 MODE[1:0] = 2/1/3 速度参数 */
                config |= (otype & 0X01) << 2;      /* 设置bit2   CNF[0]    = 0/1   推挽/开漏输出 */
                config |= (mode - 1) << 3;          /* 设置bit3   CNF[1]    = 0/1   普通/复用输出 */
            }
            else if (mode == 0)     /* 如果是普通输入模式 */
            {
                if (pupd == 0)   /* 不带上下拉,即浮空输入模式 */
                {
                    config = 1 << 2;               /* 设置bit2/3 CNF[1:0] = 01   浮空输入模式 */
                }
                else
                {
                    config = 1 << 3;                            /* 设置bit2/3 CNF[1:0] = 10   上下拉输入模式 */
                    p_gpiox->ODR &= ~(1 << pinpos);             /* 清除原来的设置 */
                    p_gpiox->ODR |= (pupd & 0X01) << pinpos;    /* 设置ODR = 0/1 下拉/上拉 */
                }
            }

            /* 根据IO口位置 设置CRL / CRH寄存器 */
            if (pinpos <= 7)
            {
                p_gpiox->CRL &= ~(0X0F << (pinpos * 4));        /* 清除原来的设置 */
                p_gpiox->CRL |= config << (pinpos * 4);         /* 设置CNFx[1:0] 和 MODEx[1:0], x = pinpos = 0~7 */
            }
            else
            {
                p_gpiox->CRH &= ~(0X0F << ((pinpos - 8) * 4));  /* 清除原来的设置 */
                p_gpiox->CRH |= config << ((pinpos - 8) * 4);   /* 设置CNFx[1:0] 和 MODEx[1:0], x = pinpos = 8~15 */

            }
        }
    }
}

/**
 * @brief       设置GPIO某个引脚的输出状态
 * @param       p_gpiox: GPIOA~GPIOG, GPIO指针
 * @param       0X0000~0XFFFF, 引脚位置, 每个位代表一个IO, 第0位代表Px0, 第1位代表Px1, 依次类推. 比如0X0101, 代表同时设置Px0和Px8.
 *   @arg       SYS_GPIO_PIN0~SYS_GPIO_PIN15, 1<<0 ~ 1<<15
 * @param       status: 0/1, 引脚状态(仅最低位有效), 设置如下:
 *   @arg       0, 输出低电平
 *   @arg       1, 输出高电平
 * @retval      无
 */
void sys_gpio_pin_set(GPIO_TypeDef *p_gpiox, uint16_t pinx, uint8_t status)
{
    if (status & 0X01)
    {
        p_gpiox->BSRR |= pinx;  /* 设置GPIOx的pinx为1 */
    }
    else
    {
        p_gpiox->BSRR |= (uint32_t)pinx << 16;  /* 设置GPIOx的pinx为0 */
    }
}

/**
 * @brief       读取GPIO某个引脚的状态
 * @param       p_gpiox: GPIOA~GPIOG, GPIO指针
 * @param       0X0000~0XFFFF, 引脚位置, 每个位代表一个IO, 第0位代表Px0, 第1位代表Px1, 依次类推. 比如0X0101, 代表同时设置Px0和Px8.
 *   @arg       SYS_GPIO_PIN0~SYS_GPIO_PIN15, 1<<0 ~ 1<<15
 * @retval      返回引脚状态, 0, 低电平; 1, 高电平
 */
uint8_t sys_gpio_pin_get(GPIO_TypeDef *p_gpiox, uint16_t pinx)
{
    if (p_gpiox->IDR & pinx)
    {
        return 1;   /* pinx的状态为1 */
    }
    else
    {
        return 0;   /* pinx的状态为0 */
    }
}

/**
 * @brief       执行: WFI指令(执行完该指令进入低功耗状态, 等待中断唤醒)
 * @param       无
 * @retval      无
 */
void sys_wfi_set(void)
{
    __ASM volatile("wfi");
}

/**
 * @brief       关闭所有中断(但是不包括fault和NMI中断)
 * @param       无
 * @retval      无
 */
void sys_intx_disable(void)
{
    __ASM volatile("cpsid i");
}

/**
 * @brief       开启所有中断
 * @param       无
 * @retval      无
 */
void sys_intx_enable(void)
{
    __ASM volatile("cpsie i");
}

/**
 * @brief       设置栈顶地址
 * @note        左侧的红X, 属于MDK误报, 实际是没问题的
 * @param       addr: 栈顶地址
 * @retval      无
 */
void sys_msr_msp(uint32_t addr)
{
    __set_MSP(addr);    /* 设置栈顶地址 */
}

/**
 * @brief       进入待机模式
 * @param       无
 * @retval      无
 */
void sys_standby(void)
{
    RCC->APB1ENR |= 1 << 28;    /* 使能电源时钟 */
    PWR->CSR |= 1 << 8;         /* 设置WKUP用于唤醒 */
    PWR->CR |= 1 << 2;          /* 清除WKUP 标志 */
    PWR->CR |= 1 << 1;          /* PDDS = 1, 允许进入深度睡眠模式(PDDS) */
    SCB->SCR |= 1 << 2;         /* 使能SLEEPDEEP位 (SYS->CTRL) */
    sys_wfi_set();              /* 执行WFI指令, 进入待机模式 */
}

/**
 * @brief       系统软复位
 * @param       无
 * @retval      无
 */
void sys_soft_reset(void)
{
    SCB->AIRCR = 0X05FA0000 | (uint32_t)0x04;
}

/**
 * @brief       时钟设置函数
 * @param       plln: PLL倍频系数(PLL倍频), 取值范围: 2~16
 * @note
 *
 *              PLLCLK: PLL输出时钟
 *              PLLSRC: PLL输入时钟频率, 可以是 HSI/2, HSE/2, HSE等, 一般选择HSE.
 *              SYSCLK: 系统时钟, 可选来自 HSI/PLLCLK/HSE, 一般选择来自PLLCLK
 *              FCLK  : Cortex M3内核时钟, 等于HCLK
 *              HCLK  : AHB总线时钟, 来自 SYSCLK 的分频, 可以是1...512分频, 一般不分频
 *              PCLK2 : APB2总线时钟, 来自 HCLK 的分频(最大72Mhz), 可以是1/2/4/8/16分频, 一般不分频
 *              PCLK1 : APB1总线时钟, 来自 HCLK 的分频(最大36Mhz), 可以是1/2/4/8/16分频, 一般二分频
 *
 *              PLLCLK = PLLSRC * plln;
 *              FCLK = HCLK = SYSCLK;
 *              PCLK2 = HCLK;
 *              PCLK1 = HCLK / 2;
 *
 *              我们一般选择PLLSRC来自HSE, 即来自外部晶振.
 *              当外部晶振为 8M的时候, 推荐: plln = 9, AHB不分频, 得到:
 *              PLLCLK = 8 * 9 = 72Mhz
 *              FCLK = HCLK = SYSCLK = PLLCLK / 1 = 72Mhz
 *              PCLK2 = HCLK = 72Mhz
 *              PCLK1 = HCLK / 2 = 36Mhz
 *
 *              关于STM32F103的PLL说明详见: <<STM32中文参考手册 V10>>第六章相关内容
 *
 * @retval      错误代码: 0, 成功; 1, HSE错误;
 */
uint8_t sys_clock_set(uint32_t plln)
{
    uint32_t retry = 0;
    uint8_t retval = 0;
    RCC->CR |= 0x00010000;          /* 外部高速时钟使能HSEON */

    while (retry < 0XFFF0)
    {
        __nop();

        /* 注意, MDK5.29或以后版本, 在使能HSEON以后, 如果不加一定的延时
         * 再开始其他配置, 会导致仿真器下载完代码, 延时函数运行不正常的 bug
         * 需要按复位按键, 延时才会正常, 在这里加一定的延时, 可以解决这个 bug
         * 这里, 我们设置的延时时间, 至少是 0X8000 个 nop时间
         */
        if (RCC->CR & (1 << 17) && retry > 0X8000)
        {
            break;
        }

        retry++;        /* 等待HSE RDY */
    }

    if (retry >= 0XFFF0)
    {
        retval = 1;     /* HSE无法就绪 */
    }
    else
    {
        RCC->CFGR = 0X00000400;     /* PCLK1 = HCLK / 2; PCLK2 = HCLK; HCLK = SYSCLK; */
        plln -= 2;                  /* 抵消2个单位(因为是从2开始的, 设置0就是2) */
        RCC->CFGR |= plln << 18;    /* 设置PLL值 2~16 */
        RCC->CFGR |= 1 << 16;       /* PLLSRC = 1, 选择 HSE 作为 PLL 输入时钟 */

        /* FLASH_ACR寄存器的描述详见: <<STM32F10xx闪存编程手册>> */
        FLASH->ACR = 1 << 4;        /* PRFTBE = 1 开启预取缓冲区 */
        FLASH->ACR |= 2 << 0;       /* LATENCY[2:0] = 2 FLASH两个等待周期 */

        RCC->CR |= 1 << 24;         /* PLLON = 1, 使能PLL */

        while (!(RCC->CR >> 25));   /* 等待PLL锁定 */

        RCC->CFGR |= 2 << 0;        /* SW[1:0] = 2, 选择PLL输出作为系统时钟 */

        while (((RCC->CFGR >> 2) & 0X03) != 2); /* 等待PLL作为系统时钟设置成功 */
    }

    return retval;
}

/**
 * @brief       系统时钟初始化函数
 * @param       plln: PLL倍频系数(PLL倍频), 取值范围: 2~16
 * @retval      无
 */
void sys_stm32_clock_init(uint32_t plln)
{
    RCC->APB1RSTR = 0x00000000;     /* 复位结束 */
    RCC->APB2RSTR = 0x00000000;
    
    RCC->AHBENR = 0x00000014;       /* 睡眠模式闪存和SRAM时钟使能.其他关闭 */
    RCC->APB2ENR = 0x00000000;      /* 外设时钟关闭 */
    RCC->APB1ENR = 0x00000000;
    
    RCC->CR |= 0x00000001;          /* 使能内部高速时钟HSION */
    RCC->CFGR &= 0xF8FF0000;        /* 复位SW[1:0], SWS[1:0], HPRE[3:0], PPRE1[2:0], PPRE2[2:0], ADCPRE[1:0], MCO[2:0] */
    RCC->CR &= 0xFEF6FFFF;          /* 复位HSEON, CSSON, PLLON */
    RCC->CR &= 0xFFFBFFFF;          /* 复位HSEBYP */
    RCC->CFGR &= 0xFF80FFFF;        /* 复位PLLSRC, PLLXTPRE, PLLMUL[3:0] 和 USBPRE/OTGFSPRE */
    RCC->CIR = 0x009F0000;          /* 关闭所有RCC中断并清除中断标志 */

    sys_clock_set(plln);            /* 设置时钟 */

    /* 配置中断向量偏移 */
#ifdef  VECT_TAB_RAM
    sys_nvic_set_vector_table(SRAM_BASE, 0x0);
#else
    sys_nvic_set_vector_table(FLASH_BASE, 0x0);
#endif
}


/**********************************************************
* @ File name -> sys.c
* @ Version   -> V1.0
* @ Date      -> 12-26-2013
* @ Brief     -> 系统设置相关的函数
**********************************************************/


/**********************************************************
* 函数功能 ---> 读取芯片闪存容量
* 入口参数 ---> *STMCapa：容量字符显示缓存
* 返回数值 ---> 容量（十进制）
* 功能说明 ---> none
**********************************************************/
void STM32_Flash_Capacity(uint8_t *STMCapa)
{
	uint16_t capa;
	
	capa = *((uint16_t*)0x1ffff7e0);	//读取闪存容量寄存器，低16位有效
	
	capa = ((capa >> 12) * 4096 + ((capa >> 8) & 0x0f) * 256 + ((capa >> 4) & 0x0f) * 16 + (capa & 0x0f));	//转换成十进制
	
	STMCapa[0] = 0x4d;	//M
	STMCapa[1] = 0x43;	//C
	STMCapa[2] = 0x55;	//U
	STMCapa[3] = 0x43;	//C
	STMCapa[4] = 0x61;	//a
	STMCapa[5] = 0x70;	//p
	STMCapa[6] = 0x3a;	//:
	
	if((capa / 1000) != 0)	STMCapa[7] = capa / 1000 + 48;	//千位不为0时显示
	else	STMCapa[7] = 0x20;
	
	STMCapa[8] = capa % 1000 / 100 + 48;	//百位
	STMCapa[9] = capa % 100 /10 + 48;		//十位
	STMCapa[10] = capa % 10 + 48;			//个位
	STMCapa[11] = 0x4b;	//K
	STMCapa[12] = 0x62;	//b
}
/**********************************************************
* 函数功能 ---> 读取CPUID
* 入口参数 ---> none
* 返回数值 ---> CPUID（十六进制）
* 功能说明 ---> none
**********************************************************/
void STM32_CPUID(uint8_t *IDbuff)
{
	uint32_t CPUID;
	CPUID = *((uint32_t*)0xe000ed00);
	sprintf((char*)IDbuff, "CPU ID:%08X", CPUID);
}
/**********************************************************
* 函数功能 ---> 设置向量表偏移地址
* 入口参数 ---> NVIC_VectTab：基址
*              Offset：偏移量	
* 返回数值 ---> 容量（十进制）
* 功能说明 ---> none
**********************************************************/	 
void MY_NVIC_SetVectorTable(uint32_t NVIC_VectTab,uint32_t Offset)	 
{ 	   	 
	SCB->VTOR = NVIC_VectTab | (Offset & (uint32_t)0x1fffff80);//设置NVIC的向量表偏移寄存器
	//用于标识向量表是在CODE区还是在RAM区
}
/**********************************************************
* 函数功能 ---> 设置中断分组
* 入口参数 ---> NVIC_PriorityGroup: 中断分组
* 返回数值 ---> none
* 功能说明 ---> 0 ~ 4组，共计有5组
**********************************************************/
void MY_NVIC_PriorityGroup_Config(uint32_t NVIC_PriorityGroup)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup);	//设置中断分组
	
//	uint32_t temp,temp1;
//	
//	temp1 = (~NVIC_PriorityGroup) & 0x00000007;//取后三位
//	temp1 <<= 8;
//	temp = SCB->AIRCR;  //读取先前的设置
//	temp &= 0x0000f8ff; //清空先前分组
//	temp |= 0x05fa0000; //写入钥匙
//	temp |= temp1;	   
//	SCB->AIRCR = temp;  //设置分组
}
/**********************************************************
* 函数功能 ---> 设置中断分组优先级
* 入口参数 ---> NVIC_PreemptionPriority：抢先优先级
*               NVIC_Subpriority：响应优先级
*               NVIC_Channel：中断编号
*               NVIC_Group: 中断分组
* 返回数值 ---> none
* 功能说明 ---> 1、组划分:
*                  组0：0位抢占优先级，4位响应优先级
*                  组1：1位抢占优先级，3位响应优先级
*                  组2：2位抢占优先级，2位响应优先级
*                  组3：3位抢占优先级，1位响应优先级
*                  组4：4位抢占优先级，0位响应优先级
*               2、抢先优先级参数和响应优先级参数原则上是数值越小优先级越高
**********************************************************/
void MY_NVIC_Init(uint8_t NVIC_PreemptionPriority,uint8_t NVIC_Subpriority,uint8_t NVIC_Channel,uint32_t NVIC_Group)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	MY_NVIC_PriorityGroup_Config(NVIC_Group);	//设置中断分组	
	
	NVIC_InitStructure.NVIC_IRQChannel = NVIC_Channel;								//设置中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PreemptionPriority;	//抢先优先级  	 	 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = NVIC_Subpriority;				//响应优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;									//使能中断

	NVIC_Init(&NVIC_InitStructure);	//初始化中断
	
//	uint32_t temp;	
//	uint8_t IPRADDR=NVIC_Channel/4;  //每组只能存4个,得到组地址 
//	uint8_t IPROFFSET=NVIC_Channel%4;//在组内的偏移
//	
//	IPROFFSET = IPROFFSET*8 + 4;    //得到偏移的确切位置
//	
//	MY_NVIC_PriorityGroup_Config(NVIC_Group);//设置分组
//	
//	temp  = NVIC_PreemptionPriority << (4 - NVIC_Group);	//抢先优先级	  
//	temp |= NVIC_Subpriority & (0x0f >> NVIC_Group);	//相应优先级
//	temp &= 0xf;//取低四位

//	if(NVIC_Channel < 32)	NVIC->ISER[0] |= 1 << NVIC_Channel;//使能中断位(要清除的话,相反操作就OK)
//	else	NVIC->ISER[1]| |= 1 << (NVIC_Channel - 32); 
//	
//	NVIC->IPR[IPRADDR] |= temp << IPROFFSET;//设置响应优先级和抢断优先级  
}
/**********************************************************
* 函数功能 ---> THUMB指令不支持汇编内联
* 入口参数 ---> none
* 返回数值 ---> none
* 功能说明 ---> 采用如下方法实现执行汇编指令WFI
**********************************************************/
__asm void WFI_SET(void)
{
	WFI;    
}
/**********************************************************
* 函数功能 ---> 所有时钟寄存器复位
* 入口参数 ---> none
* 返回数值 ---> none
* 功能说明 ---> 不能在这里执行所有外设复位!否则至少引起串口不工作
**********************************************************/
void MY_RCC_DeInit(void)
{										  					   
	RCC->APB1RSTR = 0x00000000;	//复位结束			 
	RCC->APB2RSTR = 0x00000000; 
	  
  	RCC->AHBENR   = 0x00000014;	//睡眠模式闪存和SRAM时钟使能.其他关闭.	  
  	RCC->APB2ENR  = 0x00000000;	//外设时钟关闭.			   
  	RCC->APB1ENR  = 0x00000000;   
	RCC->CR      |= 0x00000001;	//使能内部高速时钟HSION	 															 
	RCC->CFGR    &= 0xf8ff0000;	//复位SW[1:0],HPRE[3:0],PPRE1[2:0],PPRE2[2:0],ADCPRE[1:0],MCO[2:0]					 
	RCC->CR      &= 0xfef6ffff;	//复位HSEON,CSSON,PLLON
	RCC->CR      &= 0xfffbffff;	//复位HSEBYP	   	  
	RCC->CFGR    &= 0xff80ffff;	//复位PLLSRC, PLLXTPRE, PLLMUL[3:0] and USBPRE 
	RCC->CIR      = 0x00000000;	//关闭所有中断
	
	//配置向量表				  
#ifdef  VECT_TAB_RAM
	MY_NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else   
	MY_NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif
}
/**********************************************************
* 函数功能 ---> 设置芯片进入待机模式
* 入口参数 ---> none
* 返回数值 ---> none
* 功能说明 ---> 利用WKUP引脚唤醒（PA.0）
**********************************************************/
void SYS_Standby(void)
{
	SCB->SCR |= 1 << 2;			//使能sleep deep（SYS->CTRL）
	RCC->APB1ENR |= 1 << 28;	//电源接口时钟开启
	PWR->CSR |= 1 << 8;			//设置WKUP引脚用于唤醒
	PWR->CR |= 1 << 2;			//清除WAKE-UP标志
	PWR->CR |= 1 << 1;			//PDDS置位，掉电深睡眠
	WFI_SET();					//执行WFI指令
}
/**********************************************************
* 函数功能 ---> 系统软复位
* 入口参数 ---> none
* 返回数值 ---> none
* 功能说明 ---> none
**********************************************************/
void SYS_SoftReset(void)
{
	SCB->AIRCR = 0x05fa0000 | 0x00000004;
}
/**********************************************************
* 函数功能 ---> JTAG模式设置
* 入口参数 ---> mode：模式参数
*                    000：JTAG-DP + SW-DP（复位状态）
*                    001：JTAG-DP + SW-DP（除了JNTRST引脚，释放JRST引脚）
*                    010：JTAG-DP接口禁止，SW-DP接口允许
*                    100：JTAG-DP接口和SW-DP接口都禁止
*                    xxx：其他值，禁止
* 返回数值 ---> none
* 功能说明 ---> none
**********************************************************/
void STM_JTAG_Set(uint32_t mode)
{
	RCC->APB2ENR |= 1 << 0;		//使能辅助时钟
	AFIO->MAPR &= 0xf8ffffff;	//清除SWJ_CFG[2:0]位，即【26:24】
	AFIO->MAPR |= (mode << 24);	//设置JTAG模式
}
/**********************************************************
* 函数功能 ---> 系统时钟初始化
* 入口参数 ---> pll：倍频数。取值范围：2 ~ 16
* 返回数值 ---> none
* 功能说明 ---> none
**********************************************************/
void STM_Clock_Init(uint8_t pll)
{
	uint8_t tmp = 0;
	
	MY_RCC_DeInit();	//复位并配置向量表，并且将外部中断和外设时钟全部关闭
	
	RCC->CR |= 0x00010000;  //外部高速时钟使能HSEON
	
	while(!(RCC->CR >> 17));//等待外部时钟就绪
	
	RCC->CFGR = 0X00000400; //APB1=DIV2;APB2=DIV1;AHB=DIV1;
	pll -= 2;//抵消2个单位
	RCC->CFGR |= pll << 18;   //设置PLL值 2~16
	RCC->CFGR |= 1 << 16;	  //PLLSRC ON 
	FLASH->ACR |= 0x32;	  //FLASH 2个延时周期

	RCC->CR |= 0x01000000;  //PLLON
	
	while(!(RCC->CR >> 25));//等待PLL锁定
	
	RCC->CFGR |= 0x00000002;//PLL作为系统时钟，最后才能开启PLL，因为设定PLL时，PLL相关位必须在关闭状态下进行
	
	while(tmp != 0x02)     //等待PLL作为系统时钟设置成功
	{   
		tmp  = RCC->CFGR >> 2;
		tmp &= 0x03;
	}    
}
/**********************************************************
* 函数功能 ---> BCD码转为HEX
* 入口参数 ---> BCD_Data：要转换的BCD数据
* 返回数值 ---> HEX码
* 功能说明 ---> none
**********************************************************/	
uint8_t BCD_to_HEX(uint8_t BCD_Data)
{
	return((BCD_Data / 10) << 4 | (BCD_Data % 10));
}
/**********************************************************
* 函数功能 ---> HEX码转为BCD
* 入口参数 ---> HEX_Data：要转换的BCD数据
* 返回数值 ---> BCD码
* 功能说明 ---> none
**********************************************************/	
uint8_t HEX_to_BCD(uint8_t HEX_Data)
{
	return((HEX_Data >> 4) * 10 + (HEX_Data & 0x0f));
}
/**********************************************************
* 函数功能 ---> 10进制码转为16进制
* 入口参数 ---> DX_Data：要转换的10进制数据
* 返回数值 ---> 16进制
* 功能说明 ---> none
**********************************************************/
uint16_t DX_to_HX(uint16_t DX_Data)
{
	return(((DX_Data/1000)<<12) | ((DX_Data%1000/100)<<8) | ((DX_Data%100/10)<<4) | (DX_Data%10));
}
/**********************************************************
* 函数功能 ---> 16进制码转为10进制
* 入口参数 ---> HX_Data：要转换的16进制数据
* 返回数值 ---> 10进制
* 功能说明 ---> none
**********************************************************/
uint16_t HX_to_DX(uint16_t HX_Data)
{
	return((HX_Data>>12)*1000+((HX_Data&0x0f00)>>8)*100+((HX_Data&0x00f0)>>4)*10+(HX_Data&0x000f));
}	

///**********************************************************
//* 函数功能 ---> 初始化数据列表
//* 入口参数 ---> *LIST：列表指针
//* 返回数值 ---> none
//* 功能说明 ---> none
//**********************************************************/
//void Sqlist_Init(Sqlist *LIST)
//{
//	LIST->elem = (uint16_t*)malloc(MaxSize * sizeof(ElemType));
//	//分配一个长度为MaxSize * sizeof(ElemType)大小的内存空间
//	if(!LIST->elem)	return;	//没有生成数据列表，直接退出
//	//分配成功
//	LIST->length = 0;	//列表中没内容
//	LIST->listsize = MaxSize;	//该数据表占用内存大小为MaxSize（以sizeof(ElemType)为单位）
//}
///**********************************************************
//* 函数功能 ---> 复位数据列表
//* 入口参数 ---> none
//* 返回数值 ---> none
//* 功能说明 ---> none
//**********************************************************/
//void Sqlist_DeInit(void)
//{
//	Sqlist *list;
//
//	list->elem = 0;	//首地址清零
//	list->length = 0;	//长度清零
//	list->listsize = 0;	//列表大小为0
//}
///**********************************************************
//* 函数功能 ---> 向一个动态的数据列表插入一个元素
//* 入口参数 ---> *L：列表指针
//*               i：列表中第i个位置插入元素
//*               item：在第i个位置所插入的元素
//* 返回数值 ---> none
//* 功能说明 ---> none
//**********************************************************/
//void InsertElem(Sqlist *L,uint16_t i,ElemType item)
//{	/* 向顺序列表*L的第i个位置插入元素item */
//	ElemType *base, *insertPtr, *p;
//
//	if(i < 1 || i > L->length + 1)	return;	//非法插入
//	if(L->length >= L->listsize)	//在数据列表最后一个位置插入元素
//	{	//追加内存空间
//		base = (ElemType*)realloc(L->elem,(L->listsize + 10) * sizeof(ElemType));
//		L->elem = base;	//更新内存基地址
//		L->listsize = L->listsize + 100;	//存储空间增加100个单元
//	}
//	insertPtr = &(L->Elem[i - 1]);	//insertPtr为插入位置
//	for(p = &(L->elem[L->length - 1]);p >= insertPtr;p--)
//		*(p + 1) = *p;	//将i - 1以后的元素顺序向后移一个元素位置
//	*insertPtr = item;	//在第i个位置上插入元素item
//	L->length++;	//表长加1
//}








