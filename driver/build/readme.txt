编译使用说明：
    1.进入源码压缩包路径, 命令为:cd mini_asic_3559_it/project
    2.解压源代码source.tar.gz, 命令为: tar zxvf source.tar.gz
    3.进入源码路径, 命令为: cd source
    4.输入源码编译命令， 命令为:make CROSS_COMPILE=? KERNEL_DIR=? O=?
      ARCH:          编译的ARCH,默认为arm64
      CROSS_COMPILE: 编译gcc的绝对路径
      HOST_OS:       编译镜像运行的操作系统，目前支持CentOS、Ubuntu（默认）
      KERNEL_DIR:    编译ko需要指定kernel根目录的绝对路径，此目录需要有.config文件和include目录，用户要保证内部有.config文件
      O:             编译结果输出目录，支持绝对路径与相对路径，默认为当前路径下out文件夹


例如:

场景1 （编译3559RC形态）
make clean
make ARCH=arm64 CROSS_COMPILE=aarch64-himix100-linux- KERNEL_DIR=/usr/kernel/linux-4.9 0=../../lib/host

场景2 （编译x86-Ubuntu-PCIe形态）
make clean
make ARCH=x86 KERNEL_DIR=/usr/kernel/linux-4.4

场景3 （编译x86-CentOS-PCIe形态）
make clean
make ARCH=x86 HOST_OS=CentOS KERNEL_DIR=/usr/kernel/linux-3.10

场景4 （编译ctrl cpu device驱动）
编译驱动:
make clean
make 0=../../lib/device driver_device CROSS_COMPILE=/source/build/prebuilts/hcc/linux-x86/aarch64/aarch64-linux-gnu/bin/aarch64-linux-gnu- KERNEL_DIR=/source/kernel/linux-source KERNEL_DEFCONFIG=/source/kernel/linux-source/arch/arm64/configs/eulerosv2r9_defconfig build_device=true

