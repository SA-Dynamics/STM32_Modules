#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import re
import shutil
from urllib.parse import to_bytes
import subprocess
import argparse
import glob

# 获取固件版本信息
def get_version(project_name):
    version_list = [0] * 3
    info_count = 0
    try:
        # 本示例中/Application/Version.h为存放版本信息的文件, MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION为版本号数字
        with open(project_name + "/Application/Version.h", 'r', encoding='utf-8') as version_file:
            file_lines = version_file.readlines()
            for line in file_lines:
                info_list = re.sub(r'\s+', ' ', line).strip().split(' ')
                if len(info_list) == 3:
                    if 'MAJOR_VERSION' in info_list and info_list.index('MAJOR_VERSION') == 1:
                        version_list[0] = info_list[2]
                        info_count += 1
                    elif 'MINOR_VERSION' in info_list and info_list.index('MINOR_VERSION') == 1:
                        version_list[1] = info_list[2]
                        info_count += 1
                    elif 'PATCH_VERSION' in info_list and info_list.index('PATCH_VERSION') == 1:
                        version_list[2] = info_list[2]
                        info_count += 1
            return info_count == 3, version_list

    except FileNotFoundError:
        print(f"the file " + project_name +  "/Application/Version.h was not found.")
    except Exception as e:
        print(f"an error occurred: {e}")


# 创建烧录文件
def create_program_file(project_name, file_path, mcu_type):
    try:
        if mcu_type == 'STM32F4':
            # 创建一个默认大小的bin文件, F4系列为512Kb
            program_file_size = 1024 * 512
            default_bytes = bytes([0xFF] * program_file_size)
        elif mcu_type == 'STM32F1':
            # 创建一个默认大小的bin文件, F1系列为248Kb
            program_file_size = 1024 * 248
            default_bytes = bytes([0xFF] * program_file_size)
        else:
            print("invalid mcu type")
            return

        with open(file_path, 'wb') as program_file:
            program_file.write(default_bytes)

        with open(file_path, 'rb') as program_file:
            program_file_content = bytearray(program_file.read())

        # 打开并读取bootloader文件, 将该内容写入到创建的bin文件中, 偏移量为0
        with open(project_name + "/MDK-ARM/" + project_name + "/Bootloader.bin", 'rb') as bootloader_file:
            bootloader_content = bootloader_file.read()
        bootloader_file.close()
        program_file_content[0 : len(bootloader_content)] = bootloader_content

        # 打开并读取app1文件, 将该内容写入到创建的bin文件中, F4偏移量为128Kb, F1为48Kb
        with open(project_name + "/MDK-ARM/" + project_name + "/" + project_name + "1.bin", 'rb') as default_app:
            default_app_content = default_app.read()
        default_app.close()
        if mcu_type == 'STM32F4':
            program_file_content[0x20000: 0x20000 + len(default_app_content)] = default_app_content
        elif mcu_type == 'STM32F1':
            program_file_content[0xC000: 0xC000 + len(default_app_content)] = default_app_content

        # 将合并的bin文件写回到原文件中
        with open(file_path, 'wb') as program_file:
            program_file.write(program_file_content)

        program_file.close()
        print("\033[32m" + project_name + ":the program file has been created successfully\033[0m")

    except FileNotFoundError:
        print("one or both of the files do not exist.")
    except PermissionError:
        print("permission denied: Unable to read from the source file or write to the destination file.")
    except Exception as e:
        print(f"an error occurred while inserting the content: {e}")


