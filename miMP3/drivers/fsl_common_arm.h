/*
 * Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FSL_COMMON_ARM_H_
#define _FSL_COMMON_ARM_H_

/*
 * For CMSIS pack RTE.
 * CMSIS pack RTE generates "RTC_Components.h" which contains the statements
 * of the related <RTE_Components_h> element for all selected software components.
 */
#ifdef _RTE_
#include "RTE_Components.h"
#endif

/*!
 * @addtogroup ksdk_common
 * @{
 */

/*! @name Atomic modification
 *
 * These macros are used for atomic access, such as read-modify-write
 * to the peripheral registers.
 *
 * - SDK_ATOMIC_LOCAL_ADD
 * - SDK_ATOMIC_LOCAL_SET
 * - SDK_ATOMIC_LOCAL_CLEAR
 * - SDK_ATOMIC_LOCAL_TOGGLE
 * - SDK_ATOMIC_LOCAL_CLEAR_AND_SET
 *
 * Take SDK_ATOMIC_LOCAL_CLEAR_AND_SET as an example: the parameter @c addr
 * means the address of the peripheral register or variable you want to modify
 * atomically, the parameter @c clearBits is the bits to clear, the parameter
 * @c setBits it the bits to set.
 * For example, to set a 32-bit register bit1:bit0 to 0b10, use like this:
 *
 * @code
   volatile uint32_t * reg = (volatile uint32_t *)REG_ADDR;

   SDK_ATOMIC_LOCAL_CLEAR_AND_SET(reg, 0x03, 0x02);
   @endcode
 *
 * In this example, the register bit1:bit0 are cleared and bit1 is set, as a result,
 * register bit1:bit0 = 0b10.
 *
 * @note For the platforms don't support exclusive load and store, these macros
 * disable the global interrupt to pretect the modification.
 *
 * @note These macros only guarantee the local processor atomic operations. For
 * the multi-processor devices, use hardware semaphore such as SEMA42 to
 * guarantee exclusive access if necessary.
 *
 * @{
 */

/* clang-format off */
#if ((defined(__ARM_ARCH_7M__     ) && (__ARM_ARCH_7M__      == 1)) || \
     (defined(__ARM_ARCH_7EM__    ) && (__ARM_ARCH_7EM__     == 1)) || \
     (defined(__ARM_ARCH_8M_MAIN__) && (__ARM_ARCH_8M_MAIN__ == 1)) || \
     (defined(__ARM_ARCH_8M_BASE__) && (__ARM_ARCH_8M_BASE__ == 1)))
/* clang-format on */

/* If the LDREX and STREX are supported, use them. */
#define _SDK_ATOMIC_LOCAL_OPS_1BYTE(addr, val, ops) \
    do                                              \
    {                                               \
        (val) = __LDREXB(addr);                     \
        (ops);                                      \
    } while (0UL != __STREXB((val), (addr)))

#define _SDK_ATOMIC_LOCAL_OPS_2BYTE(addr, val, ops) \
    do                                              \
    {                                               \
        (val) = __LDREXH(addr);                     \
        (ops);                                      \
    } while (0UL != __STREXH((val), (addr)))

#define _SDK_ATOMIC_LOCAL_OPS_4BYTE(addr, val, ops) \
    do                                              \
    {                                               \
        (val) = __LDREXW(addr);                     \
        (ops);                                      \
    } while (0UL != __STREXW((val), (addr)))

static inline void _SDK_AtomicLocalAdd1Byte(volatile uint8_t *addr, uint8_t val)
{
    uint8_t s_val;

    _SDK_ATOMIC_LOCAL_OPS_1BYTE(addr, s_val, s_val += val);
}

static inline void _SDK_AtomicLocalAdd2Byte(volatile uint16_t *addr, uint16_t val)
{
    uint16_t s_val;

    _SDK_ATOMIC_LOCAL_OPS_2BYTE(addr, s_val, s_val += val);
}

static inline void _SDK_AtomicLocalAdd4Byte(volatile uint32_t *addr, uint32_t val)
{
    uint32_t s_val;

    _SDK_ATOMIC_LOCAL_OPS_4BYTE(addr, s_val, s_val += val);
}

