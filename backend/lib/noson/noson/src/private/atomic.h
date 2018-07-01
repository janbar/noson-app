/*
 *      Copyright (C) 2014-2015 Jean-Luc Barriere
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 3, or (at your option)
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301 USA
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#ifndef ATOMIC_H
#define	ATOMIC_H

#include "local_config.h"

#ifdef __GNUC__

#if defined __arm__ && (!defined __thumb__ || defined __thumb2__)
/* The __ARM_ARCH define is provided by gcc 4.8.  Construct it otherwise.  */
#ifndef __ARM_ARCH
#ifdef __ARM_ARCH_2__
#define __ARM_ARCH 2
#elif defined (__ARM_ARCH_3__) || defined (__ARM_ARCH_3M__)
#define __ARM_ARCH 3
#elif defined (__ARM_ARCH_4__) || defined (__ARM_ARCH_4T__)
#define __ARM_ARCH 4
#elif defined (__ARM_ARCH_5__) || defined (__ARM_ARCH_5E__) \
        || defined(__ARM_ARCH_5T__) || defined(__ARM_ARCH_5TE__) \
        || defined(__ARM_ARCH_5TEJ__)
#define __ARM_ARCH 5
#elif defined (__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) \
        || defined (__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__) \
        || defined (__ARM_ARCH_6K__) || defined(__ARM_ARCH_6T2__)
#define __ARM_ARCH 6
#elif defined (__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) \
        || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) \
        || defined(__ARM_ARCH_7EM__)
#define __ARM_ARCH 7
#endif
#endif
#endif

namespace NSROOT
{
  template<typename T>
  class atomic
  {
  public:
    typedef volatile T atomic_t;

    atomic(T val) : m_val(val) {}

    atomic_t load()
    {
      return m_val;
    }

  private:
    atomic_t m_val;
  };

  template<>
  class atomic<int>
  {
  public:
    typedef volatile int atomic_t;

    atomic(int val) : m_val(val) {}

    int __attribute__((always_inline)) load()
    {
      return m_val;
    }

    int __attribute__((always_inline)) operator()()
    {
      return load();
    }

