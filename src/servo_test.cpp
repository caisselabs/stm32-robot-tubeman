//
// Copyright (c) 2024 Michael Caisse
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// ----------------------------------------------------------------------------
//
// Timer interrupt hooked to a priority to a timer scheduler
//
//
// ----------------------------------------------------------------------------
#include "interrupt.hpp"  // for setup_interrupts

// a concurrency policy is needed by the async library
#include "blinky_concurrency.hpp"

#include "fixed_priority_scheduler.hpp"
#include "timer_scheduler.hpp"

#include "servo/lx_16a_servo.hpp"

#include <caisselabs/stm32/stm32l432.hpp>

#include <async/sequence.hpp>
#include <async/repeat.hpp>
#include <async/continue_on.hpp>
#include <async/start_detached.hpp>
#include <async/schedulers/priority_scheduler.hpp>
#include <async/schedulers/task_manager.hpp>
#include <async/schedulers/time_scheduler.hpp>

#include <cstdint>
#include <chrono>
#include <tuple>

// method to initialize basic board functionality
void initialize_board();


namespace stm32 = caisselabs::stm32;

using namespace std::chrono_literals;
using namespace groov::literals;

volatile std::uint32_t sleep_temp = 0;
void dumb_sleep() {
  for (int i=0; i<1; ++i) {
    for (int j=0; j<0xffff; ++j) {
      sleep_temp = sleep_temp + 1;
    }
  }
}

auto update_position_value(std::uint16_t angle, std::uint16_t increment) {
    angle += increment;
    
    if (angle < 300) {
      increment = ~increment;
      angle = 300;
    } else if (angle > 700) {
      increment = ~increment;
      angle = 700;
    }

    return std::make_tuple(angle, increment);
}


int main() {

  initialize_board();
  setup_interrupts();
  initialize_timer();
  setup_servo_comms();

  
  auto delay = [](auto v) {
    return
      async::continue_on(async::time_scheduler{v});
  };

  auto led_on  = groov::write(stm32::gpiob("odr.3"_f=true));
  auto led_off = groov::write(stm32::gpiob("odr.3"_f=false));
  auto on_cycle  = led_on  | delay(300ms);
  auto off_cycle = led_off | delay(1s);

  
  async::sender auto blinky =
      on_cycle
    | async::seq(off_cycle)
    ;

  //  auto s = blinky | async::repeat() | async::start_detached();



  std::uint16_t servo1_increment = 0x02;
  std::uint16_t servo2_increment = 0x05;
  std::uint16_t servo3_increment = 0x02;
  std::uint16_t servo4_increment = 0x05;

  std::uint16_t servo1_angle = 500;
  std::uint16_t servo2_angle = 500;
  std::uint16_t servo3_angle = 500;
  std::uint16_t servo4_angle = 500;
  
  
  while(true) {
    // spin little heater, spin

    // about 40ms
    dumb_sleep();

    // position read
    //write_command(1, 0x1c);

    // move things
    write_command(1, 0x01, (servo1_angle & 0xff), (servo1_angle >> 8), 0x20, 0x00);
    write_command(2, 0x01, (servo2_angle & 0xff), (servo2_angle >> 8), 0x20, 0x00);
    write_command(3, 0x01, (servo3_angle & 0xff), (servo3_angle >> 8), 0x20, 0x00);
    write_command(4, 0x01, (servo4_angle & 0xff), (servo4_angle >> 8), 0x20, 0x00);

    std::tie(servo1_angle, servo1_increment) = update_position_value(servo1_angle, servo1_increment);
    std::tie(servo2_angle, servo2_increment) = update_position_value(servo2_angle, servo2_increment);
    std::tie(servo3_angle, servo3_increment) = update_position_value(servo3_angle, servo3_increment);
    std::tie(servo4_angle, servo4_increment) = update_position_value(servo4_angle, servo4_increment);

  }
  
}
