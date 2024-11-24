/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
/*  This program is protected by German copyright law and international      */
/*  treaties. The use of this software including but not limited to its      */
/*  Source Code is subject to restrictions as agreed in the license          */
/*  agreement between you and Siemens.                                       */
/*  Copying or distribution is not allowed unless expressly permitted        */
/*  according to your license agreement with Siemens.                        */
/*****************************************************************************/
/*                                                                           */
/*  P r o j e c t         &P: PROFINET IO Runtime Software              :P&  */
/*                                                                           */
/*  P a c k a g e         &W: PROFINET IO Runtime Software              :W&  */
/*                                                                           */
/*  C o m p o n e n t     &C: PnIODDevkits                              :C&  */
/*                                                                           */
/*  F i l e               &F: arm926.h                                  :F&  */
/*                                                                           */
/*  V e r s i o n         &V: PnIODDevkits_P05.01.00.00_00.01.00.20     :V&  */
/*                                                                           */
/*  D a t e  (YYYY-MM-DD) &D: 2023-05-24                                :D&  */
/*                                                                           */
/*****************************************************************************/
/*                                                                           */
/*  D e s c r i p t i o n :                                                  */
/*                                                                           */
/*                                                                           */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* reinclude protection */
#ifndef __ARM926_INC__
#define __ARM926_INC__


/*****************************************************************************/
/* internal defines */

// ------------------------------------------
// Makros for access to peripheral registers
// ------------------------------------------
enum asm_access_t {SINGLE=1, INCR4=4, INCR8=8};

#define REG8(x)                     (*(unsigned char  volatile *)(x))
#define REG16(x)                    (*(unsigned short volatile *)(x))
#define REG32(x)                    (*(unsigned long  volatile *)(x))
#define REG64(x)                    (*(unsigned long long  volatile *)(x))

#define READ_ULONG(adr)          (*(volatile unsigned long  *) (adr))
#define WRITE_ULONG(adr, value)  (*(volatile unsigned long  *) (adr) = value)
#define READ_USHORT(adr)         (*(volatile unsigned short *) (adr))
#define WRITE_USHORT(adr,value)  (*(volatile unsigned short *) (adr) = (unsigned short)value)
#define READ_UCHAR(adr)          (*(volatile unsigned char  *) (adr))
#define WRITE_UCHAR(adr,value)   (*(volatile unsigned char  *) (adr) = (unsigned char)value)

#define BIT_0                           0x00000001
#define BIT_1                           0x00000002
#define BIT_2                           0x00000004
#define BIT_3                           0x00000008
#define BIT_4                           0x00000010
#define BIT_5                           0x00000020
#define BIT_6                           0x00000040
#define BIT_7                           0x00000080

#define BIT_8                           0x00000100
#define BIT_9                           0x00000200
#define BIT_10                          0x00000400
#define BIT_11                          0x00000800
#define BIT_12                          0x00001000
#define BIT_13                          0x00002000
#define BIT_14                          0x00004000
#define BIT_15                          0x00008000

#define BIT_16                          0x00010000
#define BIT_17                          0x00020000
#define BIT_18                          0x00040000
#define BIT_19                          0x00080000
#define BIT_20                          0x00100000
#define BIT_21                          0x00200000
#define BIT_22                          0x00400000
#define BIT_23                          0x00800000

#define BIT_24                          0x01000000
#define BIT_25                          0x02000000
#define BIT_26                          0x04000000
#define BIT_27                          0x08000000
#define BIT_28                          0x10000000
#define BIT_29                          0x20000000
#define BIT_30                          0x40000000
#define BIT_31                          0x80000000

#define swap_uint8(_value)                              ( ((_value & 0x0F) << 4) | ((_value & 0xF0) >> 4) )
#define swap_uint16(_value)                             ( ((_value & 0x00FF) << 8) | ((_value & 0xFF00) >> 8) )

#define set_bit__(_var, _bit)                           (_var |= (_bit))
#define clear_bit__(_var, _bit)                         (_var &= ~(_bit))
#define toggle_bit__(_var, _bit)                        (_var ^= (_bit))
#define test_bit_set__(_var, _bit)                      ((_var & (_bit)) == (_bit))
#define test_bit_clear__(_var, _bit)                    ((_var & (_bit)) == 0)

#define set_bit_field__(_var, _mask, _setting, _shift)  \
{                                                       \
    clear_bit__(REG32(_var), _mask);                    \
    set_bit__(REG32(_var), _setting << _shift);         \
}

#define port_direction_out__(_var, _bit)                clear_bit__(_var, _bit)
#define port_direction_in__(_var, _bit)                 set_bit__(_var, _bit)


#define ARM926_IRQ_ENABLE               WRITE_ULONG(U_ICU_ICU96_INST__MASKALL, 0x00000000)
#define ARM926_FIQ_ENABLE               WRITE_ULONG(U_ICU_ICU8_INST__MASKALL,  0x00000000)

#define ARM926_IRQ_DISABLE              WRITE_ULONG(U_ICU_ICU96_INST__MASKALL, 0x00000001)
#define ARM926_FIQ_DISABLE              WRITE_ULONG(U_ICU_ICU8_INST__MASKALL,  0x00000001)

#define ARM926_MASKREG0_IRQ_DISABLE     WRITE_ULONG(U_ICU_ICU96_INST__MASKREG0, 0xFFFFFFFF)

