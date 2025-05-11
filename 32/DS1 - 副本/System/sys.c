/**
 ****************************************************************************************************
 * @file        sys.c
 * @author      ����ԭ���Ŷ�(ALIENTEK)
 * @version     V1.1
 * @date        2020-04-17
 * @brief       ϵͳ��ʼ������(����ʱ������/�жϹ���/GPIO���õ�)
 * @license     Copyright (c) 2020-2032, ������������ӿƼ����޹�˾
 ****************************************************************************************************
 * @attention
 *
 * ʵ��ƽ̨:����ԭ�� STM32F103������
 * ������Ƶ:www.yuanzige.com
 * ������̳:www.openedv.com
 * ��˾��ַ:www.alientek.com
 * �����ַ:openedv.taobao.com
 *
 * �޸�˵��
 * V1.0 20200417
 * ��һ�η���
 *
 * V1.1 20221031
 * ��sys_stm32_clock_init���������ظ�λ/��λ����,�رշǱ�Ҫ����,���ⲿ�������쳣
 *
 ****************************************************************************************************
 */

#include "sys.h"


/**
 * @brief       �����ж�������ƫ�Ƶ�ַ
 * @param       baseaddr: ��ַ
 * @param       offset: ƫ����(������0, ����0X100�ı���)
 * @retval      ��
 */
void sys_nvic_set_vector_table(uint32_t baseaddr, uint32_t offset)
{
    /* ����NVIC��������ƫ�ƼĴ���,VTOR��9λ����,��[8:0]���� */
    SCB->VTOR = baseaddr | (offset & (uint32_t)0xFFFFFE00);
}

/**
 * @brief       ����NVIC����
 * @param       group: 0~4,��5��, ��ϸ���ͼ�: sys_nvic_init��������˵��
 * @retval      ��
 */
static void sys_nvic_priority_group_config(uint8_t group)
{
    uint32_t temp, temp1;
    temp1 = (~group) & 0x07;/* ȡ����λ */
    temp1 <<= 8;
    temp = SCB->AIRCR;      /* ��ȡ��ǰ������ */
    temp &= 0X0000F8FF;     /* �����ǰ���� */
    temp |= 0X05FA0000;     /* д��Կ�� */
    temp |= temp1;
    SCB->AIRCR = temp;      /* ���÷��� */
}

/**
 * @brief       ����NVIC(��������/��ռ���ȼ�/�����ȼ���)
 * @param       pprio: ��ռ���ȼ�(PreemptionPriority)
 * @param       sprio: �����ȼ�(SubPriority)
 * @param       ch: �жϱ��(Channel)
 * @param       group: �жϷ���
 *   @arg       0, ��0: 0λ��ռ���ȼ�, 4λ�����ȼ�
 *   @arg       1, ��1: 1λ��ռ���ȼ�, 3λ�����ȼ�
 *   @arg       2, ��2: 2λ��ռ���ȼ�, 2λ�����ȼ�
 *   @arg       3, ��3: 3λ��ռ���ȼ�, 1λ�����ȼ�
 *   @arg       4, ��4: 4λ��ռ���ȼ�, 0λ�����ȼ�
 * @note        ע�����ȼ����ܳ����趨����ķ�Χ! ����������벻���Ĵ���
 * @retval      ��
 */
void sys_nvic_init(uint8_t pprio, uint8_t sprio, uint8_t ch, uint8_t group)
{
    uint32_t temp;
    sys_nvic_priority_group_config(group);  /* ���÷��� */
    temp = pprio << (4 - group);
    temp |= sprio & (0x0f >> group);
    temp &= 0xf;                            /* ȡ����λ */
    NVIC->ISER[ch / 32] |= 1 << (ch % 32);  /* ʹ���ж�λ(Ҫ����Ļ�,����ICER��ӦλΪ1����) */
    NVIC->IP[ch] |= temp << 4;              /* ������Ӧ���ȼ����������ȼ� */
}

/**
 * @brief       �ⲿ�ж����ú���, ֻ���GPIOA~GPIOG
 * @note        �ú������Զ�������Ӧ�ж�, �Լ�������
 * @param       p_gpiox: GPIOA~GPIOG, GPIOָ��
 * @param       pinx: 0X0000~0XFFFF, ����λ��, ÿ��λ����һ��IO, ��0λ����Px0, ��1λ����Px1, ��������. ����0X0101, ����ͬʱ����Px0��Px8.
 *   @arg       SYS_GPIO_PIN0~SYS_GPIO_PIN15, 1<<0 ~ 1<<15
 * @param       tmode: 1~3, ����ģʽ
 *   @arg       SYS_GPIO_FTIR, 1, �½��ش���
 *   @arg       SYS_GPIO_RTIR, 2, �����ش���
 *   @arg       SYS_GPIO_BTIR, 3, �����ƽ����
 * @retval      ��
 */
