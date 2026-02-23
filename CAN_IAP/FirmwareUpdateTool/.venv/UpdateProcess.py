from ctypes import c_uint16

import os.path
import CAN_DeviceBase
from CAN_DeviceBase import CAN_OBJ
import struct
import threading
import time
from enum import Enum, auto
from setuptools.command.easy_install import auto_chmod


SCAN_ALL_DEVICE = 0x18000000
UPDATE_FIRMWARE_REQUEST_BASE = 0x18100000
PACKAGE_INFO_BASE = 0x18200000
FIRMWARE_DATA_BASE = 0x18300000

SCAN_RESPOND_BASE = 0x18FF0000
UPDATE_RESPOND_BASE = 0x18FF1000

EEPROM_READ_BASE = 0x18A00000
EEPROM_WRITE_BASE = 0x18B00000


EEPROM_READ_RESPOND_BASE = 0x18FFA000
EEPROM_WRITE_RESPOND_BASE = 0x18FFB000


# 更新应答
RESPOND_UPDATE = 1
UPDATE_FLASH1_ALLOW = 1
UPDATE_FLASH2_ALLOW = 2
UPDATE_NOT_ALLOW = 3

# 固件包应答
RESPOND_PACK_INFO = 2
READY_FOR_DATA = 1
RECV_1PACK_SUCCESS = 2
RECV_ALL_PACK_SUCCESS = 3
RECV_TIMEOUT = 4
WRITE_FLASH_FAILED = 5
CHECKSUM_FAILED = 6
EEPROM_WRITE_FAILED = 0x10
EEPROM_READ_FAILED = 0x11
EEPROM_CHECK_FAILED = 0x12

# 回退应答
RESPOND_ROLLBACK = 3
CANNOT_ROLLBACK = 1
READY_ROLLBACK = 2


# EEPROM操作应答
RESPOND_EEPROM_READ_DATA_HANDLE = 4
RESPOND_EEPROM_WRITE_DATA_HANDLE = 5
EEPROM_WRITE_DATA_OK = 1
EEPROM_WRITE_DATA_FAILED = 2


class RecvEvent(Enum):
    EVENT_NONE = 0
    EVENT_UPDATE_ALLOW_1ST = 1
    EVENT_UPDATE_ALLOW_2ND = 2
    EVENT_UPDATE_NOT_ALLOW = 3
    EVENT_READY_FOR_DATA = 4
    EVENT_RECV_1PACK_SUCCESS = 5
    EVENT_RECV_ALL_PACK_SUCCESS = 6
    EVENT_RECV_TIMEOUT = 7
    EVENT_WRITE_FLASH_FAILED = 8
    EVENT_CHECKSUM_FAILED = 9
    EVENT_EEPROM_WRITE_FAILED = 10
    EVENT_EEPROM_READ_FAILED = 11
    EVNET_EEPROM_CHECK_FAILED = 12
    EVENT_CANNOT_ROLLBACK = 13
    EVENT_READY_ROLLBACK = 14
    EVENT_EEPROM_DATA_READ_OK = 15
    EVENT_EEPROM_DATA_READ_FAILED = 16
    EVENT_EEPROM_DATA_WRITE_OK = 17
    EVENT_EEPROM_DATA_WRITE_FAILED = 18