#define PNIP_ICU2_IRQ_DISABLE                           \
{                                                       \
    WRITE_ULONG(U_PNIP__IRQ2MASK_LOW_0, 0xFFFFFFFF);    \
    WRITE_ULONG(U_PNIP__IRQ2MASK_MID_0, 0xFFFFFFFF);    \
    WRITE_ULONG(U_PNIP__IRQ2MASK_HIGH_0, 0xFFFFFFFF);   \
}

//========================================================================
//  KRISC DTCM
//========================================================================
#define KRISC_DTCM_BASE             (0x04000000 + ERTEC200P_TOP_BASE)

//========================================================================
//  PNIP RAM
//========================================================================
#define PNIP_RAM__OFFSET_STATISTIC  0x0000e000
#define PNIP_RAM__OFFSET_SYNC       0x0003c000
#define PNIP_RAM__OFFSET_PACK_CTRL  0x00051000
#define PNIP_RAM__OFFSET_PACK_DATA  0x00058000
#define PNIP_RAM__OFFSET_CMD        0x00068000
#define PNIP_RAM__OFFSET_API_CTRL   0x00080000

// sizes must be divisible by 4!
#define PNIP_RAM__SIZE_STATISTIC    (512)
#define PNIP_RAM__SIZE_SYNC         (768)
#define PNIP_RAM__SIZE_PACK_CTRL    (0)
#define PNIP_RAM__SIZE_PACK_DATA    (0)
#define PNIP_RAM__SIZE_CMD          (1024*8)
#define PNIP_RAM__SIZE_API_CTRL     (1024*16)


/*@settings for GPIO_PORT_MODE_0_L__GPIO_XX_MODE_X_L/H  alternate functions*/
#define GPIO_FUNCTION                       0x0
#define ALTERNATE_FUNCTION_A                0x1
#define ALTERNATE_FUNCTION_B                0x2
#define ALTERNATE_FUNCTION_C                0x3

#define gpio_BASE 0x40018000
#define GPIO_IOCTRL_0                         (0x00 + gpio_BASE)
#define GPIO_OUT_0                            (0x04 + gpio_BASE)
#define GPIO_OUT_SET_0                        (0x08 + gpio_BASE)
#define GPIO_OUT_CLEAR_0                      (0x0C + gpio_BASE)
#define GPIO_RES_DIS_0                        (0x10 + gpio_BASE)
#define GPIO_IN_0                             (0x14 + gpio_BASE)
#define GPIO_PORT_MODE_0_L                    (0x18 + gpio_BASE)


/*****************************************************************************
 *  Timer
 *****************************************************************************/
// f = 125MHz / 4 = 31.25MHz    (T = 1 / f = 32ns)

#define TIMER_RELOAD_VALUE_MAX              0xFFFFFFFF // corresponds 137,4389534s
#define TIMER_RELOAD_VALUE_MIDDLE           0x7FFFFFFF // corresponds 68,7194767s
#define TIMER_RELOAD_VALUE_1s               0x01DCD64F
#define TIMER_RELOAD_VALUE_100ms            0x002FAF07
#define TIMER_RELOAD_VALUE_50ms             0x0017D783
#define TIMER_RELOAD_VALUE_10ms             0x0004C4B3
#define TIMER_RELOAD_VALUE_2ms              0x0000F423
#define TIMER_RELOAD_VALUE_1ms              0x00007A11
#define TIMER_RELOAD_VALUE_650us            0x00004F57
#define TIMER_RELOAD_VALUE_300us            0x0000249E
#define TIMER_RELOAD_VALUE_100us            0x00000C34
#define TIMER_RELOAD_VALUE_1_DIV_256_s      0x0001DCD5


#define TIM_ALL_CLK                                 \
(                                                   \
    U_TIMER__GATE_TRIG_CONTROL_REG__TIM_0_CLK_EN    \
  | U_TIMER__GATE_TRIG_CONTROL_REG__TIM_1_CLK_EN    \
  | U_TIMER__GATE_TRIG_CONTROL_REG__TIM_2_CLK_EN    \
  | U_TIMER__GATE_TRIG_CONTROL_REG__TIM_3_CLK_EN    \
  | U_TIMER__GATE_TRIG_CONTROL_REG__TIM_4_CLK_EN    \
  | U_TIMER__GATE_TRIG_CONTROL_REG__TIM_5_CLK_EN    \
)

/*****************************************************************************
 *  LEDs
 *  LEDs on ET200 usecase board
 *****************************************************************************/
#define ON1_LED     BIT_10
#define MAINT1_LED  BIT_12
#define BF1_LED     BIT_14
#define SF1_LED     BIT_15

#define led_on__(_bit)      clear_bit__(REG32(GPIO_OUT_0), _bit)
#define led_off__(_bit)     set_bit__(REG32(GPIO_OUT_0), _bit)
#define led_toggle__(_bit)  toggle_bit__(REG32(GPIO_OUT_0), _bit)


/*****************************************************************************/
/* reinclude-protection */
#else
    #pragma message ("The header ARM926.H is included twice or more !")
#endif
/*** end of file *************************************************************/

/*****************************************************************************/
/*  Copyright (C) 2023 Siemens Aktiengesellschaft. All rights reserved.      */
/*****************************************************************************/