void sys_nvic_ex_config(GPIO_TypeDef *p_gpiox, uint16_t pinx, uint8_t tmode)
{
    uint8_t offset;
    uint32_t gpio_num = 0;      /* gpio���, 0~10, ����GPIOA~GPIOG */
    uint32_t pinpos = 0, pos = 0, curpin = 0;

    gpio_num = ((uint32_t)p_gpiox - (uint32_t)GPIOA) / 0X400 ;/* �õ�gpio��� */
    RCC->APB2ENR |= 1 << 0;     /* AFIO = 1,ʹ��AFIOʱ�� */

    for (pinpos = 0; pinpos < 16; pinpos++)
    {
        pos = 1 << pinpos;      /* һ����λ��� */
        curpin = pinx & pos;    /* ��������Ƿ�Ҫ���� */

        if (curpin == pos)      /* ��Ҫ���� */
        {
            offset = (pinpos % 4) * 4;
            AFIO->EXTICR[pinpos / 4] &= ~(0x000F << offset);    /* ���ԭ�����ã����� */
            AFIO->EXTICR[pinpos / 4] |= gpio_num << offset;     /* EXTI.BITxӳ�䵽gpiox.bitx */

            EXTI->IMR |= 1 << pinpos;   /* ����line BITx�ϵ��ж�(���Ҫ��ֹ�жϣ��򷴲�������) */

            if (tmode & 0x01) EXTI->FTSR |= 1 << pinpos;        /* line bitx���¼��½��ش��� */

            if (tmode & 0x02) EXTI->RTSR |= 1 << pinpos;        /* line bitx���¼��������ش��� */
        }
    }
}

/**
 * @brief       GPIO��ӳ�书��ѡ������
 *   @note      �����֧�ֶ�MAPR�Ĵ���������, ��֧�ֶ�MAPR2�Ĵ���������!!!
 * @param       pos: ��AFIO_MAPR�Ĵ����������ʼλ��, 0~24
 *   @arg       [0]    , SPI1_REMAP;         [1]    , I2C1_REMAP;         [2]    , USART1_REMAP;        [3]    , USART2_REMAP;
 *   @arg       [5:4]  , USART3_REMAP;       [7:6]  , TIM1_REMAP;         [9:8]  , TIM2_REMAP;          [11:10], TIM3_REMAP;
 *   @arg       [12]   , TIM4_REMAP;         [14:13], CAN_REMAP;          [15]   , PD01_REMAP;          [16]   , TIM15CH4_REMAP;
 *   @arg       [17]   , ADC1_ETRGINJ_REMAP; [18]   , ADC1_ETRGREG_REMAP; [19]   , ADC2_ETRGINJ_REMAP;  [20]   , ADC2_ETRGREG_REMAP;
 *   @arg       [26:24], SWJ_CFG;
 * @param       bit: ռ�ö���λ, 1 ~ 3, ���pos����˵��
 * @param       val: Ҫ���õĸ��ù���, 0 ~ 4, �ø���posλ������, ��ϸ������ֵ, �μ�: <<STM32���Ĳο��ֲ� V10>> 8.4.2��, ��MAPR�Ĵ�����˵��
 *              ��: sys_gpio_remap_set(24, 3, 2); ��������SWJ_CFG[2:0]    = 2, ѡ��ر�JTAG, ����SWD.
 *                  sys_gpio_remap_set(10, 2, 2); ��������TIM3_REMAP[1:0] = 2, TIM3ѡ�񲿷���ӳ��, CH1->PB4, CH2->PB5, CH3->PB0, CH4->PB1
 * @retval      ��
 */
void sys_gpio_remap_set(uint8_t pos, uint8_t bit, uint8_t val)
{
    uint32_t temp = 0;
    uint8_t i = 0;
    RCC->APB2ENR |= 1 << 0;     /* ��������ʱ�� */

    for (i = 0; i < bit; i++)   /* ���bit��1 */
    {
        temp <<= 1;
        temp += 1;
    }

    AFIO->MAPR &= ~(temp << pos);       /* ���MAPR��Ӧλ��ԭ�������� */
    AFIO->MAPR |= (uint32_t)val << pos; /* ����MAPR��Ӧλ�õ�ֵ */
}