class FirmwareHandler:
    def __init__(self):
        self.__device_index = 0
        self.__firmware_correct = False
        self.__firmware_data_dict = {1 : [], 2 : []}
        self.__firmware1_size = 0
        self.__firmware2_size = 0

    def get_current_firmware_status(self):
        return self.__firmware_correct, self.__device_index


    def __check_device_index_correct(self, index):
        if index > 0 and index < 6:
            return True
        else:
            return False

    def __check_firmware_correct(self, start_data):
        #print(start_data)
        if start_data & 0x00000020 == 0x00000020:
            return True

    def set_current_firmware(self, data_list):
        self.__firmware_correct = False
        self.__device_index = 0
        self.__firmware_data_dict[1].clear()
        self.__firmware_data_dict[2].clear()
        self.__firmware1_size = 0
        self.__firmware2_size = 0

        # 获取并判断固件类型
        data_tuple = data_list[0]
        if self.__check_device_index_correct(data_tuple[0]):
            self.__device_index = data_tuple[0]
        else:
            return False, "文件错误:硬件类型错误"

        # 获取两个固件的大小
        firmware1_size = ((data_tuple[4] << 24) | (data_tuple[3] << 16) |
                          (data_tuple[2] << 8) | data_tuple[1])
        firmware2_size = (data_tuple[7] << 16) | (data_tuple[6] << 8) | data_tuple[5]
        data_tuple = data_list[1]   # 高字节在下一个list元素中
        firmware2_size = (data_tuple[0] << 24) | firmware2_size

        version_string = str(data_tuple[1]) + "." + str(data_tuple[2]) + "." + str(data_tuple[3])

        data_tuple = data_list[128 // 8]
        firmware1_start_data = ((data_tuple[0] << 24) | (data_tuple[1] << 16) |
                                (data_tuple[2] << 8) | data_tuple[3])
        data_tuple = data_list[(firmware1_size + 128) // 8]
        firmware2_start_data = ((data_tuple[0] << 24) | (data_tuple[1] << 16) |
                                (data_tuple[2] << 8) | data_tuple[3])
        if (self.__check_firmware_correct(firmware1_start_data) and
            self.__check_firmware_correct(firmware2_start_data)):
            self.__firmware1_size = firmware1_size
            self.__firmware2_size = firmware2_size
        else:
            return False, "文件错误:固件数据错误"

        #print(self.__firmware1_size, self.__firmware2_size)

        pack_num = -1
        data_offset = 128 // 8
        # 写入固件数据
        for i in range(self.__firmware1_size // 8):
            if i % 64 == 0:
                data_pack = []
                self.__firmware_data_dict[1].append(data_pack)
                pack_num += 1

            self.__firmware_data_dict[1][pack_num].append(data_list[i + data_offset])
        #print(self.__firmware_data_dict[1])

        pack_num = -1
        data_offset = (firmware1_size + 128) // 8
        for i in range(self.__firmware2_size // 8):
            if i % 64 == 0:
                data_pack = []
                self.__firmware_data_dict[2].append(data_pack)
                pack_num += 1
            self.__firmware_data_dict[2][pack_num].append(data_list[i + data_offset])
        #print(self.__firmware_data_dict[2])
        self.__firmware_correct = True
        return True, "固件文件载入成功,版本为:" + version_string, self.__device_index


    def get_firmware_pack_size(self, firmware_index):
        if firmware_index == 1 or firmware_index == 2:
            return len(self.__firmware_data_dict[firmware_index])
        else:
            return 0

    def get_firmware_pack_info(self, firmware_index, pack_index):
        data_tuple_size = 0
        check_sum = c_uint16(0)
        if firmware_index == 1 or firmware_index == 2:
            data_tuple_size = len(self.__firmware_data_dict[firmware_index][pack_index])
            for i in range(data_tuple_size):
                for j in range(8):
                    check_sum.value = check_sum.value + self.__firmware_data_dict[firmware_index][pack_index][i][j]
                    #print(check_sum)
            check_sum.value = ~check_sum.value + 1

        return data_tuple_size, check_sum.value, self.__firmware_data_dict[firmware_index][pack_index]

#FIRMWARE_OK = 0x0

class UpdateProcessHandler(FirmwareHandler):
    def __init__(self):
        super().__init__()
        self.__device_dict = {1: "xx1", 2: "xx2", 3: "xx3", 4: "xx4", 5: "xx5"}
        self.__respond_event_dict = {RESPOND_UPDATE : {UPDATE_FLASH1_ALLOW : RecvEvent.EVENT_UPDATE_ALLOW_1ST,
                                                       UPDATE_FLASH2_ALLOW : RecvEvent.EVENT_UPDATE_ALLOW_2ND,
                                                       UPDATE_NOT_ALLOW : RecvEvent.EVENT_UPDATE_NOT_ALLOW},
                                     RESPOND_PACK_INFO : {READY_FOR_DATA : RecvEvent.EVENT_READY_FOR_DATA,
                                                          RECV_1PACK_SUCCESS: RecvEvent.EVENT_RECV_1PACK_SUCCESS,
                                                          RECV_ALL_PACK_SUCCESS : RecvEvent.EVENT_RECV_ALL_PACK_SUCCESS,
                                                          RECV_TIMEOUT : RecvEvent.EVENT_RECV_TIMEOUT,
                                                          WRITE_FLASH_FAILED : RecvEvent.EVENT_WRITE_FLASH_FAILED,
                                                          CHECKSUM_FAILED : RecvEvent.EVENT_CHECKSUM_FAILED,
                                                          EEPROM_WRITE_FAILED : RecvEvent.EVENT_EEPROM_WRITE_FAILED,
                                                          EEPROM_READ_FAILED : RecvEvent.EVENT_EEPROM_READ_FAILED,
                                                          EEPROM_CHECK_FAILED : RecvEvent.EVNET_EEPROM_CHECK_FAILED},
                                     RESPOND_ROLLBACK : {CANNOT_ROLLBACK : RecvEvent.EVENT_CANNOT_ROLLBACK,
                                                         READY_ROLLBACK : RecvEvent.EVENT_READY_ROLLBACK,
                                                         EEPROM_WRITE_FAILED: RecvEvent.EVENT_EEPROM_WRITE_FAILED,
                                                         EEPROM_READ_FAILED: RecvEvent.EVENT_EEPROM_READ_FAILED,
                                                         EEPROM_CHECK_FAILED: RecvEvent.EVNET_EEPROM_CHECK_FAILED},
                                     RESPOND_EEPROM_READ_DATA_HANDLE : RecvEvent.EVENT_EEPROM_DATA_READ_OK,
                                     RESPOND_EEPROM_WRITE_DATA_HANDLE : {EEPROM_WRITE_DATA_OK : RecvEvent.EVENT_EEPROM_DATA_WRITE_OK,
                                                                         EEPROM_WRITE_DATA_FAILED : RecvEvent.EVENT_EEPROM_WRITE_FAILED}}

        self.__firmware_flag = False
        self.__continue_scan = True
        self.__recv_event_list = []

        # 猴子补丁替换原有方法
        CAN_DeviceBase.read_handler = self.recv_msg_handler

    def __get_msg_event(self):
        event_msg = RecvEvent.EVENT_NONE
        if len(self.__recv_event_list):
            event_msg = self.__recv_event_list.pop(0)
        return event_msg

    def connect_handler(self, connect_result, connect_info):
        print(connect_info + ":", connect_result)

    def __connect_comm_tool_thread(self):
        CAN_DeviceBase.can_close()
        ret, connect_info = CAN_DeviceBase.can_init()
        self.connect_handler(ret, connect_info)

    def connect_comm_tool(self):
        connect_thread = threading.Thread(target=self.__connect_comm_tool_thread)
        connect_thread.start()


    def disconnect_comm_tool(self):
        CAN_DeviceBase.can_close()


    def __get_device_type(self, device_index):
        return self.__device_dict[device_index]

    def __get_device_index(self, device_name):
        device_index = 0
        for key, val in self.__device_dict.items():
            if val == device_name:
                device_index = key
                break
        return device_index

    def device_info_print(self, msg):
        print(msg)

    def process_msg_print(self, msg):
        print(msg)

    def update_result_handler(self, msg, result):
        print(msg)

    def update_progress_disp(self, progress):
        print(progress)

    def scan_device_handler(self, msg):
        str_device = self.__get_device_type(msg[0].ID & 0xF)
        str_app_type = "未知"
        if 0x1 == msg[0].data[0]:
            str_app_type = "bootloader"
        elif 0x2 == msg[0].data[0]:
            str_app_type = "app"
        str_version = str(msg[0].data[1]) + "." + str(msg[0].data[2]) + "." + str(msg[0].data[3])
        str_flash_area = msg[0].data[4]
        device_data = [str_device, str_app_type, str_version, str_flash_area]
        self.device_info_print(device_data)


    def update_device_handler(self, msg):
        event_list = self.__respond_event_dict[msg[0].data[0]]
        self.__recv_event_list.append(event_list[msg[0].data[1]])

    def eeprom_read_handler(self, msg):
        self.__recv_event_list.append(self.__respond_event_dict[RESPOND_EEPROM_READ_DATA_HANDLE])


    def eeprom_write_handler(self, msg):
        event_list = self.__respond_event_dict[RESPOND_EEPROM_WRITE_DATA_HANDLE]
        self.__recv_event_list.append(event_list[msg[0].data[0]])


    def recv_msg_handler(self, msg):
        if 1 == msg[0].ExternFlag:
            if SCAN_RESPOND_BASE == msg[0].ID & 0xFFFFF000:
                self.scan_device_handler(msg)
            elif UPDATE_RESPOND_BASE == msg[0].ID & 0xFFFFF000:
                self.update_device_handler(msg)
            elif EEPROM_WRITE_RESPOND_BASE == msg[0].ID & 0xFFFFB000:
                self.eeprom_write_handler(msg)
            elif EEPROM_READ_RESPOND_BASE == msg[0].ID & 0xFFFFA000:
                read_data = "EEPROM数据:"
                for i in range(msg[0].DataLen):
                    read_data  = read_data + str(msg[0].data[i]) + " "
                self.process_msg_print(read_data)
                self.eeprom_read_handler(msg)





    def scan_all_device(self):
        can_obj = CAN_OBJ()
        can_obj.ID = SCAN_ALL_DEVICE
        can_obj.DataLen = 2
        can_obj.data[0] = 0
        can_obj.data[1] = 0
        can_obj.RemoteFlag = 0
        can_obj.ExternFlag = 1
        CAN_DeviceBase.can_send(can_obj)


    def __scan_specified_device_thread(self, device_index, force_flag):
        for i in range(10000):
            can_obj = CAN_OBJ()
            can_obj.ID = SCAN_ALL_DEVICE
            can_obj.DataLen = 2
            can_obj.data[0] = device_index
            can_obj.data[1] = 0
            if (force_flag):
                can_obj.data[1] = 1
            can_obj.RemoteFlag = 0
            can_obj.ExternFlag = 1

            if not self.__continue_scan:
                break

            CAN_DeviceBase.can_send(can_obj)

            if False == force_flag:
                break
            else:
                time.sleep(0.5)


    def scan_specified_device(self, scan_flag, device_name, force_flag):
        device_index = self.__get_device_index(device_name)
        if scan_flag:
            self.__continue_scan = True
        else:
            self.__continue_scan = False
        connect_thread = threading.Thread(target=self.__scan_specified_device_thread, args=(device_index, force_flag))
        connect_thread.start()



    def load_firmware(self, file_path):
        firmware_list = []
        firmware_file = open(file_path, 'rb')
        firmware_file_size = os.path.getsize(file_path)

        for i in range(firmware_file_size // 8):
            firmware_data = firmware_file.read(8)
            firmware_data_tuple = struct.unpack("BBBBBBBB", firmware_data)
            firmware_list.append(firmware_data_tuple)

        # 载入并校验固件
        self.__firmware_flag = False
        self.__firmware_flag, msg, device_index = self.set_current_firmware(firmware_list)
        self.process_msg_print(self.__device_dict[device_index] + msg)
        firmware_file.close()
        return self.__firmware_flag


    def __send_update_request_msg(self, device_index, pack1_size, pack2_size):
        can_obj = CAN_OBJ()
        can_obj.ID = UPDATE_FIRMWARE_REQUEST_BASE + device_index
        can_obj.DataLen = 5
        can_obj.data[0] = 0x01
        can_obj.data[1] = (pack1_size >> 8) & 0xFF
        can_obj.data[2] = pack1_size & 0xFF
        can_obj.data[3] = (pack2_size >> 8) & 0xFF
        can_obj.data[4] = pack2_size & 0xFF
        can_obj.RemoteFlag = 0
        can_obj.ExternFlag = 1
        CAN_DeviceBase.can_send(can_obj)


    def __send_data_package_info(self, device_index, pack_index, data_len, check_sum):
        can_obj = CAN_OBJ()
        can_obj.ID = PACKAGE_INFO_BASE + device_index
        can_obj.DataLen = 6
        can_obj.data[0] = (pack_index >> 8) & 0xFF
        can_obj.data[1] = pack_index & 0xFF
        can_obj.data[2] = (data_len >> 8) & 0xFF
        can_obj.data[3] = data_len & 0xFF
        can_obj.data[4] = (check_sum >> 8) & 0xFF
        can_obj.data[5] = check_sum & 0xFF
        can_obj.RemoteFlag = 0
        can_obj.ExternFlag = 1
        CAN_DeviceBase.can_send(can_obj)

    def __send_firmware_data(self, device_index, data_list):
        can_obj = CAN_OBJ()
        can_obj.ID = FIRMWARE_DATA_BASE + device_index
        can_obj.DataLen = 8
        for i in range(can_obj.DataLen):
            can_obj.data[i] = data_list[i]
        can_obj.RemoteFlag = 0
        can_obj.ExternFlag = 1
        CAN_DeviceBase.can_send(can_obj)


    def __update_firmware_process(self, device_index):
        self.update_progress_disp(0)
        firmware_flag, firmware_device = self.get_current_firmware_status()
        if not firmware_flag:
            self.update_result_handler("未加载正确的固件")
            return
        if firmware_device != device_index:
            self.update_result_handler("固件与当前选择硬件不匹配")
            return

        timeout_count = 0
        firmware_index = 0
        device_name = self.__get_device_type(device_index)
        self.process_msg_print("向" + device_name + "发送更新请求")

        # 发送更新请求, 并等待下位机回应
        self.__send_update_request_msg(device_index, self.get_firmware_pack_size(1),
                                       self.get_firmware_pack_size(2))
        print(self.get_firmware_pack_size(1), self.get_firmware_pack_size(2))
        while True:
            event_recv = self.__get_msg_event()
            if (event_recv == RecvEvent.EVENT_UPDATE_ALLOW_1ST or
                    event_recv == RecvEvent.EVENT_UPDATE_ALLOW_2ND):
                self.process_msg_print(device_name + "可更新固件")
                if event_recv == RecvEvent.EVENT_UPDATE_ALLOW_1ST:
                    firmware_index = 1
                else:
                    firmware_index = 2
                break
            elif event_recv == RecvEvent.EVENT_UPDATE_NOT_ALLOW:
                self.update_result_handler(device_name + "正忙, 不可更新固件")
                return
            else:
                if timeout_count > 100:
                    self.update_result_handler("接收" + device_name + "消息超时")
                    return
                time.sleep(0.1)
                timeout_count += 1

        self.process_msg_print("可更新固件区域为" + str(firmware_index))
        package_num = self.get_firmware_pack_size(firmware_index)

        self.process_msg_print("共有" + str(package_num) + "个数据包")
        timeout_count = 0
        for i in range(package_num):
            self.process_msg_print("向" + device_name + "发送第" + str(i + 1) + "个固件包信息")

            # 获取指定固件包的数据帧个数, 校验, 数据
            tuple_num, check_sum, pack_data = self.get_firmware_pack_info(firmware_index, i)

            # 发送更新请求, 并等待下位机回应
            self.__send_data_package_info(device_index, i, tuple_num, check_sum)
            while True:
                event_recv = self.__get_msg_event()
                if event_recv == RecvEvent.EVENT_READY_FOR_DATA:
                    self.process_msg_print(device_name + "准备接收第" + str(i + 1) + "个固件包数据")
                    break
                else:
                    if timeout_count > 100:
                        self.update_result_handler("接收" + device_name + "确认固件包信息消息超时")
                        return
                    time.sleep(0.1)
                    timeout_count += 1

            self.process_msg_print("向" + device_name + "发送第" + str(i + 1) + "个固件包数据")
            for j in range(tuple_num):
                self.__send_firmware_data(device_index, pack_data[j])
                time.sleep(0.01)

            timeout_count = 0
            while True:
                event_recv = self.__get_msg_event()
                if event_recv == RecvEvent.EVENT_RECV_1PACK_SUCCESS:
                    self.process_msg_print(device_name + "接收第" + str(i + 1) + "个固件包数据成功")
                    self.update_progress_disp((i + 1) * 100 / package_num)
                    break
                elif event_recv == RecvEvent.EVENT_WRITE_FLASH_FAILED:
                    self.update_result_handler("写FLASH失败")
                    return
                elif event_recv == RecvEvent.EVENT_CHECKSUM_FAILED:
                    self.update_result_handler("数据校验有误")
                    return
                else:
                    if timeout_count > 100:
                        self.update_result_handler("接收" + device_name + "确认固件包数据消息超时")
                        return
                time.sleep(0.1)
                timeout_count += 1

        self.process_msg_print("向" + device_name + "发送完所有固件数据")
        timeout_count = -1
        while True:
            event_recv = self.__get_msg_event()
            if event_recv == RecvEvent.EVENT_RECV_ALL_PACK_SUCCESS:
                extern_info = ""
                if 1 == device_index:
                    extern_info = ",关闭工控机后应用更新"
                self.update_result_handler(device_name + "固件更新完成" + extern_info, True)
                break
            elif event_recv == RecvEvent.EVENT_EEPROM_WRITE_FAILED:
                self.update_result_handler(device_name + "写EEPROM失败", False)
                break
            elif event_recv == RecvEvent.EVENT_EEPROM_READ_FAILED:
                self.update_result_handler(device_name + "读EEPROM失败", False)
                break
            elif event_recv == RecvEvent.EVNET_EEPROM_CHECK_FAILED:
                self.update_result_handler(device_name + "EEPROM数据校验错误", False)
                break
            else:
                if timeout_count > 100:
                    self.update_result_handler("接收" + device_name + "固件更新完成消息超时", False)
                    break
            time.sleep(0.1)
            timeout_count += 1


    def update_firmware(self, device_name):
        device_index = self.__get_device_index(device_name)
        update_thread = threading.Thread(target=self.__update_firmware_process, args=(device_index,))
        update_thread.start()


    def __send_rollback_request_msg(self, device_index):
        can_obj = CAN_OBJ()
        can_obj.ID = UPDATE_FIRMWARE_REQUEST_BASE + device_index
        can_obj.DataLen = 1
        can_obj.data[0] = 0x2
        can_obj.RemoteFlag = 0
        can_obj.ExternFlag = 1
        CAN_DeviceBase.can_send(can_obj)


    def __rollback_firmware_process(self, device_index):
        self.__send_rollback_request_msg(device_index)
        timeout_count = -1
        while True:
            event_recv = self.__get_msg_event()
            if event_recv == RecvEvent.EVENT_CANNOT_ROLLBACK:
                self.process_msg_print("不能回退固件")
                break
            elif event_recv == RecvEvent.EVENT_READY_ROLLBACK:
                self.process_msg_print("准备回退固件,稍后重新扫描")
                break
            elif event_recv == RecvEvent.EVENT_EEPROM_WRITE_FAILED:
                self.process_msg_print("写EEPROM失败")
                break
            elif event_recv == RecvEvent.EVENT_EEPROM_READ_FAILED:
                self.process_msg_print("读EEPROM失败")
                break
            elif event_recv == RecvEvent.EVNET_EEPROM_CHECK_FAILED:
                self.process_msg_print("EEPROM数据校验错误")
                break
            else:
                if timeout_count > 50:
                    self.process_msg_print("接收固件回退消息超时")
                    break
            time.sleep(0.1)
            timeout_count += 1


    def rollback_firmware(self, device_name):
        device_index = self.__get_device_index(device_name)
        rollback_thread = threading.Thread(target=self.__rollback_firmware_process, args=(device_index,))
        rollback_thread.start()

    def __send_read_eeprom_msg(self, device_index, read_address, byte_num):
        can_obj = CAN_OBJ()
        can_obj.ID = EEPROM_READ_BASE + device_index
        can_obj.DataLen = 3
        can_obj.data[0] = (read_address >> 8) & 0xFF
        can_obj.data[1] = read_address & 0xFF
        can_obj.data[2] = byte_num
        can_obj.RemoteFlag = 0
        can_obj.ExternFlag = 1
        CAN_DeviceBase.can_send(can_obj)

    def __read_eeprom_process(self, device_index, read_address, byte_num):
        if byte_num > 8:
            byte_num = 8
        self.__send_read_eeprom_msg(device_index, read_address, byte_num)

        timeout_count = 0
        while True:
            event_recv = self.__get_msg_event()
            if event_recv == RecvEvent.EVENT_EEPROM_DATA_READ_OK:
                self.process_msg_print("读取EEPROM数据成功")
                break
            elif event_recv == RecvEvent.EVENT_EEPROM_DATA_READ_FAILED:
                self.process_msg_print("读取EEPROM数据失败")
                break
            else:
                if timeout_count > 50:
                    self.process_msg_print("读取EEPROM数据超时")
                    break
            time.sleep(0.1)
            timeout_count += 1


    def read_eeprom(self, device_name, read_address, byte_num):
        if byte_num <= 0:
            return
        device_index = self.__get_device_index(device_name)
        read_eeprom_thread = threading.Thread(target=self.__read_eeprom_process, args=(device_index, read_address, byte_num,))
        read_eeprom_thread.start()


    def __send_write_eeprom_msg(self, device_index, write_address, byte_value):
        can_obj = CAN_OBJ()
        can_obj.ID = EEPROM_WRITE_BASE + device_index
        can_obj.DataLen = 3
        can_obj.data[0] = (write_address >> 8) & 0xFF
        can_obj.data[1] = write_address & 0xFF
        can_obj.data[2] = byte_value
        can_obj.RemoteFlag = 0
        can_obj.ExternFlag = 1
        CAN_DeviceBase.can_send(can_obj)


    def __write_eeprom_process(self, device_index, write_address, byte_value):
        self.__send_write_eeprom_msg(device_index, write_address, byte_value)

        timeout_count = 0
        while True:
            event_recv = self.__get_msg_event()
            if event_recv == RecvEvent.EVENT_EEPROM_DATA_WRITE_OK:
                self.process_msg_print("写入EEPROM数据成功")
                break
            elif event_recv == RecvEvent.EVENT_EEPROM_DATA_WRITE_FAILED:
                self.process_msg_print("写入EEPROM数据失败")
                break
            else:
                if timeout_count > 50:
                    self.process_msg_print("写入EEPROM数据超时")
                    break
            time.sleep(0.1)
            timeout_count += 1


    def write_eeprom(self, device_name, write_address, byte_value):
        device_index = self.__get_device_index(device_name)
        write_eeprom_thread = threading.Thread(target=self.__write_eeprom_process, args=(device_index, write_address, byte_value,))
        write_eeprom_thread.start()