static inline void _SDK_AtomicLocalSub1Byte(volatile uint8_t *addr, uint8_t val)
{
    uint8_t s_val;

    _SDK_ATOMIC_LOCAL_OPS_1BYTE(addr, s_val, s_val -= val);
}

static inline void _SDK_AtomicLocalSub2Byte(volatile uint16_t *addr, uint16_t val)
{
    uint16_t s_val;

    _SDK_ATOMIC_LOCAL_OPS_2BYTE(addr, s_val, s_val -= val);
}

static inline void _SDK_AtomicLocalSub4Byte(volatile uint32_t *addr, uint32_t val)
{
    uint32_t s_val;

    _SDK_ATOMIC_LOCAL_OPS_4BYTE(addr, s_val, s_val -= val);
}

static inline void _SDK_AtomicLocalSet1Byte(volatile uint8_t *addr, uint8_t bits)
{
    uint8_t s_val;

    _SDK_ATOMIC_LOCAL_OPS_1BYTE(addr, s_val, s_val |= bits);
}

static inline void _SDK_AtomicLocalSet2Byte(volatile uint16_t *addr, uint16_t bits)
{
    uint16_t s_val;

    _SDK_ATOMIC_LOCAL_OPS_2BYTE(addr, s_val, s_val |= bits);
}

static inline void _SDK_AtomicLocalSet4Byte(volatile uint32_t *addr, uint32_t bits)
{
    uint32_t s_val;

    _SDK_ATOMIC_LOCAL_OPS_4BYTE(addr, s_val, s_val |= bits);
}

static inline void _SDK_AtomicLocalClear1Byte(volatile uint8_t *addr, uint8_t bits)
{
    uint8_t s_val;

    _SDK_ATOMIC_LOCAL_OPS_1BYTE(addr, s_val, s_val &= ~bits);
}

static inline void _SDK_AtomicLocalClear2Byte(volatile uint16_t *addr, uint16_t bits)
{
    uint16_t s_val;

    _SDK_ATOMIC_LOCAL_OPS_2BYTE(addr, s_val, s_val &= ~bits);
}

static inline void _SDK_AtomicLocalClear4Byte(volatile uint32_t *addr, uint32_t bits)
{
    uint32_t s_val;

    _SDK_ATOMIC_LOCAL_OPS_4BYTE(addr, s_val, s_val &= ~bits);
}

static inline void _SDK_AtomicLocalToggle1Byte(volatile uint8_t *addr, uint8_t bits)
{
    uint8_t s_val;

    _SDK_ATOMIC_LOCAL_OPS_1BYTE(addr, s_val, s_val ^= bits);
}

static inline void _SDK_AtomicLocalToggle2Byte(volatile uint16_t *addr, uint16_t bits)
{
    uint16_t s_val;

    _SDK_ATOMIC_LOCAL_OPS_2BYTE(addr, s_val, s_val ^= bits);
}

static inline void _SDK_AtomicLocalToggle4Byte(volatile uint32_t *addr, uint32_t bits)
{
    uint32_t s_val;

    _SDK_ATOMIC_LOCAL_OPS_4BYTE(addr, s_val, s_val ^= bits);
}

static inline void _SDK_AtomicLocalClearAndSet1Byte(volatile uint8_t *addr, uint8_t clearBits, uint8_t setBits)
{
    uint8_t s_val;

    _SDK_ATOMIC_LOCAL_OPS_1BYTE(addr, s_val, s_val = (s_val & ~clearBits) | setBits);
}

static inline void _SDK_AtomicLocalClearAndSet2Byte(volatile uint16_t *addr, uint16_t clearBits, uint16_t setBits)
{
    uint16_t s_val;

    _SDK_ATOMIC_LOCAL_OPS_2BYTE(addr, s_val, s_val = (s_val & ~clearBits) | setBits);
}