/**
 * @brief       GPIOͨ������
 * @param       p_gpiox: GPIOA~GPIOG, GPIOָ��
 * @param       pinx: 0X0000~0XFFFF, ����λ��, ÿ��λ����һ��IO, ��0λ����Px0, ��1λ����Px1, ��������. ����0X0101, ����ͬʱ����Px0��Px8.
 *   @arg       SYS_GPIO_PIN0~SYS_GPIO_PIN15, 1<<0 ~ 1<<15
 *
 * @param       mode: 0~3; ģʽѡ��, ��������:
 *   @arg       SYS_GPIO_MODE_IN,  0, ����ģʽ(ϵͳ��λĬ��״̬)
 *   @arg       SYS_GPIO_MODE_OUT, 1, ���ģʽ
 *   @arg       SYS_GPIO_MODE_AF,  2, ���ù���ģʽ
 *   @arg       SYS_GPIO_MODE_AIN, 3, ģ������ģʽ
 *
 * @param       otype: 0 / 1; �������ѡ��, ��������:
 *   @arg       SYS_GPIO_OTYPE_PP, 0, �������
 *   @arg       SYS_GPIO_OTYPE_OD, 1, ��©���
 *
 * @param       ospeed: 0~2; ����ٶ�, ��������(ע��: ����Ϊ0!!):
 *   @arg       SYS_GPIO_SPEED_LOW,  2, ����
 *   @arg       SYS_GPIO_SPEED_MID,  1, ����
 *   @arg       SYS_GPIO_SPEED_HIGH, 3, ����
 *
 * @param       pupd: 0~3: ����������, ��������:
 *   @arg       SYS_GPIO_PUPD_NONE, 0, ����������
 *   @arg       SYS_GPIO_PUPD_PU,   1, ����
 *   @arg       SYS_GPIO_PUPD_PD,   2, ����
 *   @arg       SYS_GPIO_PUPD_RES,  3, ����
 *
 * @note:       ע��:
 *              1, ������ģʽ(��ͨ����/ģ������)��, otype �� ospeed ������Ч!!
 *              2, �����ģʽ��, pupd ������Ч!!(��©����޷�ʹ���ڲ���������!!)
 * @retval      ��
 */
void sys_gpio_set(GPIO_TypeDef *p_gpiox, uint16_t pinx, uint32_t mode, uint32_t otype, uint32_t ospeed, uint32_t pupd)
{
    uint32_t pinpos = 0, pos = 0, curpin = 0;
    uint32_t config = 0;        /* ���ڱ���ĳһ��IO������(CNF[1:0] + MODE[1:0]),ֻ���������4λ */

    for (pinpos = 0; pinpos < 16; pinpos++)
    {
        pos = 1 << pinpos;      /* һ����λ��� */
        curpin = pinx & pos;    /* ��������Ƿ�Ҫ���� */

        if (curpin == pos)      /* ��Ҫ���� */
        {
            config = 0;         /* bit0~3������Ϊ0, ��CNF[1:0] = 0; MODE[1:0] = 0;  Ĭ����ģ������ģʽ */

            if ((mode == 0X01) || (mode == 0X02))   /* �������ͨ���ģʽ/���ù���ģʽ */
            {
                config = ospeed & 0X03;             /* ����bit0/1 MODE[1:0] = 2/1/3 �ٶȲ��� */
                config |= (otype & 0X01) << 2;      /* ����bit2   CNF[0]    = 0/1   ����/��©��� */
                config |= (mode - 1) << 3;          /* ����bit3   CNF[1]    = 0/1   ��ͨ/������� */
            }
            else if (mode == 0)     /* �������ͨ����ģʽ */
            {
                if (pupd == 0)   /* ����������,����������ģʽ */
                {
                    config = 1 << 2;               /* ����bit2/3 CNF[1:0] = 01   ��������ģʽ */
                }
                else
                {
                    config = 1 << 3;                            /* ����bit2/3 CNF[1:0] = 10   ����������ģʽ */
                    p_gpiox->ODR &= ~(1 << pinpos);             /* ���ԭ�������� */
                    p_gpiox->ODR |= (pupd & 0X01) << pinpos;    /* ����ODR = 0/1 ����/���� */
                }
            }

            /* ����IO��λ�� ����CRL / CRH�Ĵ��� */
            if (pinpos <= 7)
            {
                p_gpiox->CRL &= ~(0X0F << (pinpos * 4));        /* ���ԭ�������� */
                p_gpiox->CRL |= config << (pinpos * 4);         /* ����CNFx[1:0] �� MODEx[1:0], x = pinpos = 0~7 */
            }
            else
            {
                p_gpiox->CRH &= ~(0X0F << ((pinpos - 8) * 4));  /* ���ԭ�������� */
                p_gpiox->CRH |= config << ((pinpos - 8) * 4);   /* ����CNFx[1:0] �� MODEx[1:0], x = pinpos = 8~15 */

            }
        }
    }
}

/**
 * @brief       ����GPIOĳ�����ŵ����״̬
 * @param       p_gpiox: GPIOA~GPIOG, GPIOָ��
 * @param       0X0000~0XFFFF, ����λ��, ÿ��λ����һ��IO, ��0λ����Px0, ��1λ����Px1, ��������. ����0X0101, ����ͬʱ����Px0��Px8.
 *   @arg       SYS_GPIO_PIN0~SYS_GPIO_PIN15, 1<<0 ~ 1<<15
 * @param       status: 0/1, ����״̬(�����λ��Ч), ��������:
 *   @arg       0, ����͵�ƽ
 *   @arg       1, ����ߵ�ƽ
 * @retval      ��
 */
void sys_gpio_pin_set(GPIO_TypeDef *p_gpiox, uint16_t pinx, uint8_t status)
{
    if (status & 0X01)
    {
        p_gpiox->BSRR |= pinx;  /* ����GPIOx��pinxΪ1 */
    }
    else
    {
        p_gpiox->BSRR |= (uint32_t)pinx << 16;  /* ����GPIOx��pinxΪ0 */
    }
}

