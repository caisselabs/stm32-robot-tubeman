# Robo Tubeman

The project behind Michael's cppcon 2024 talk.

The dev board is a STM32 NUCLEO-L432KC which has an STM32L432KC device.

> [!Note]
> There are many ways to accomplish the tasks on this page such as loading flash,
> configuring cmake, and envoking builds. The steps listed aren't event the "best"
> methods. Hopefully they provide an easy-enough experience for those new to the tools.
> For those with experience, please use your prefered techniques.


# Toolchain

## Compiler

I'm using the official [Arm GNU Toolchain](https://developer.arm.com/Tools%20and%20Software/GNU%20Toolchain) release 13.2 on an Intel MacBook Pro. The toolchain works easily in Linux also.

The project's `toolchains/gcc-arm.cmake` file is used by the cmake gcc preset to point at the specified compiler. Please update this file based on your installation/paths.


## Debugger

If you are interested in using a debugger you will need to install GDB. An easy way to debug on-target is by starting a GDB server via openocd and then connecting to it with GDB. This is also useful for other things like access flash or processor state.

Learn more here about the [Arm GNU Toolchain](https://learn.arm.com/install-guides/gcc/arm-gnu/).

### Running GDB

Start `openocd` as described below. This will open a debug port to connect on.

Start the ARM gdb. For me, the binary is `arm-none-eabi-gdb`. Tell gdb to connect to the server on the port OCD is listening. For example:

```bash
arm-none-eabi-gdb
(gdb) target remote :3333
```


## Flashing Memory

A key difference between a microcontroller (MCU) and microprocessor (MPU) is that a microcontroller typically uses on-chip embedded flash memory to store and execute memory from. Regardless if the flash is on-chip or located in a serial flash device, it must be programmed with the desired content. With the proper parameters this can often be done via openocd.

### Flashing from GDB

Loading the image via the debugger will flash the memory. You can simply start the debugger and then load the image (make sure to start `openocd` first):

```bash
arm-none-eabi-gdb
(gdb) target remote :3333
(gdb) file servo_test
(gdb) load servo_test
(gdb) c
```

The above sequence will load the symbols into the debugger, the image into flash, and then continue executing.


## openocd

As mentioned above, openocd is being used for a lot of direct access tasks.

Start running the `openocd` server for gdb to attach:

```bash
openocd -f openocd/st_nucleo_l4.cfg
```


### Mac Instructions

These are the things I did to make `openocd` work on a Mac.

- Using brew, install openocd

```bash
brew install openocd
```

- Find the directory location of where openocd is installed

```bash
which openocd | xargs ls -l
```

  The symbolic link path will allow us to find the board configurations
  
- Start the server

```bash
openocd -f /path/to/openocd/install/share/openocd/scripts/board/st_nucleo_l4.cfg
```

For me, the path was:

```bash
openocd -f /usr/local/Cellar/open-ocd/0.12.0_1/share/openocd/scripts/board/st_nucleo_l4.cfg
```


# Building

You can use the cmake preset to get things up and going. From the repository's top level directory:

```bash
cmake --preset=gcc
```

The preset will generate the cmake files into the `build` directory and use the Ninja generator. Required packages will be downloaded via CPM. If you do not have a specific cache location set for CPM, the collateral will be fetched to `build/_dep`.

```bash
ninja -C build
```

This will result in the elf file `build/blinky` which can be loaded onto the target using gdb as explained above.


# License

This project is released under the Boost Software License 1.0 ([BSL 1.0](https://www.boost.org/LICENSE_1_0.txt)) with the exception of the following files:

 - `startup/startup_gcc.s` is from the Arm gcc release and has been modified to include additional interrupt vectors for the STM32L432KC. See file's header for license information.
 - `ldscripts/*` - files in this directory were linker scripts that originated with Arm gcc compiler and have been modified for the dev board in use.
 - `openocd/stm32_nucleo_l4.cfg` - GPL 2.0. Originated from `openocd` distribution.