static inline void _SDK_AtomicLocalClearAndSet4Byte(volatile uint32_t *addr, uint32_t clearBits, uint32_t setBits)
{
    uint32_t s_val;

    _SDK_ATOMIC_LOCAL_OPS_4BYTE(addr, s_val, s_val = (s_val & ~clearBits) | setBits);
}

#define SDK_ATOMIC_LOCAL_ADD(addr, val)                                                                                        \
    ((1UL == sizeof(*(addr))) ?                                                                                                \
         _SDK_AtomicLocalAdd1Byte((volatile uint8_t *)(volatile void *)(addr), (uint8_t)(val)) :                               \
         ((2UL == sizeof(*(addr))) ? _SDK_AtomicLocalAdd2Byte((volatile uint16_t *)(volatile void *)(addr), (uint16_t)(val)) : \
                                     _SDK_AtomicLocalAdd4Byte((volatile uint32_t *)(volatile void *)(addr), (uint32_t)(val))))

#define SDK_ATOMIC_LOCAL_SUB(addr, val)                                                                                        \
    ((1UL == sizeof(*(addr))) ?                                                                                                \
         _SDK_AtomicLocalSub1Byte((volatile uint8_t *)(volatile void *)(addr), (uint8_t)(val)) :                               \
         ((2UL == sizeof(*(addr))) ? _SDK_AtomicLocalSub2Byte((volatile uint16_t *)(volatile void *)(addr), (uint16_t)(val)) : \
                                     _SDK_AtomicLocalSub4Byte((volatile uint32_t *)(volatile void *)(addr), (uint32_t)(val))))

#define SDK_ATOMIC_LOCAL_SET(addr, bits)                                                                                        \
    ((1UL == sizeof(*(addr))) ?                                                                                                 \
         _SDK_AtomicLocalSet1Byte((volatile uint8_t *)(volatile void *)(addr), (uint8_t)(bits)) :                               \
         ((2UL == sizeof(*(addr))) ? _SDK_AtomicLocalSet2Byte((volatile uint16_t *)(volatile void *)(addr), (uint16_t)(bits)) : \
                                     _SDK_AtomicLocalSet4Byte((volatile uint32_t *)(volatile void *)(addr), (uint32_t)(bits))))

#define SDK_ATOMIC_LOCAL_CLEAR(addr, bits)                                                                 \
    ((1UL == sizeof(*(addr))) ?                                                                            \
         _SDK_AtomicLocalClear1Byte((volatile uint8_t *)(volatile void *)(addr), (uint8_t)(bits)) :        \
         ((2UL == sizeof(*(addr))) ?                                                                       \
              _SDK_AtomicLocalClear2Byte((volatile uint16_t *)(volatile void *)(addr), (uint16_t)(bits)) : \
              _SDK_AtomicLocalClear4Byte((volatile uint32_t *)(volatile void *)(addr), (uint32_t)(bits))))

#define SDK_ATOMIC_LOCAL_TOGGLE(addr, bits)                                                                 \
    ((1UL == sizeof(*(addr))) ?                                                                             \
         _SDK_AtomicLocalToggle1Byte((volatile uint8_t *)(volatile void *)(addr), (uint8_t)(bits)) :        \
         ((2UL == sizeof(*(addr))) ?                                                                        \
              _SDK_AtomicLocalToggle2Byte((volatile uint16_t *)(volatile void *)(addr), (uint16_t)(bits)) : \
              _SDK_AtomicLocalToggle4Byte((volatile uint32_t *)(volatile void *)(addr), (uint32_t)(bits))))

#define SDK_ATOMIC_LOCAL_CLEAR_AND_SET(addr, clearBits, setBits)                                                                           \
    ((1UL == sizeof(*(addr))) ?                                                                                                            \
         _SDK_AtomicLocalClearAndSet1Byte((volatile uint8_t *)(volatile void *)(addr), (uint8_t)(clearBits), (uint8_t)(setBits)) :         \
         ((2UL == sizeof(*(addr))) ?                                                                                                       \
              _SDK_AtomicLocalClearAndSet2Byte((volatile uint16_t *)(volatile void *)(addr), (uint16_t)(clearBits), (uint16_t)(setBits)) : \
              _SDK_AtomicLocalClearAndSet4Byte((volatile uint32_t *)(volatile void *)(addr), (uint32_t)(clearBits), (uint32_t)(setBits))))