/**
 * @brief       ��ȡGPIOĳ�����ŵ�״̬
 * @param       p_gpiox: GPIOA~GPIOG, GPIOָ��
 * @param       0X0000~0XFFFF, ����λ��, ÿ��λ����һ��IO, ��0λ����Px0, ��1λ����Px1, ��������. ����0X0101, ����ͬʱ����Px0��Px8.
 *   @arg       SYS_GPIO_PIN0~SYS_GPIO_PIN15, 1<<0 ~ 1<<15
 * @retval      ��������״̬, 0, �͵�ƽ; 1, �ߵ�ƽ
 */
uint8_t sys_gpio_pin_get(GPIO_TypeDef *p_gpiox, uint16_t pinx)
{
    if (p_gpiox->IDR & pinx)
    {
        return 1;   /* pinx��״̬Ϊ1 */
    }
    else
    {
        return 0;   /* pinx��״̬Ϊ0 */
    }
}

/**
 * @brief       ִ��: WFIָ��(ִ�����ָ�����͹���״̬, �ȴ��жϻ���)
 * @param       ��
 * @retval      ��
 */
void sys_wfi_set(void)
{
    __ASM volatile("wfi");
}

/**
 * @brief       �ر������ж�(���ǲ�����fault��NMI�ж�)
 * @param       ��
 * @retval      ��
 */
void sys_intx_disable(void)
{
    __ASM volatile("cpsid i");
}

/**
 * @brief       ���������ж�
 * @param       ��
 * @retval      ��
 */
void sys_intx_enable(void)
{
    __ASM volatile("cpsie i");
}

/**
 * @brief       ����ջ����ַ
 * @note        ���ĺ�X, ����MDK��, ʵ����û�����
 * @param       addr: ջ����ַ
 * @retval      ��
 */
void sys_msr_msp(uint32_t addr)
{
    __set_MSP(addr);    /* ����ջ����ַ */
}

/**
 * @brief       �������ģʽ
 * @param       ��
 * @retval      ��
 */
void sys_standby(void)
{
    RCC->APB1ENR |= 1 << 28;    /* ʹ�ܵ�Դʱ�� */
    PWR->CSR |= 1 << 8;         /* ����WKUP���ڻ��� */
    PWR->CR |= 1 << 2;          /* ���WKUP ��־ */
    PWR->CR |= 1 << 1;          /* PDDS = 1, ����������˯��ģʽ(PDDS) */
    SCB->SCR |= 1 << 2;         /* ʹ��SLEEPDEEPλ (SYS->CTRL) */
    sys_wfi_set();              /* ִ��WFIָ��, �������ģʽ */
}

/**
 * @brief       ϵͳ��λ
 * @param       ��
 * @retval      ��
 */
void sys_soft_reset(void)
{
    SCB->AIRCR = 0X05FA0000 | (uint32_t)0x04;
}

/**
 * @brief       ʱ�����ú���
 * @param       plln: PLL��Ƶϵ��(PLL��Ƶ), ȡֵ��Χ: 2~16
 * @note
 *
 *              PLLCLK: PLL���ʱ��
 *              PLLSRC: PLL����ʱ��Ƶ��, ������ HSI/2, HSE/2, HSE��, һ��ѡ��HSE.
 *              SYSCLK: ϵͳʱ��, ��ѡ���� HSI/PLLCLK/HSE, һ��ѡ������PLLCLK
 *              FCLK  : Cortex M3�ں�ʱ��, ����HCLK
 *              HCLK  : AHB����ʱ��, ���� SYSCLK �ķ�Ƶ, ������1...512��Ƶ, һ�㲻��Ƶ
 *              PCLK2 : APB2����ʱ��, ���� HCLK �ķ�Ƶ(���72Mhz), ������1/2/4/8/16��Ƶ, һ�㲻��Ƶ
 *              PCLK1 : APB1����ʱ��, ���� HCLK �ķ�Ƶ(���36Mhz), ������1/2/4/8/16��Ƶ, һ�����Ƶ
 *
 *              PLLCLK = PLLSRC * plln;
 *              FCLK = HCLK = SYSCLK;
 *              PCLK2 = HCLK;
 *              PCLK1 = HCLK / 2;
 *
 *              ����һ��ѡ��PLLSRC����HSE, �������ⲿ����.
 *              ���ⲿ����Ϊ 8M��ʱ��, �Ƽ�: plln = 9, AHB����Ƶ, �õ�:
 *              PLLCLK = 8 * 9 = 72Mhz
 *              FCLK = HCLK = SYSCLK = PLLCLK / 1 = 72Mhz
 *              PCLK2 = HCLK = 72Mhz
 *              PCLK1 = HCLK / 2 = 36Mhz
 *
 *              ����STM32F103��PLL˵�����: <<STM32���Ĳο��ֲ� V10>>�������������
 *
 * @retval      �������: 0, �ɹ�; 1, HSE����;
 */
