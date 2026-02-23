from ECAN_Device import ECAN, CAN_OBJ
import ctypes
from ctypes import *
import os,sys
import threading

DevType = c_uint

USBCAN1 = DevType(3)
USBCAN2 = DevType(4)
USBCANFD = DevType(6)

_device_index = c_uint(0)  # 设备索引

channel1 = c_uint(0)  # CAN1
Channel2 = c_uint(1)  # CAN2

STATUS_ERR = 0
STATUS_OK = 1

class BoardInfo(Structure):
    _fields_ = [("hw_Version", c_ushort),  # 硬件版本号，用16进制表示
                ("fw_Version", c_ushort),  # 固件版本号，用16进制表示
                ("dr_Version", c_ushort),  # 驱动程序版本号，用16进制表示
                ("in_Version", c_ushort),  # 接口库版本号，用16进制表示
                ("irq_Num", c_ushort),  # 板卡所使用的中断号
                ("can_Num", c_byte),  # 表示有几路CAN通道
                ("str_Serial_Num", c_byte * 20),  # 此板卡的序列号，用ASC码表示
                ("str_hw_Type", c_byte * 40),  # 硬件类型，用ASC码表示
                ("Reserved", c_byte * 4)]  # 系统保留


class INIT_CONFIG(Structure):
    _fields_ = [("acccode", c_uint32),  # 验收码。SJA1000的帧过滤验收码
                ("accmask", c_uint32),  # 屏蔽码。SJA1000的帧过滤屏蔽码。屏蔽码推荐设置为0xFFFF FFFF，即全部接收
                ("reserved", c_uint32),  # 保留
                ("filter", c_byte),  # 滤波使能。0=不使能，1=使能。使能时，请参照SJA1000验收滤波器设置验收码和屏蔽码
                ("timing0", c_byte),  # 波特率定时器0,详见动态库使用说明书7页
                ("timing1", c_byte),  # 波特率定时器1,详见动态库使用说明书7页
                ("mode", c_byte)]  # 模式。=0为正常模式，=1为只听模式，=2为自发自收模式。


# 加载动态库
ecan = ECAN()

if hasattr(sys,'frozen'):
    os.environ['PATH'] = sys._MEIPASS + ":" + os.environ['PATH']


_usb_can_opened = False
_recv_start = True
_rec_can1 = 1


def read_handler(msg):
    global _rec_can1
    mstr = "Rec: " + str(_rec_can1)
    _rec_can1 = _rec_can1 + 1
    if msg[0].TimeFlag == 0:
        mstr = mstr + " Time: "
    else:
        mstr = mstr + " Time:" + hex(msg[0].TimeStamp).zfill(8)
    if msg[0].ExternFlag == 0:
        mstr = mstr + " ID:" + hex(msg[0].ID).zfill(3) + " Format:Stand "
    else:
        mstr = mstr + " ID:" + hex(msg[0].ID).zfill(8) + " Format:Exten "
    if msg[0].RemoteFlag == 0:
        mstr = mstr + " Type:Data " + " Data: "
        for i in range(0, msg[0].DataLen):
            mstr = mstr + hex(msg[0].data[i]).zfill(2) + " "
    else:
        mstr = mstr + " Type:Romte " + " Data: Remote Request"

    print(mstr)


def read_can():
    global _usb_can_opened, _recv_start
    if (_usb_can_opened == True):
        scount = 0
        while (_recv_start):
            scount = scount + 1
            len, rec, ret = ecan.Receivce(USBCAN2, _device_index, channel1, 1)
            #print("_recv_start:", _recv_start)
            if (len > 0 and ret == 1):
                read_handler(rec)


read_thread = threading.Timer(0.03, read_can)

def can_init():
    global _usb_can_opened, t, rec_can1, _recv_start
    if (_usb_can_opened == False):
        init_config = INIT_CONFIG()
        init_config.acccode = 0  # 设置验收码
        init_config.accmask = 0xFFFFFFFF  # 设置屏蔽码
        init_config.filter = 0  # 设置滤波使能
        can1_baud = "1M"    # 波特率

        init_info = "开始初始化调试器"
        # 打开设备
        if (ecan.OpenDevice(USBCAN2, _device_index) != STATUS_OK):
            init_info = "打开调试器失败"
            return False, init_info

        init_config.timing0, init_config.timing1 = get_timing(can1_baud)
        init_config.mode = 0

        # 初始化CAN1
        if (ecan.InitCan(USBCAN2, _device_index, channel1, init_config) != STATUS_OK):
            init_info = "初始化调试器失败"
            ecan.CloseDevice(USBCAN2, _device_index)
            return False, init_info

        if (ecan.StartCan(USBCAN2, _device_index, channel1) != STATUS_OK):
            init_info = "初始化调试器失败"
            ecan.CloseDevice(USBCAN2, _device_index)
            return False, init_info

        init_info = "调试器初始化成功"
        _usb_can_opened = True
        _recv_start = True
        rec_can1 = 1
        read_thread = threading.Timer(0.03, read_can)
        read_thread.start()

    else:
        init_info = "调试器已初始化成功"

    return True, init_info

def can_close():
    global _recv_start, _usb_can_opened
    _recv_start = False
    ecan.CloseDevice(USBCAN2, _device_index)
    _usb_can_opened = False


def get_timing(mbaud):
    if mbaud == "1M":
        return 0, 0x14
    if mbaud == "800k":
        return 0, 0x16
    if mbaud == "666k":
        return 0x80, 0xb6
    if mbaud == "500k":
        return 0, 0x1c
    if mbaud == "400k":
        return 0x80, 0xfa
    if mbaud == "250k":
        return 0x01, 0x1c
    if mbaud == "200k":
        return 0x81, 0xfa
    if mbaud == "125k":
        return 0x03, 0x1c
    if mbaud == "100k":
        return 0x04, 0x1c
    if mbaud == "80k":
        return 0x83, 0xff
    if mbaud == "50k":
        return 0x09, 0x1c


def clearcan1():
    listreadcan1.delete(0, END)


def clearcan2():
    listreadcan2.delete(0, END)


def can_send(can_obj):
    global _usb_can_opened, channel1
    if (_usb_can_opened == False):
        return
    else:
        ecan.Tramsmit(USBCAN2, _device_index, channel1, can_obj)