#else

#define SDK_ATOMIC_LOCAL_ADD(addr, val)      \
    do                                       \
    {                                        \
        uint32_t s_atomicOldInt;             \
        s_atomicOldInt = DisableGlobalIRQ(); \
        *(addr) += (val);                    \
        EnableGlobalIRQ(s_atomicOldInt);     \
    } while (0)

#define SDK_ATOMIC_LOCAL_SUB(addr, val)      \
    do                                       \
    {                                        \
        uint32_t s_atomicOldInt;             \
        s_atomicOldInt = DisableGlobalIRQ(); \
        *(addr) -= (val);                    \
        EnableGlobalIRQ(s_atomicOldInt);     \
    } while (0)

#define SDK_ATOMIC_LOCAL_SET(addr, bits)     \
    do                                       \
    {                                        \
        uint32_t s_atomicOldInt;             \
        s_atomicOldInt = DisableGlobalIRQ(); \
        *(addr) |= (bits);                   \
        EnableGlobalIRQ(s_atomicOldInt);     \
    } while (0)

#define SDK_ATOMIC_LOCAL_CLEAR(addr, bits)   \
    do                                       \
    {                                        \
        uint32_t s_atomicOldInt;             \
        s_atomicOldInt = DisableGlobalIRQ(); \
        *(addr) &= ~(bits);                  \
        EnableGlobalIRQ(s_atomicOldInt);     \
    } while (0)

#define SDK_ATOMIC_LOCAL_TOGGLE(addr, bits)  \
    do                                       \
    {                                        \
        uint32_t s_atomicOldInt;             \
        s_atomicOldInt = DisableGlobalIRQ(); \
        *(addr) ^= (bits);                   \
        EnableGlobalIRQ(s_atomicOldInt);     \
    } while (0)

#define SDK_ATOMIC_LOCAL_CLEAR_AND_SET(addr, clearBits, setBits) \
    do                                                           \
    {                                                            \
        uint32_t s_atomicOldInt;                                 \
        s_atomicOldInt = DisableGlobalIRQ();                     \
        *(addr)        = (*(addr) & ~(clearBits)) | (setBits);   \
        EnableGlobalIRQ(s_atomicOldInt);                         \
    } while (0)

#endif
/* @} */

/*! @name Timer utilities */
/* @{ */
/*! Macro to convert a microsecond period to raw count value */
#define USEC_TO_COUNT(us, clockFreqInHz) (uint64_t)(((uint64_t)(us) * (clockFreqInHz)) / 1000000U)
/*! Macro to convert a raw count value to microsecond */
#define COUNT_TO_USEC(count, clockFreqInHz) (uint64_t)((uint64_t)(count)*1000000U / (clockFreqInHz))

/*! Macro to convert a millisecond period to raw count value */
#define MSEC_TO_COUNT(ms, clockFreqInHz) (uint64_t)((uint64_t)(ms) * (clockFreqInHz) / 1000U)
/*! Macro to convert a raw count value to millisecond */
#define COUNT_TO_MSEC(count, clockFreqInHz) (uint64_t)((uint64_t)(count)*1000U / (clockFreqInHz))
/* @} */

/*! @name ISR exit barrier
 * @{
 *
 * ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
 * exception return operation might vector to incorrect interrupt.
 * For Cortex-M7, if core speed much faster than peripheral register write speed,
 * the peripheral interrupt flags may be still set after exiting ISR, this results to
 * the same error similar with errata 83869.
 */
#if (defined __CORTEX_M) && ((__CORTEX_M == 4U) || (__CORTEX_M == 7U))
#define SDK_ISR_EXIT_BARRIER __DSB()
#else
#define SDK_ISR_EXIT_BARRIER
#endif

/* @} */

/*! @name Alignment variable definition macros */
/* @{ */
#if (defined(__ICCARM__))
/*
 * Workaround to disable MISRA C message suppress warnings for IAR compiler.
 * http:/ /supp.iar.com/Support/?note=24725
 */