uint8_t sys_clock_set(uint32_t plln)
{
    uint32_t retry = 0;
    uint8_t retval = 0;
    RCC->CR |= 0x00010000;          /* �ⲿ����ʱ��ʹ��HSEON */

    while (retry < 0XFFF0)
    {
        __nop();

        /* ע��, MDK5.29���Ժ�汾, ��ʹ��HSEON�Ժ�, �������һ������ʱ
         * �ٿ�ʼ��������, �ᵼ�·��������������, ��ʱ�������в������� bug
         * ��Ҫ����λ����, ��ʱ�Ż�����, �������һ������ʱ, ���Խ����� bug
         * ����, �������õ���ʱʱ��, ������ 0X8000 �� nopʱ��
         */
        if (RCC->CR & (1 << 17) && retry > 0X8000)
        {
            break;
        }

        retry++;        /* �ȴ�HSE RDY */
    }

    if (retry >= 0XFFF0)
    {
        retval = 1;     /* HSE�޷����� */
    }
    else
    {
        RCC->CFGR = 0X00000400;     /* PCLK1 = HCLK / 2; PCLK2 = HCLK; HCLK = SYSCLK; */
        plln -= 2;                  /* ����2����λ(��Ϊ�Ǵ�2��ʼ��, ����0����2) */
        RCC->CFGR |= plln << 18;    /* ����PLLֵ 2~16 */
        RCC->CFGR |= 1 << 16;       /* PLLSRC = 1, ѡ�� HSE ��Ϊ PLL ����ʱ�� */

        /* FLASH_ACR�Ĵ������������: <<STM32F10xx�������ֲ�>> */
        FLASH->ACR = 1 << 4;        /* PRFTBE = 1 ����Ԥȡ������ */
        FLASH->ACR |= 2 << 0;       /* LATENCY[2:0] = 2 FLASH�����ȴ����� */

        RCC->CR |= 1 << 24;         /* PLLON = 1, ʹ��PLL */

        while (!(RCC->CR >> 25));   /* �ȴ�PLL���� */

        RCC->CFGR |= 2 << 0;        /* SW[1:0] = 2, ѡ��PLL�����Ϊϵͳʱ�� */

        while (((RCC->CFGR >> 2) & 0X03) != 2); /* �ȴ�PLL��Ϊϵͳʱ�����óɹ� */
    }

    return retval;
}

/**
 * @brief       ϵͳʱ�ӳ�ʼ������
 * @param       plln: PLL��Ƶϵ��(PLL��Ƶ), ȡֵ��Χ: 2~16
 * @retval      ��
 */
void sys_stm32_clock_init(uint32_t plln)
{
    RCC->APB1RSTR = 0x00000000;     /* ��λ���� */
    RCC->APB2RSTR = 0x00000000;
    
    RCC->AHBENR = 0x00000014;       /* ˯��ģʽ�����SRAMʱ��ʹ��.�����ر� */
    RCC->APB2ENR = 0x00000000;      /* ����ʱ�ӹر� */
    RCC->APB1ENR = 0x00000000;
    
    RCC->CR |= 0x00000001;          /* ʹ���ڲ�����ʱ��HSION */
    RCC->CFGR &= 0xF8FF0000;        /* ��λSW[1:0], SWS[1:0], HPRE[3:0], PPRE1[2:0], PPRE2[2:0], ADCPRE[1:0], MCO[2:0] */
    RCC->CR &= 0xFEF6FFFF;          /* ��λHSEON, CSSON, PLLON */
    RCC->CR &= 0xFFFBFFFF;          /* ��λHSEBYP */
    RCC->CFGR &= 0xFF80FFFF;        /* ��λPLLSRC, PLLXTPRE, PLLMUL[3:0] �� USBPRE/OTGFSPRE */
    RCC->CIR = 0x009F0000;          /* �ر�����RCC�жϲ�����жϱ�־ */

    sys_clock_set(plln);            /* ����ʱ�� */

    /* �����ж�����ƫ�� */
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
* @ Brief     -> ϵͳ������صĺ���
**********************************************************/


/**********************************************************
* �������� ---> ��ȡоƬ��������
* ��ڲ��� ---> *STMCapa�������ַ���ʾ����
* ������ֵ ---> ������ʮ���ƣ�
* ����˵�� ---> none
**********************************************************/
void STM32_Flash_Capacity(uint8_t *STMCapa)
{
	uint16_t capa;
	
	capa = *((uint16_t*)0x1ffff7e0);	//��ȡ���������Ĵ�������16λ��Ч
	
	capa = ((capa >> 12) * 4096 + ((capa >> 8) & 0x0f) * 256 + ((capa >> 4) & 0x0f) * 16 + (capa & 0x0f));	//ת����ʮ����
	
	STMCapa[0] = 0x4d;	//M
	STMCapa[1] = 0x43;	//C
	STMCapa[2] = 0x55;	//U
	STMCapa[3] = 0x43;	//C
	STMCapa[4] = 0x61;	//a
	STMCapa[5] = 0x70;	//p
	STMCapa[6] = 0x3a;	//:
	
	if((capa / 1000) != 0)	STMCapa[7] = capa / 1000 + 48;	//ǧλ��Ϊ0ʱ��ʾ
	else	STMCapa[7] = 0x20;
	
	STMCapa[8] = capa % 1000 / 100 + 48;	//��λ
	STMCapa[9] = capa % 100 /10 + 48;		//ʮλ
	STMCapa[10] = capa % 10 + 48;			//��λ
	STMCapa[11] = 0x4b;	//K
	STMCapa[12] = 0x62;	//b
}
/**********************************************************
* �������� ---> ��ȡCPUID
* ��ڲ��� ---> none
* ������ֵ ---> CPUID��ʮ�����ƣ�
* ����˵�� ---> none
**********************************************************/
void STM32_CPUID(uint8_t *IDbuff)
{
	uint32_t CPUID;
	CPUID = *((uint32_t*)0xe000ed00);
	sprintf((char*)IDbuff, "CPU ID:%08X", CPUID);
}
/**********************************************************
* �������� ---> ����������ƫ�Ƶ�ַ
* ��ڲ��� ---> NVIC_VectTab����ַ
*              Offset��ƫ����	
* ������ֵ ---> ������ʮ���ƣ�
* ����˵�� ---> none
**********************************************************/	 
void MY_NVIC_SetVectorTable(uint32_t NVIC_VectTab,uint32_t Offset)	 
{ 	   	 
	SCB->VTOR = NVIC_VectTab | (Offset & (uint32_t)0x1fffff80);//����NVIC��������ƫ�ƼĴ���
	//���ڱ�ʶ����������CODE��������RAM��
}
/**********************************************************
* �������� ---> �����жϷ���
* ��ڲ��� ---> NVIC_PriorityGroup: �жϷ���
* ������ֵ ---> none
* ����˵�� ---> 0 ~ 4�飬������5��
**********************************************************/
void MY_NVIC_PriorityGroup_Config(uint32_t NVIC_PriorityGroup)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup);	//�����жϷ���
	
