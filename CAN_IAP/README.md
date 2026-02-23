# CAN IAP
    基于CAN总线的在线更新或回退程序功能, 无需烧录器

## FirmwareBuild.py
    固件打包程序, 命令行执行 py FirmwareBuild.py /自定义编译目录, 即可生成目录下所有工程的固件文件, 烧录使用的文件为.bin格式(裸板必须通过烧录器烧录一次), 更新包为.pack格式, 可通过上位机软件在线更新

## Bootloader.c
    MCU上电后第一个执行的程序, 可用于更新或跳转固件, 同时对应用程序进行检查, 严重故障时可停留在此程序内, 进行应用程序的重新升级

## CAN_IAP.c
    IAP主程序, 用于接受总线数据, 并写入FLASH中

## flash_if.c
    用于将固件数据安全写入到FLASH中

## UpdateProcess.c
    用于将固件数据安全写入到FLASH中

## FirmwareUpdateTool
    上位机固件更新Demo