_Pragma("diag_suppress=Pm120")
#define SDK_PRAGMA(x) _Pragma(#x)
    _Pragma("diag_error=Pm120")
/*! Macro to define a variable with alignbytes alignment */
#define SDK_ALIGN(var, alignbytes) SDK_PRAGMA(data_alignment = alignbytes) var
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
/*! Macro to define a variable with alignbytes alignment */
#define SDK_ALIGN(var, alignbytes) __attribute__((aligned(alignbytes))) var
#elif defined(__GNUC__)
/*! Macro to define a variable with alignbytes alignment */
#define SDK_ALIGN(var, alignbytes) var __attribute__((aligned(alignbytes)))
#else
#error Toolchain not supported
#endif

/*! Macro to define a variable with L1 d-cache line size alignment */
#if defined(FSL_FEATURE_L1DCACHE_LINESIZE_BYTE)
#define SDK_L1DCACHE_ALIGN(var) SDK_ALIGN(var, FSL_FEATURE_L1DCACHE_LINESIZE_BYTE)
#endif
/*! Macro to define a variable with L2 cache line size alignment */
#if defined(FSL_FEATURE_L2CACHE_LINESIZE_BYTE)
#define SDK_L2CACHE_ALIGN(var) SDK_ALIGN(var, FSL_FEATURE_L2CACHE_LINESIZE_BYTE)
#endif

/*! Macro to change a value to a given size aligned value */
#define SDK_SIZEALIGN(var, alignbytes) \
    ((unsigned int)((var) + ((alignbytes)-1U)) & (unsigned int)(~(unsigned int)((alignbytes)-1U)))
/* @} */

/*! @name Non-cacheable region definition macros */
/* For initialized non-zero non-cacheable variables, please using "AT_NONCACHEABLE_SECTION_INIT(var) ={xx};" or
 * "AT_NONCACHEABLE_SECTION_ALIGN_INIT(var) ={xx};" in your projects to define them, for zero-inited non-cacheable
 * variables, please using "AT_NONCACHEABLE_SECTION(var);" or "AT_NONCACHEABLE_SECTION_ALIGN(var);" to define them,
 * these zero-inited variables will be initialized to zero in system startup.
 */
/* @{ */

#if ((!(defined(FSL_FEATURE_HAS_NO_NONCACHEABLE_SECTION) && FSL_FEATURE_HAS_NO_NONCACHEABLE_SECTION)) && \
     defined(FSL_FEATURE_L1ICACHE_LINESIZE_BYTE))

#if (defined(__ICCARM__))
#define AT_NONCACHEABLE_SECTION(var)                   var @"NonCacheable"
#define AT_NONCACHEABLE_SECTION_ALIGN(var, alignbytes) SDK_PRAGMA(data_alignment = alignbytes) var @"NonCacheable"
#define AT_NONCACHEABLE_SECTION_INIT(var)              var @"NonCacheable.init"
#define AT_NONCACHEABLE_SECTION_ALIGN_INIT(var, alignbytes) \
    SDK_PRAGMA(data_alignment = alignbytes) var @"NonCacheable.init"

#elif (defined(__CC_ARM) || defined(__ARMCC_VERSION))
#define AT_NONCACHEABLE_SECTION_INIT(var) __attribute__((section("NonCacheable.init"))) var
#define AT_NONCACHEABLE_SECTION_ALIGN_INIT(var, alignbytes) \
    __attribute__((section("NonCacheable.init"))) __attribute__((aligned(alignbytes))) var
#if (defined(__CC_ARM))
#define AT_NONCACHEABLE_SECTION(var) __attribute__((section("NonCacheable"), zero_init)) var
#define AT_NONCACHEABLE_SECTION_ALIGN(var, alignbytes) \
    __attribute__((section("NonCacheable"), zero_init)) __attribute__((aligned(alignbytes))) var