//	uint32_t temp,temp1;
//	
//	temp1 = (~NVIC_PriorityGroup) & 0x00000007;//ȡ����λ
//	temp1 <<= 8;
//	temp = SCB->AIRCR;  //��ȡ��ǰ������
//	temp &= 0x0000f8ff; //�����ǰ����
//	temp |= 0x05fa0000; //д��Կ��
//	temp |= temp1;	   
//	SCB->AIRCR = temp;  //���÷���
}
/**********************************************************
* �������� ---> �����жϷ������ȼ�
* ��ڲ��� ---> NVIC_PreemptionPriority���������ȼ�
*               NVIC_Subpriority����Ӧ���ȼ�
*               NVIC_Channel���жϱ��
*               NVIC_Group: �жϷ���
* ������ֵ ---> none
* ����˵�� ---> 1���黮��:
*                  ��0��0λ��ռ���ȼ���4λ��Ӧ���ȼ�
*                  ��1��1λ��ռ���ȼ���3λ��Ӧ���ȼ�
*                  ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
*                  ��3��3λ��ռ���ȼ���1λ��Ӧ���ȼ�
*                  ��4��4λ��ռ���ȼ���0λ��Ӧ���ȼ�
*               2���������ȼ���������Ӧ���ȼ�����ԭ��������ֵԽС���ȼ�Խ��
**********************************************************/
void MY_NVIC_Init(uint8_t NVIC_PreemptionPriority,uint8_t NVIC_Subpriority,uint8_t NVIC_Channel,uint32_t NVIC_Group)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	MY_NVIC_PriorityGroup_Config(NVIC_Group);	//�����жϷ���	
	
	NVIC_InitStructure.NVIC_IRQChannel = NVIC_Channel;								//�����ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_PreemptionPriority;	//�������ȼ�  	 	 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = NVIC_Subpriority;				//��Ӧ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;									//ʹ���ж�

	NVIC_Init(&NVIC_InitStructure);	//��ʼ���ж�
	
//	uint32_t temp;	
//	uint8_t IPRADDR=NVIC_Channel/4;  //ÿ��ֻ�ܴ�4��,�õ����ַ 
//	uint8_t IPROFFSET=NVIC_Channel%4;//�����ڵ�ƫ��
//	
//	IPROFFSET = IPROFFSET*8 + 4;    //�õ�ƫ�Ƶ�ȷ��λ��
//	
//	MY_NVIC_PriorityGroup_Config(NVIC_Group);//���÷���
//	
//	temp  = NVIC_PreemptionPriority << (4 - NVIC_Group);	//�������ȼ�	  
//	temp |= NVIC_Subpriority & (0x0f >> NVIC_Group);	//��Ӧ���ȼ�
//	temp &= 0xf;//ȡ����λ

