import _ctypes
import os,sys
from ctypes import *

_cur_work_directory = os.getcwd()

class CAN_OBJ(Structure):
    _fields_ = [("ID", c_uint),  # 报文帧ID
                ("TimeStamp", c_uint),  # 接收到信息帧时的时间标识，从CAN控制器初始化开始计时，单位微秒
                ("TimeFlag", c_byte),  # 是否使用时间标识，为1时TimeStamp有效，TimeFlag和TimeStamp只在此帧为接收帧时有意义。
                ("SendType", c_byte),
                # 发送帧类型。=0时为正常发送，=1时为单次发送（不自动重发），=2时为自发自收（用于测试CAN卡是否损坏），=3时为单次自发自收（只发送一次，用于自测试），只在此帧为发送帧时有意义
                ("RemoteFlag", c_byte),  # 是否是远程帧。=0时为数据帧，=1时为远程帧
                ("ExternFlag", c_byte),  # 是否是扩展帧。=0时为标准帧（11位帧ID），=1时为扩展帧（29位帧ID）
                ("DataLen", c_byte),  # 数据长度DLC(<=8)，即Data的长度
                ("data", c_ubyte * 8),  # CAN报文的数据。空间受DataLen的约束
                ("Reserved", c_byte * 3)]  # 系统保留。

def readmess():
    global musbcanopen
    if (musbcanopen == False):
        tkinter.messagebox.showinfo("ERROR", "请先打开设备")
    else:
        mboardinfo, ret = ecan.ReadBoardInfo(USBCAN2, DevIndex)  # 读取设备信息需要在打开设备后执行
        if ret == STATUS_OK:
            mstr = ""
            for i in range(0, 10):
                mstr = mstr + chr(mboardinfo.str_Serial_Num[i])  # 结构体中str_Serial_Num内部存放存放SN号的ASC码
            #lbsn.configure(text="SN:" + mstr)
            print("SN:" + mstr)

        else:
            #lbsn.configure(text="Read info Fault")
            print("Read info Fault")

class ECAN(object):
    def __init__(self):
        self.dll = cdll.LoadLibrary(_cur_work_directory + '/ECanVci64.dll')
        if self.dll == None:
            print("DLL Couldn't be loaded")

    def OpenDevice(self, DeviceType, DeviceIndex):
        try:
            return self.dll.OpenDevice(DeviceType, DeviceIndex, 0)
        except:
            print("Exception on OpenDevice!")
            raise

    def CloseDevice(self, DeviceType, DeviceIndex):
        try:
            return self.dll.CloseDevice(DeviceType, DeviceIndex, 0)
        except:
            print("Exception on CloseDevice!")
            raise

    def InitCan(self, DeviceType, DeviceIndex, CanInd, Initconfig):
        try:
            return self.dll.InitCAN(DeviceType, DeviceIndex, CanInd, byref(Initconfig))
        except:
            print("Exception on InitCan!")
            raise

    def StartCan(self, DeviceType, DeviceIndex, CanInd):
        try:
            return self.dll.StartCAN(DeviceType, DeviceIndex, CanInd)
        except:
            print("Exception on StartCan!")
            raise

    def ReadBoardInfo(self, DeviceType, DeviceIndex):
        try:
            mboardinfo = BoardInfo()
            ret = self.dll.ReadBoardInfo(DeviceType, DeviceIndex, byref(mboardinfo))
            return mboardinfo, ret
        except:
            print("Exception on ReadBoardInfo!")
            raise

    def Receivce(self, DeviceType, DeviceIndex, CanInd, length):
        try:
            recmess = (CAN_OBJ * length)()
            ret = self.dll.Receive(DeviceType, DeviceIndex, CanInd, byref(recmess), length, 0)
            return length, recmess, ret
        except:
            print("Exception on Receive!")
            raise

    def Tramsmit(self, DeviceType, DeviceIndex, CanInd, mcanobj):
        try:
            # mCAN_OBJ=CAN_OBJ*2
            # self.dll.Transmit.argtypes = [ctypes.c_uint32, ctypes.c_uint32, ctypes.c_uint32, POINTER(CAN_OBJ),
            # ctypes.c_uint16]
            return self.dll.Transmit(DeviceType, DeviceIndex, CanInd, byref(mcanobj), c_uint16(1))
        except:
            print("Exception on Tramsmit!")
            raise