#else
#define AT_NONCACHEABLE_SECTION(var) __attribute__((section(".bss.NonCacheable"))) var
#define AT_NONCACHEABLE_SECTION_ALIGN(var, alignbytes) \
    __attribute__((section(".bss.NonCacheable"))) __attribute__((aligned(alignbytes))) var
#endif

#elif (defined(__GNUC__))
#if defined(__ARM_ARCH_8A__) /* This macro is ARMv8-A specific */
#define __CS "//"
#else
#define __CS "@"
#endif

/* For GCC, when the non-cacheable section is required, please define "__STARTUP_INITIALIZE_NONCACHEDATA"
 * in your projects to make sure the non-cacheable section variables will be initialized in system startup.
 */
#define AT_NONCACHEABLE_SECTION_INIT(var) __attribute__((section("NonCacheable.init"))) var
#define AT_NONCACHEABLE_SECTION_ALIGN_INIT(var, alignbytes) \
    __attribute__((section("NonCacheable.init"))) var __attribute__((aligned(alignbytes)))
#define AT_NONCACHEABLE_SECTION(var) __attribute__((section("NonCacheable,\"aw\",%nobits " __CS))) var
#define AT_NONCACHEABLE_SECTION_ALIGN(var, alignbytes) \
    __attribute__((section("NonCacheable,\"aw\",%nobits " __CS))) var __attribute__((aligned(alignbytes)))
#else
#error Toolchain not supported.
#endif

#else

#define AT_NONCACHEABLE_SECTION(var)                        var
#define AT_NONCACHEABLE_SECTION_ALIGN(var, alignbytes)      SDK_ALIGN(var, alignbytes)
#define AT_NONCACHEABLE_SECTION_INIT(var)                   var
#define AT_NONCACHEABLE_SECTION_ALIGN_INIT(var, alignbytes) SDK_ALIGN(var, alignbytes)

#endif

/* @} */

/*!
 * @name Time sensitive region
 * @{
 */
#if (defined(__ICCARM__))
#define AT_QUICKACCESS_SECTION_CODE(func) func @"CodeQuickAccess"
#define AT_QUICKACCESS_SECTION_DATA(var)  var @"DataQuickAccess"
#define AT_QUICKACCESS_SECTION_DATA_ALIGN(var, alignbytes) \
    SDK_PRAGMA(data_alignment = alignbytes) var @"DataQuickAccess"
#elif (defined(__CC_ARM) || defined(__ARMCC_VERSION))
#define AT_QUICKACCESS_SECTION_CODE(func) __attribute__((section("CodeQuickAccess"), __noinline__)) func
#define AT_QUICKACCESS_SECTION_DATA(var)  __attribute__((section("DataQuickAccess"))) var
#define AT_QUICKACCESS_SECTION_DATA_ALIGN(var, alignbytes) \
    __attribute__((section("DataQuickAccess"))) __attribute__((aligned(alignbytes))) var
#elif (defined(__GNUC__))
#define AT_QUICKACCESS_SECTION_CODE(func) __attribute__((section("CodeQuickAccess"), __noinline__)) func
#define AT_QUICKACCESS_SECTION_DATA(var)  __attribute__((section("DataQuickAccess"))) var
#define AT_QUICKACCESS_SECTION_DATA_ALIGN(var, alignbytes) \
    __attribute__((section("DataQuickAccess"))) var __attribute__((aligned(alignbytes)))
#else
#error Toolchain not supported.
#endif /* defined(__ICCARM__) */

/*! @name Ram Function */
#if (defined(__ICCARM__))
#define RAMFUNCTION_SECTION_CODE(func) func @"RamFunction"
#elif (defined(__CC_ARM) || defined(__ARMCC_VERSION))
#define RAMFUNCTION_SECTION_CODE(func) __attribute__((section("RamFunction"))) func
#elif (defined(__GNUC__))
#define RAMFUNCTION_SECTION_CODE(func) __attribute__((section("RamFunction"))) func
#else
#error Toolchain not supported.
#endif /* defined(__ICCARM__) */
/* @} */

#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
        void DefaultISR(void);
#endif



#endif /* _FSL_COMMON_ARM_H_ */