//	if(NVIC_Channel < 32)	NVIC->ISER[0] |= 1 << NVIC_Channel;//ʹ���ж�λ(Ҫ����Ļ�,�෴������OK)
//	else	NVIC->ISER[1]| |= 1 << (NVIC_Channel - 32); 
//	
//	NVIC->IPR[IPRADDR] |= temp << IPROFFSET;//������Ӧ���ȼ����������ȼ�  
}
/**********************************************************
* �������� ---> THUMBָ�֧�ֻ������
* ��ڲ��� ---> none
* ������ֵ ---> none
* ����˵�� ---> �������·���ʵ��ִ�л��ָ��WFI
**********************************************************/
__asm void WFI_SET(void)
{
	WFI;    
}
/**********************************************************
* �������� ---> ����ʱ�ӼĴ�����λ
* ��ڲ��� ---> none
* ������ֵ ---> none
* ����˵�� ---> ����������ִ���������踴λ!�����������𴮿ڲ�����
**********************************************************/
void MY_RCC_DeInit(void)
{										  					   
	RCC->APB1RSTR = 0x00000000;	//��λ����			 
	RCC->APB2RSTR = 0x00000000; 
	  
  	RCC->AHBENR   = 0x00000014;	//˯��ģʽ�����SRAMʱ��ʹ��.�����ر�.	  
  	RCC->APB2ENR  = 0x00000000;	//����ʱ�ӹر�.			   
  	RCC->APB1ENR  = 0x00000000;   
	RCC->CR      |= 0x00000001;	//ʹ���ڲ�����ʱ��HSION	 															 
	RCC->CFGR    &= 0xf8ff0000;	//��λSW[1:0],HPRE[3:0],PPRE1[2:0],PPRE2[2:0],ADCPRE[1:0],MCO[2:0]					 
	RCC->CR      &= 0xfef6ffff;	//��λHSEON,CSSON,PLLON
	RCC->CR      &= 0xfffbffff;	//��λHSEBYP	   	  
	RCC->CFGR    &= 0xff80ffff;	//��λPLLSRC, PLLXTPRE, PLLMUL[3:0] and USBPRE 
	RCC->CIR      = 0x00000000;	//�ر������ж�
	
	//����������				  
#ifdef  VECT_TAB_RAM
	MY_NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else   
	MY_NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0);
#endif
}
/**********************************************************
* �������� ---> ����оƬ�������ģʽ
* ��ڲ��� ---> none
* ������ֵ ---> none
* ����˵�� ---> ����WKUP���Ż��ѣ�PA.0��
**********************************************************/
void SYS_Standby(void)
{
	SCB->SCR |= 1 << 2;			//ʹ��sleep deep��SYS->CTRL��
	RCC->APB1ENR |= 1 << 28;	//��Դ�ӿ�ʱ�ӿ���
	PWR->CSR |= 1 << 8;			//����WKUP�������ڻ���
	PWR->CR |= 1 << 2;			//���WAKE-UP��־
	PWR->CR |= 1 << 1;			//PDDS��λ��������˯��
	WFI_SET();					//ִ��WFIָ��
}
/**********************************************************
* �������� ---> ϵͳ��λ
* ��ڲ��� ---> none
* ������ֵ ---> none
* ����˵�� ---> none
**********************************************************/
void SYS_SoftReset(void)
{
	SCB->AIRCR = 0x05fa0000 | 0x00000004;
}
/**********************************************************
* �������� ---> JTAGģʽ����
* ��ڲ��� ---> mode��ģʽ����
*                    000��JTAG-DP + SW-DP����λ״̬��
*                    001��JTAG-DP + SW-DP������JNTRST���ţ��ͷ�JRST���ţ�
*                    010��JTAG-DP�ӿڽ�ֹ��SW-DP�ӿ�����
*                    100��JTAG-DP�ӿں�SW-DP�ӿڶ���ֹ
*                    xxx������ֵ����ֹ
* ������ֵ ---> none
* ����˵�� ---> none
**********************************************************/
void STM_JTAG_Set(uint32_t mode)
{
	RCC->APB2ENR |= 1 << 0;		//ʹ�ܸ���ʱ��
	AFIO->MAPR &= 0xf8ffffff;	//���SWJ_CFG[2:0]λ������26:24��
	AFIO->MAPR |= (mode << 24);	//����JTAGģʽ
}
/**********************************************************
* �������� ---> ϵͳʱ�ӳ�ʼ��
* ��ڲ��� ---> pll����Ƶ����ȡֵ��Χ��2 ~ 16
* ������ֵ ---> none
* ����˵�� ---> none
**********************************************************/
void STM_Clock_Init(uint8_t pll)
{
	uint8_t tmp = 0;
	
	MY_RCC_DeInit();	//��λ���������������ҽ��ⲿ�жϺ�����ʱ��ȫ���ر�
	
	RCC->CR |= 0x00010000;  //�ⲿ����ʱ��ʹ��HSEON
	
	while(!(RCC->CR >> 17));//�ȴ��ⲿʱ�Ӿ���
	
	RCC->CFGR = 0X00000400; //APB1=DIV2;APB2=DIV1;AHB=DIV1;
	pll -= 2;//����2����λ
	RCC->CFGR |= pll << 18;   //����PLLֵ 2~16
	RCC->CFGR |= 1 << 16;	  //PLLSRC ON 
	FLASH->ACR |= 0x32;	  //FLASH 2����ʱ����

	RCC->CR |= 0x01000000;  //PLLON
	
	while(!(RCC->CR >> 25));//�ȴ�PLL����
	
	RCC->CFGR |= 0x00000002;//PLL��Ϊϵͳʱ�ӣ������ܿ���PLL����Ϊ�趨PLLʱ��PLL���λ�����ڹر�״̬�½���
	
	while(tmp != 0x02)     //�ȴ�PLL��Ϊϵͳʱ�����óɹ�
	{   
		tmp  = RCC->CFGR >> 2;
		tmp &= 0x03;
	}    
}
/**********************************************************
* �������� ---> BCD��תΪHEX
* ��ڲ��� ---> BCD_Data��Ҫת����BCD����
* ������ֵ ---> HEX��
* ����˵�� ---> none
**********************************************************/	
uint8_t BCD_to_HEX(uint8_t BCD_Data)
{
	return((BCD_Data / 10) << 4 | (BCD_Data % 10));
}
/**********************************************************
* �������� ---> HEX��תΪBCD
* ��ڲ��� ---> HEX_Data��Ҫת����BCD����
* ������ֵ ---> BCD��
* ����˵�� ---> none
**********************************************************/	
uint8_t HEX_to_BCD(uint8_t HEX_Data)
{
	return((HEX_Data >> 4) * 10 + (HEX_Data & 0x0f));
}
/**********************************************************
* �������� ---> 10������תΪ16����
* ��ڲ��� ---> DX_Data��Ҫת����10��������
* ������ֵ ---> 16����
* ����˵�� ---> none
**********************************************************/
uint16_t DX_to_HX(uint16_t DX_Data)
{
	return(((DX_Data/1000)<<12) | ((DX_Data%1000/100)<<8) | ((DX_Data%100/10)<<4) | (DX_Data%10));
}
/**********************************************************
* �������� ---> 16������תΪ10����
* ��ڲ��� ---> HX_Data��Ҫת����16��������
* ������ֵ ---> 10����
* ����˵�� ---> none
**********************************************************/
uint16_t HX_to_DX(uint16_t HX_Data)
{
	return((HX_Data>>12)*1000+((HX_Data&0x0f00)>>8)*100+((HX_Data&0x00f0)>>4)*10+(HX_Data&0x000f));
}	