# 创建更新包文件
def create_update_file(project_name, file_path, file_version, device_index):
    try:
        # 打开并读取app1文件, 将该内容写入到创建的bin文件中, 偏移量为128
        with open(project_name + "/MDK-ARM/" + project_name + "/" + project_name + "1.bin", 'rb') as app1_file:
            app1_file_content = app1_file.read()
        app1_file.close()

        # 打开并读取app2文件, 将该内容写入到创建的bin文件中, 偏移量为128+app1
        with open(project_name + "/MDK-ARM/" + project_name + "/" + project_name + "2.bin", 'rb') as app2_file:
            app2_file_content = app2_file.read()
        app2_file.close()

        # 创建更新文件, 大小为 128 + app1 + app2
        program_file_size = 128 + len(app1_file_content) + len(app2_file_content)
        default_bytes = bytes([0x00] * program_file_size)
        with open(file_path, 'wb') as update_file:
            update_file.write(default_bytes)

        with open(file_path, 'rb') as update_file:
            update_file_content = bytearray(update_file.read())

        # 写入更新包信息
        update_file_content[0] = device_index
        update_file_content[1 : 4] = len(app1_file_content).to_bytes(4, "little")
        update_file_content[5 : 8] = len(app2_file_content).to_bytes(4, "little")
        update_file_content[9] = file_version[0]
        update_file_content[10] = file_version[1]
        update_file_content[11] = file_version[2]


        # 写入固件信息
        update_file_content[128 : 128 + len(app1_file_content)] = app1_file_content
        update_file_content[128 + len(app1_file_content) : 128 + len(app1_file_content) + len(app2_file_content)] = app2_file_content

        # 将合并的bin文件写回到原文件中
        with open(file_path, 'wb') as update_file:
            update_file.write(update_file_content)

        update_file.close()
        print("\033[32m" + project_name + ":the update file has been created successfully\033[0m")

    except FileNotFoundError:
        print("one or both of the files do not exist.")
    except PermissionError:
        print("permission denied: Unable to read from the source file or write to the destination file.")
    except Exception as e:
        print(f"An error occurred while inserting the content: {e}")




# 使用MDK命令编译出项目原始的bin文件
def compile_firmware(project_name):
    params_list = [
        ['"Bootloader"', 'build_bootloader_output.txt'],
        ['"' + project_name + '1"', 'build_app1_output.txt'],
        ['"' + project_name + '2"', 'build_app2_output.txt']
    ]

    try:
        # 编译3个固件, 打印编译日志和结果
        for i in range(len(params_list)):
            project_file = './' + project_name + '/MDK-ARM/' + project_name + '.uvprojx'
            log_file_path = './' + project_name + '/MDK-ARM/' + params_list[i][1]
            compile_command = 'UV4 -cr ' + project_file + ' -jO -t ' + params_list[i][0] + ' -o ' + params_list[i][1]
            print("\033[33mexecuting command:" + compile_command + "\033[0m")
            cmd_result = subprocess.run(compile_command, shell=True, capture_output=True, text=True)
            print("result:", cmd_result)

            with open(log_file_path, 'r', encoding='utf-8') as log_file:
                log_content = log_file.read()
            print(log_content)

            if cmd_result.returncode > 1:
                return False

        return True
    except subprocess.CalledProcessError as e:
        print("command failed with return code:", e.returncode)
        print("error output:", e.stderr)



def main():
    parser = argparse.ArgumentParser(description="generate firmware")
    parser.add_argument("output_path", help="output file path")
    args = parser.parse_args()

    # 创建输出文件目录
    release_path = args.output_path
    if not release_path:
        print("invalid path")
    else:
        if os.path.exists(release_path) and os.path.isdir(release_path):
            #shutil.rmtree(release_path)
            bin_files = glob.glob(os.path.join(release_path, f'*.bin'))
            for file in bin_files:
                os.remove(file)
            bin_files = glob.glob(os.path.join(release_path, f'*.pack'))
            for file in bin_files:
                os.remove(file)
            xml_files = glob.glob(os.path.join(release_path, f'*.xml'))
            for file in xml_files:
                os.remove(file)
        else:
            os.mkdir(release_path)

    # 需要编译的工程参数: 工程名, 平台, 硬件索引
    project_list = [
        ['TestBoard1', 'STM32F4', 1],
        ['TestBoard2', 'STM32F1', 4],
        ['TestBoard3', 'STM32F1', 3],
        ['TestBoard4', 'STM32F4', 2],
        ['TestBoard5', 'STM32F4', 5]
    ]

    for i in range(len(project_list)):
        # 编译文件
        compile_result = compile_firmware(project_list[i][0])
        if False == compile_result:
            print("\033[31mcompilation error\033[0m")
            return

        ret, version_list = get_version(project_list[i][0])

        if ret:
            # 创建烧录文件
            program_file_path = release_path + '/' + project_list[i][0] + '_V' + version_list[0] + '.' + version_list[1] + '.' + \
                                version_list[2] + '.bin'
            create_program_file(project_list[i][0], program_file_path, project_list[i][1])

            # 创建更新文件
            update_file_path = release_path + '/' + project_list[i][0] + '_V' + version_list[0] + '.' + version_list[1] + '.' + \
                                version_list[2] + '.pack'
            create_update_file(project_list[i][0], update_file_path, [int(item) for item in version_list], project_list[i][2])

        else:
            print("get version info failed")



if __name__ == "__main__":
    main()