    int __attribute__((always_inline)) add_fetch(int amount)
    {
      atomic_t __val;

#if defined __mips__
      int temp;
      __asm__ volatile (
        "    .set   arch=r4000\n"
        "1:  ll     %0, %1\n"       /* load old value */
        "    addu   %2, %0, %3\n"   /* calculate new value */
        "    sc     %2, %1\n"       /* attempt to store */
        "    beqzl  %2, 1b\n"       /* spin if failed */
        "    .set   mips0\n"
        : "=&r" (__val), "=m" (m_val), "=&r" (temp)
        : "r" (amount), "m" (m_val));
      /* __val is the old value, so normalize it. */
      __val += amount;

#elif defined __i386__ || defined __i486__ || defined __i586__ || defined __i686__ || defined __x86_64__
      __asm__ volatile (
        "lock xaddl %0, (%1);"
        : "=r" (__val)
        : "r" (&m_val), "0" (amount)
        : "cc", "memory"
        );
      /* __val is the old value, so normalize it. */
      __val += amount;

#elif defined __powerpc__ || defined __ppc__ || defined __ppc64__
      __asm__ volatile (
        "1:  lwarx   %0,0,%1\n"
        "    add     %0,%2,%0\n"
        "    dcbt    %0,%1\n"
        "    stwcx.  %0,0,%1\n"
        "    bne-    1b\n"
        "    isync\n"
        : "=&r" (__val)
        : "r" (&m_val), "r" (amount)
        : "cc", "memory");

#elif defined __sparc__ || defined __sparc64__
      atomic_t __old, __new = m_val;
      do
      {
        __old = __new;
        __new = __old + amount;
        /* compare and swap: if (*a == b) swap(*a, c) else c = *a */
        __asm__ volatile (
          "cas [%2], %3, %0"
          : "=&r" (__new)
          : "" (__new), "r" (&m_val), "r" (__old)
          : "memory");
      }
      while (__new != __old);
      __val = __old + amount;

#elif (defined __ARM_ARCH && __ARM_ARCH == 7)
      __asm__ volatile (
        "    dmb     ish\n"           /* Memory barrier */
        "1:  ldrex   %0, [%1]\n"
        "    add     %0, %0, %2\n"
        "    strex   r1, %0, [%1]\n"
        "    cmp     r1, #0\n"
        "    bne     1b\n"
        "    dmb     ish\n"           /* Memory barrier */
        : "=&r" (__val)
        : "r" (&m_val), "r" (amount)
        : "r1", "memory");

#elif (defined __ARM_ARCH && __ARM_ARCH == 6)
      __asm__ volatile (
        "mcr p15, 0, %0, c7, c10, 5"  /* Memory barrier */
        : : "r" (0) : "memory");
      __asm__ volatile (
        "1:  ldrex   %0, [%1]\n"
        "    add     %0, %0, %2\n"
        "    strex   r1, %0, [%1]\n"
        "    cmp     r1, #0\n"
        "    bne     1b\n"
        : "=&r" (__val)
        : "r" (&m_val), "r" (amount)
        : "r1");
      __asm__ volatile (
        "mcr p15, 0, %0, c7, c10, 5"  /* Memory barrier */
        : : "r" (0) : "memory");

#elif (defined __ARM_ARCH && __ARM_ARCH < 6)
      int tmp1, tmp2;
      __asm__ volatile (
        "0:  ldr     %0, [%3]\n"
        "    add     %1, %0, %4\n"
        "    swp     %2, %1, [%3]\n"
        "    cmp     %0, %2\n"
        "    swpne   %0, %2, [%3]\n"
        "    bne     0b\n"
        : "=&r" (tmp1), "=&r" (__val), "=&r" (tmp2)
        : "r" (&m_val), "r" (amount)
        : "cc", "memory");

#elif defined __aarch64__
      unsigned long tmp;
      __asm__ volatile (
        "    dmb     ish\n"           /* Memory barrier */
        "1:  ldxr    %w0, %2\n"
        "    add     %w0, %w0, %w3\n"
        "    stlxr   %w1, %w0, %2\n"
        "    cbnz    %w1, 1b\n"
        "    dmb     ish\n"           /* Memory barrier */
        : "=&r" (__val), "=&r" (tmp), "+Q" (m_val)
        : "Ir" (amount)
        : "memory");

#else
/* warning unknown architecture, atomic increment is not... */
#ifndef ATOMIC_NOATOMIC
#define ATOMIC_NOATOMIC
#endif
      __val = m_val += amount;

#endif
      return __val;
    }

    int __attribute__((always_inline)) operator++()
    {
      return add_fetch(1);
    }

