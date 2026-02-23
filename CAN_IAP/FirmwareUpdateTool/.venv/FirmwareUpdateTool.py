import time
from UpdateProcess import UpdateProcessHandler
from tkinter.ttk import Treeview
import tkinter.ttk
from tkinter import *
from tkinter.tix import *
from tkinter import ttk
from tkinter import messagebox
from tkinter.messagebox import *
from tkinter import filedialog
import UpdateProcess
import re

# 设置窗口
main_window = Tk()
main_window.title("固件更新工具V0.1.0")
main_window.resizable(0, 0)


class UpdateHandler(UpdateProcessHandler):
    # 添加或更新硬件信息显示
    def device_info_print(self, msg):
        if device_list.exists(msg[0]):
            device_list.item(msg[0], values=msg)
        else:
            device_list.insert('', END, iid=msg[0], values=msg)
        return

    def process_msg_print(self, msg):
        text_disp.insert(END, msg + "\r\n")
        text_disp.see(END)
        return

    def connect_handler(self, connect_result, connect_info):
        if connect_result == True:
            # 使能按键
            btn_scan_all_device.config(state=NORMAL)
            btn_scan_specified_device.config(state=NORMAL)
        text_disp.insert(END, connect_info + "\r\n")
        text_disp.see(END)

    def update_progress_disp(self, progress):
        progress_var.set(progress)
        progress_value_diap.config(text=("%"+"{:.1f}".format(progress)))

    def update_result_handler(self, msg, result):
        self.process_msg_print(msg)
        btn_update_firmware.config(state=NORMAL)
        btn_open_file.config(state=NORMAL)
        btn_rollback_firmware.config(state=NORMAL)
        btn_connect_debugger.config(state=NORMAL)
        btn_scan_all_device.config(state=NORMAL)
        btn_scan_specified_device.config(state=NORMAL)
        btn_write_eeprom.config(state=NORMAL)
        btn_read_eeprom.config(state=NORMAL)

update_handler = UpdateHandler()

# 处理适配器连接事务
def on_connect_debugger(connect_result, connect_info):
    if connect_result == True:
        # 使能按键
        btn_scan_all_device.config(state=NORMAL)
        btn_scan_specified_device.config(state=NORMAL)
    text_disp.insert(END, connect_info)
    text_disp.see(END)
    return


def on_connect_debugger_click():
    update_handler.connect_comm_tool()


def on_window_closing():
    if messagebox.askokcancel("Quit", "确定退出吗?"):
        update_handler.disconnect_comm_tool()
        main_window.destroy()


def on_scan_all_device_click():
    device_items = device_list.get_children()

    for item in device_items:
        device_list.delete(item)
    update_handler.scan_all_device()
    return

_btn_scan = True
_force_boot_var = BooleanVar()
_current_device_index = 0

def scan_specified_device_handler():
    global _btn_scan
    update_handler.scan_specified_device(_btn_scan, device_enum.get(), _force_boot_var.get())
    if _force_boot_var.get():
        if _btn_scan:
            btn_scan_specified_device.config(text="停止扫描")
            btn_scan_specified_device.grid(row=3, column=1, ipadx=12, padx=20, pady=5, sticky='w')
            check_button.config(state=DISABLED)
        else:
            btn_scan_specified_device.config(text="扫描指定硬件")
            btn_scan_specified_device.grid(row=3, column=1, ipadx=0, padx=20, pady=5, sticky='w')
            check_button.config(state=NORMAL)
        _btn_scan = not _btn_scan

def on_scan_specified_device_click():
    device_items = device_list.get_children()
    for item in device_items:
        device_list.delete(item)
    scan_specified_device_handler()


def on_open_file():
    file_path = filedialog.askopenfilename()
    if file_path:
        open_state = update_handler.load_firmware(file_path)
        if open_state:
            btn_update_firmware.config(state=NORMAL)


def on_update_firmware():
    if not _btn_scan:
        # 如果有正在运行扫描线程就先停掉
        scan_specified_device_handler()
    time.sleep(0.1)
    selected_item = device_list.focus()
    if selected_item:
        values = device_list.item(selected_item, "values")
        update_handler.update_firmware(values[0])
        btn_update_firmware.config(state=DISABLED)
        btn_open_file.config(state=DISABLED)
        btn_rollback_firmware.config(state=DISABLED)
        btn_connect_debugger.config(state=DISABLED)
        btn_scan_all_device.config(state=DISABLED)
        btn_scan_specified_device.config(state=DISABLED)
        btn_write_eeprom.config(state=DISABLED)
        btn_read_eeprom.config(state=DISABLED)


def on_rollback_firmware():
    if not _btn_scan:
        # 如果有正在运行扫描线程就先停掉
        scan_specified_device_handler()
    time.sleep(0.1)
    selected_item = device_list.focus()
    if selected_item:
        values = device_list.item(selected_item, "values")
        update_handler.rollback_firmware(values[0])

def on_device_select(event=None):
    current_text = device_enum.get()


def on_read_eeprom():
    selected_item = device_list.focus()
    if selected_item:
        update_handler.read_eeprom(device_list.focus(), int(read_address_spin.get()), int(read_num_spin.get()))

