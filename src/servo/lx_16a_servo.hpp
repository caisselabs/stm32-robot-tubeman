//
// Copyright (c) 2024 Michael Caisse
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// STM32 register descriptions for the USART device.
//
#pragma once

#include <caisselabs/stm32/stm32l432.hpp>

#include <groov/config.hpp>
#include <groov/path.hpp>
#include <groov/resolve.hpp>
#include <groov/write.hpp>
#include <groov/write_spec.hpp>
#include <groov/value_path.hpp>
#include <async/just.hpp>
#include <async/sequence.hpp>
#include <async/sync_wait.hpp>

#include <span>
#include <array>

using namespace groov::literals;

inline void setup_servo_comms() {

  groov::write(
      stm32::rcc("apb2enr.USART1EN"_f = true,
                 "ahb2enr.GPIOAEN"_f = true))
    | async::sync_wait();


  groov::write(
    stm32::gpioa(
      // PA9 setup - USART1_TX
      "moder.9"_f  = stm32::gpio::mode_t::alternate,
      "afrh.9"_f   = stm32::gpio::afsel_t::AF7,
      "ospeedr.9"_f = stm32::gpio::speed_t::high_speed,
      "otyper.9"_f  = stm32::gpio::outtype_t::open_drain,
      //"pupdr.9"_f   = stm32::gpio::pupd_t::pull_up
      "pupdr.9"_f   = stm32::gpio::pupd_t::none

      // // PA10 setup - USART1_CK
      // "moder.10"_f  = stm32::gpio::mode_t::alternate,
      // "afrh.10"_f   = stm32::gpio::afsel_t::AF7,
      // "ospeedr.10"_f = stm32::gpio::speed_t::low_speed,
      // "otyper.10"_f  = stm32::gpio::outtype_t::push_pull,
      // "pupdr.10"_f   = stm32::gpio::pupd_t::none
    ))
    | async::sync_wait();

  auto init_usart1 =
    stm32::usart1 (
      // TODO: fix when split fields work
      // 1 start bit, 8 data bits, n stop
      "cr1.M1"_f = false,
      "cr1.M0"_f = false,

      "cr3.HDSEL"_f = true,  // half-duplex
      //"cr3.HDSEL"_f = false,  // full-duplex
      "cr2.LINEN"_f = false, //   required to be cleared for half-duplex
      "cr3.SCEN"_f  = false, //   required to be cleared for half-duplex
      "cr3.IREN"_f  = false, //   required to be cleared for half-duplex

      "brr.BRR"_f = 16'000'000/115200
      
    );

  auto usart1_disable = stm32::usart1("cr1.UE"_f = false, "cr1.TE"_f = false, "cr1.RE"_f = false);
  auto usart1_enable  = stm32::usart1("cr1.UE"_f = true , "cr1.TE"_f = true , "cr1.RE"_f = true);

  groov::write(usart1_disable)
    | async::seq(groov::write(init_usart1))
    | async::seq(groov::write(usart1_enable))
    | async::sync_wait();
}




inline void servo_send(std::span<const std::uint8_t> cmd) {

  if (cmd.size() == 0) return;
  
  auto iter = cmd.begin();
  auto end_iter = cmd.end();

  auto ser_out = (volatile std::uint32_t * const)(0x4001'3828);
  auto isr = (volatile std::uint32_t * const)(0x4001'381c);
  // auto ser_out = (volatile std::uint32_t * const)(0x40004428);
  // auto isr = (volatile std::uint32_t * const)(0x4000441c);

  while (iter != end_iter) {
    *ser_out = *iter;
    while((*isr & (1<<7)) == 0);
    ++iter;
  }

  // std::uint8_t check_sum = 0;
  // iter = cmd.begin();
  // std::advance(iter, 2);
  // while (iter != end_iter) {
  //   check_sum += *iter++;
  // }
  // send checksum
  // *ser_out = ~check_sum;
  // while((*isr & (1<<7)) == 0);
}

template <typename ... T>
inline bool write_command(std::uint8_t id, std::uint8_t cmd, T ... params) {

  constexpr std::uint8_t byte_length = 3 + sizeof...(params);
  std::uint8_t check_sum = id;
  check_sum += byte_length;
  check_sum += cmd;
  check_sum += (std::uint8_t{0} + ... + static_cast<std::uint8_t>(params));

  std::array<std::uint8_t, byte_length+3>
    data{0x55, 0x55, id, byte_length, cmd,
	 static_cast<std::uint8_t>(params)...,
	 ~check_sum};
  //data{0x55, 0x55, 0x08, 0x03, 0x01, 0xe8, 0x03, 0x01, 0x20, 0x03};
  //data{0x55, 0x55, id, byte_length - 2, cmd, static_cast<std::uint8_t>(params)...};

  servo_send(data);
  return true;
}