    int __attribute__((always_inline)) sub_fetch(int amount)
    {
      atomic_t __val;

#if defined __mips__
      int temp;
      __asm__ volatile (
        "    .set   arch=r4000\n"
        "1:  ll     %0, %1\n"       /* load old value */
        "    subu   %2, %0, %3\n"   /* calculate new value */
        "    sc     %2, %1\n"       /* attempt to store */
        "    beqzl  %2, 1b\n"       /* spin if failed */
        "    .set   mips0\n"
        : "=&r" (__val), "=m" (m_val), "=&r" (temp)
        : "r" (amount), "m" (m_val));
      /* __val is the old value, so normalize it */
      __val -= amount;

#elif defined __i386__ || defined __i486__ || defined __i586__ || defined __i686__ || defined __x86_64__
      __asm__ volatile (
        "lock xaddl %0, (%1);"
        : "=r" (__val)
        : "r" (&m_val), "0" (-amount)
        : "cc", "memory"
        );
      /* __val is the pre-decrement value, so normalize it */
      __val -= amount;

#elif defined __powerpc__ || defined __ppc__ || defined __ppc64__
      __asm__ volatile (
        "1:  lwarx   %0,0,%1\n"
        "    subf    %0,%2,%0\n"
        "    dcbt    %0,%1\n"
        "    stwcx.  %0,0,%1\n"
        "    bne-    1b\n"
        "    isync\n"
        : "=&r" (__val)
        : "r" (&m_val), "r" (amount)
        : "cc", "memory");

#elif defined __sparc__ || defined __sparc64__
      atomic_t __old, __new = m_val;
      do
      {
        __old = __new;
        __new = __old - amount;
        /* compare and swap: if (*a == b) swap(*a, c) else c = *a */
        __asm__ volatile (
          "cas [%2], %3, %0"
          : "=&r" (__new)
          : "" (__new), "r" (&m_val), "r" (__old)
          : "memory");
      }
      while (__new != __old);
      __val = __old - amount;

#elif (defined __ARM_ARCH && __ARM_ARCH == 7)
      __asm__ volatile (
        "    dmb     ish\n"           /* Memory barrier */
        "1:  ldrex   %0, [%1]\n"
        "    sub     %0, %0, %2\n"
        "    strex   r1, %0, [%1]\n"
        "    cmp     r1, #0\n"
        "    bne     1b\n"
        "    dmb     ish\n"           /* Memory barrier */
        : "=&r" (__val)
        : "r" (&m_val), "r" (amount)
        : "r1", "memory");

#elif (defined __ARM_ARCH && __ARM_ARCH == 6)
      __asm__ volatile (
        "mcr p15, 0, %0, c7, c10, 5"  /* Memory barrier */
        : : "r" (0) : "memory");
      __asm__ volatile (
        "1:  ldrex   %0, [%1]\n"
        "    sub     %0, %0, %2\n"
        "    strex   r1, %0, [%1]\n"
        "    cmp     r1, #0\n"
        "    bne     1b\n"
        : "=&r" (__val)
        : "r" (&m_val), "r" (amount)
        : "r1");
      __asm__ volatile (
        "mcr p15, 0, %0, c7, c10, 5"  /* Memory barrier */
        : : "r" (0) : "memory");

#elif (defined __ARM_ARCH && __ARM_ARCH < 6)
      int tmp1, tmp2;
      __asm__ volatile (
        "0:  ldr     %0, [%3]\n"
        "    add     %1, %0, %4\n"
        "    swp     %2, %1, [%3]\n"
        "    cmp     %0, %2\n"
        "    swpne   %0, %2, [%3]\n"
        "    bne     0b\n"
        : "=&r" (tmp1), "=&r" (__val), "=&r" (tmp2)
        : "r" (&m_val), "r" (-amount)
        : "cc", "memory");

#elif defined __aarch64__
      unsigned long tmp;
      __asm__ volatile (
        "    dmb     ish\n"           /* Memory barrier */
        "1:  ldxr    %w0, %2\n"
        "    sub     %w0, %w0, %w3\n"
        "    stlxr   %w1, %w0, %2\n"
        "    cbnz    %w1, 1b\n"
        "    dmb     ish\n"           /* Memory barrier */
        : "=&r" (__val), "=&r" (tmp), "+Q" (m_val)
        : "Ir" (amount)
        : "memory");

#else
/* warning unknown architecture, atomic deccrement is not... */
#ifndef ATOMIC_NOATOMIC
#define ATOMIC_NOATOMIC
#endif
      __val = m_val -= amount;

#endif
      return __val;
    }

    int __attribute__((always_inline)) operator--()
    {
      return sub_fetch(1);
    }

  private:
    atomic_t m_val;
  };
}

#else
#ifndef ATOMIC_NOATOMIC
#define ATOMIC_NOATOMIC
#endif
#endif

#endif	/* ATOMIC_H */