///**********************************************************
//* �������� ---> ��ʼ�������б�
//* ��ڲ��� ---> *LIST���б�ָ��
//* ������ֵ ---> none
//* ����˵�� ---> none
//**********************************************************/
//void Sqlist_Init(Sqlist *LIST)
//{
//	LIST->elem = (uint16_t*)malloc(MaxSize * sizeof(ElemType));
//	//����һ������ΪMaxSize * sizeof(ElemType)��С���ڴ�ռ�
//	if(!LIST->elem)	return;	//û�����������б�ֱ���˳�
//	//����ɹ�
//	LIST->length = 0;	//�б���û����
//	LIST->listsize = MaxSize;	//�����ݱ�ռ���ڴ��СΪMaxSize����sizeof(ElemType)Ϊ��λ��
//}
///**********************************************************
//* �������� ---> ��λ�����б�
//* ��ڲ��� ---> none
//* ������ֵ ---> none
//* ����˵�� ---> none
//**********************************************************/
//void Sqlist_DeInit(void)
//{
//	Sqlist *list;
//
//	list->elem = 0;	//�׵�ַ����
//	list->length = 0;	//��������
//	list->listsize = 0;	//�б��СΪ0
//}
///**********************************************************
//* �������� ---> ��һ����̬�������б����һ��Ԫ��
//* ��ڲ��� ---> *L���б�ָ��
//*               i���б��е�i��λ�ò���Ԫ��
//*               item���ڵ�i��λ���������Ԫ��
//* ������ֵ ---> none
//* ����˵�� ---> none
//**********************************************************/
//void InsertElem(Sqlist *L,uint16_t i,ElemType item)
//{	/* ��˳���б�*L�ĵ�i��λ�ò���Ԫ��item */
//	ElemType *base, *insertPtr, *p;
//
//	if(i < 1 || i > L->length + 1)	return;	//�Ƿ�����
//	if(L->length >= L->listsize)	//�������б����һ��λ�ò���Ԫ��
//	{	//׷���ڴ�ռ�
//		base = (ElemType*)realloc(L->elem,(L->listsize + 10) * sizeof(ElemType));
//		L->elem = base;	//�����ڴ����ַ
//		L->listsize = L->listsize + 100;	//�洢�ռ�����100����Ԫ
//	}
//	insertPtr = &(L->Elem[i - 1]);	//insertPtrΪ����λ��
//	for(p = &(L->elem[L->length - 1]);p >= insertPtr;p--)
//		*(p + 1) = *p;	//��i - 1�Ժ��Ԫ��˳�������һ��Ԫ��λ��
//	*insertPtr = item;	//�ڵ�i��λ���ϲ���Ԫ��item
//	L->length++;	//����1
//}