def on_write_eeprom():
    selected_item = device_list.focus()
    if selected_item:
        update_handler.write_eeprom(device_list.focus(), int(write_address_spin.get()), int(write_data_spin.get()))


# 连接适配器按钮
btn_connect_debugger = Button(main_window, text="连接适配器", command=on_connect_debugger_click)
btn_connect_debugger.grid(row=1, column=1 ,ipadx=6, padx=20, pady=5, sticky='w')

# 扫描所有硬件按钮
btn_scan_all_device = Button(main_window, text="扫描所有硬件", command=on_scan_all_device_click)
btn_scan_all_device.grid(row=2, column=1 ,padx=20, pady=5, sticky='w')
btn_scan_all_device.config(state=DISABLED)

# 扫描制定硬件按钮
btn_scan_specified_device = Button(main_window, text="扫描指定硬件", command=on_scan_specified_device_click)
btn_scan_specified_device.grid(row=3, column=1, padx=20, pady=5, sticky='w')
btn_scan_specified_device.config(state=DISABLED)


# 硬件信息列表
column_type = ["硬件", "当前运行固件", "版本", "FLASH区"]
device_list = Treeview(main_window, columns=column_type, show='headings')
device_list.column("硬件", width=60)
device_list.column("当前运行固件", width=90)
device_list.column("版本", width=80)
device_list.column("FLASH区", width=70)
device_list.heading("硬件", text="硬件")
device_list.heading("当前运行固件", text="当前运行固件")
device_list.heading("版本", text="版本")
device_list.heading("FLASH区", text="FLASH区")
device_list.grid(row=5, column=1, padx=20, pady=10, columnspan=6, sticky='w')



device_enum = ttk.Combobox(main_window, values=["xx1", "xx2", "xx3", "xx4", "xx5"])
device_enum.configure(width=4)
device_enum.grid(row=3, column=2, columnspan=1, sticky='w')
device_enum.bind("<<ComboboxSelected>>", on_device_select)

check_button = Checkbutton(main_window, text="bootloader禁止跳转", variable=_force_boot_var)
check_button.grid(row=3, column=3, columnspan=2, sticky='w')


# 打开固件按钮
btn_open_file = Button(main_window, text="打开固件文件", command=on_open_file)
btn_open_file.grid(row=1, column=6 ,padx=20, pady=5, sticky='w')


# 更新固件按钮
btn_update_firmware = Button(main_window, text="更新固件", command=on_update_firmware)
btn_update_firmware.grid(row=2, column=6, ipadx=12 ,padx=20, pady=5, sticky='w')
btn_update_firmware.config(state=DISABLED)

# 回退固件按钮
btn_rollback_firmware = Button(main_window, text="回退固件", command=on_rollback_firmware)
btn_rollback_firmware.grid(row=3, column=6, ipadx=12 ,padx=20, pady=5, sticky='w')


# 创建 Text 控件
text_disp = Text(main_window, height=17, width=50)
text_disp.grid(row=5, column=6, padx=20, pady=10, columnspan=7, sticky='w')
vertical_scroll = Scrollbar(main_window, command=text_disp.yview)
text_disp.configure(yscrollcommand=vertical_scroll.set)
#vertical_scroll.pack(side=RIGHT, fill=Y)


# 读取EEPROM按钮
btn_read_eeprom = Button(main_window, text="读EEPROM", command=on_read_eeprom)
btn_read_eeprom.grid(row=6, column=1, ipadx=8 ,padx=20, pady=5, sticky='w')

Label(main_window, width=8,text="读取数量:").grid(row=6, column=2, pady=5, sticky='w')
read_num_spin = Spinbox(main_window, from_=1, to=8, width=2,wrap=True)
read_num_spin.grid(row=6, column=3, pady=5, sticky='w')

Label(main_window, width=8,text="读取地址:").grid(row=7, column=2, pady=5, sticky='w')
read_address_spin = Spinbox(main_window, from_=0, to=4294967295, width=10,wrap=True)
read_address_spin.grid(row=7, column=3, pady=5, sticky='w')


# 写EEPROM按钮
btn_write_eeprom = Button(main_window, text="写EEPROM", command=on_write_eeprom)
btn_write_eeprom.grid(row=8, column=1, ipadx=8 ,padx=20, pady=5, sticky='w')

Label(main_window, width=8,text="写入地址:").grid(row=8, column=2, pady=5, sticky='w')
write_address_spin = Spinbox(main_window, from_=0, to=4294967295, width=10,wrap=True)
write_address_spin.grid(row=8, column=3, pady=5, sticky='w')

Label(main_window, width=8,text="写入数据:").grid(row=9, column=2, pady=5, sticky='w')
write_data_spin = Spinbox(main_window, from_=0, to=255, width=3,wrap=True)
write_data_spin.grid(row=9, column=3, pady=2, sticky='w')




progress_var = DoubleVar(value=0.0)
update_progress = ttk.Progressbar(main_window, length=200,variable=progress_var)
update_progress.grid(row=2, column=7, columnspan=4, sticky='w')


progress_value_diap = Label(main_window, text="%0")
progress_value_diap.grid(row=2, column=11)


main_window.protocol("WM_DELETE_WINDOW", on_window_closing)
main_window.mainloop()
