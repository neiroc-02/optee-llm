# OP-TEE Trusted Application

## Notes (In Development)
Goals:
- Develop an OP-TEE Trusted App that can print the debug outputs from the trusted world to the secure world
- Start by getting a Hello World, then an Increment, then debug outputs
- Just get familiar with the OP-TEE toolkit

<!---
Note:
Packages I couldn't install
- netcat (installed subversions of netcat)
- lilibftdi-dev




--->

## Introduction

## Setup
Note: These setup steps are for the Jetson Orin Nano Board 
1.  First download all the Prerequisites listed in this [link](https://optee.readthedocs.io/en/latest/building/prerequisites.html#prerequisites).
2.  Then download the Android source code [here](https://source.android.com/docs/setup/download).
3.  Then download the Driver Package BSP tools and the Bootline Toolchain gcc 9.3 from [here](https://developer.nvidia.com/embedded/jetson-linux-r3541).
4.  Unzip all the downloaded files
5.  Export the target board device name: `export NV_TARGET_BOARD=234`
6.  Within `Linux_for_Tegra/sources` run `./source_sync.sh -t jetson_35.4.1`
7.  Export more environment variables:
        - `export CROSS_COMPILE_AARCH64_PATH=/path/to/aarch64-glibc-stable-final`
        - `export CROSS_COMPILE_AARCH64=/path/to/aarch64-glibc-stable-final/bin/aarch64-buildroot-linux-gnu-`
        - `export UEFI_STMM_PATH=/path/to/Jetson_Linux_R35.4.1_aarch64/Linux_for_Tegra/bootloader/standalonemm_optee_t234.bin`
8.  Then do within `Linux_for_Tegra/sources/tegra/optee-src/nv-optee` run `./optee_src_build.sh -p t234`
9.  Then continue this from __Building the OPTEE dtb__ on down in this [link](https://developer.ridgerun.com/wiki/index.php/Implementation_of_a_Trusted_Application_using_OP-TEE_and_JetPack5.1)

## Usage 
This is how I would compile the app from __my__ file structure. Yours may be a little different at the top depending on where you placed everything. I put it in `/opt`.

1. Copy the development folder into:
`cp -r optee_logger /opt/Jetson_LinuxR35.4.1_aarch64/Linux_for_Tegra/sources/tegra/optee-src/nv-optee/optee/samples/`
2. Move into the folder where `optee_src_build.sh` is:
`cd /opt/Jetson_LinuxR35.4.1_aarch64/Linux_for_Tegra/sources/tegra/optee-src/nv-optee/`
3. Run the build:
`./optee_src_build.sh -p t234`
4. Find the executable files here:
`cd /opt/Jetson_LinuxR35.4.1_aarch64/Linux_for_Tegra/sources/tegra/optee-src/nv-optee/optee/install/t234/usr/sbin`
5. Find the ta files here:
`cd /opt/Jetson_LinuxR35.4.1_aarch64/Linux_for_Tegra/sources/tegra/optee-src/nv-optee/optee/build/t234/ta/optee_logger`
6. Transfer all those files into the Jetson Board

In our case, our Jetson board is structured where we place the specific files here...
- Place the executable in `/usr/sbin`
- Place the ta files in `/lib/optee_armtz5`

Then run the executable with `sudo ./optee_logger`

## Resources
OP-TEE Docs:
- optee_examples: https://github.com/linaro-swg/optee_examples
- optee_build: https://github.com/OP-TEE/build 
- optee-docs: https://optee.readthedocs.io/en/latest/general/about.html

Other Help:
- https://wiki.st.com/stm32mpu/wiki/How_to_develop_an_OP-TEE_Trusted_Application
- https://developer.ridgerun.com/wiki/index.php/Implementation_of_a_Trusted_Application_using_OP-TEE_and_JetPack5.1