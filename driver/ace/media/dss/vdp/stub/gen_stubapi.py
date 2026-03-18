#!/usr/bin/env python
#coding:utf-8

# Copyright 2021 Huawei Technologies Co., Ltd
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

""" Tool for auto generate stub .cc from .h """

import os
import re
import sys

PATTERN_FUNCTION = re.compile(r'ACL_FUNC_VISIBILITY\s+\n+.+\w+\([^();]*\);|.+\w+\([^();]*\);')
PATTERN_RETURN = re.compile(r'([^ ]+[ *])\w+\([^;]+;')

RETURN_STATEMENTS = {
    'hi_s32':               '    return 0;',
    'hi_u32':               '    return 0;',
    'hi_void*':             '    return HI_NULL;',
    'hi_void':              '',
}

MODULE_PARAM = {
    'tde':        ['hi_mpi_tde'],
}

def collect_header_files(path, module_name):
    dvpp_headers = []
    param_list = MODULE_PARAM[module_name]

    for root, dirs, files in os.walk(path):
        for file in files:
            for param in param_list:
                if file.find(param) >= 0:
                    file_path = os.path.join(root, file)
                    file_path = file_path.replace('\\', '/')
                    dvpp_headers.append(file_path)

    return dvpp_headers

def collect_functions(file_path):
    signatures = []
    with open(file_path) as f:
        content = f.read()
        matches = PATTERN_FUNCTION.findall(content)
        for signature in matches:
            if signature.strip().startswith('//'):
                continue
            signatures.append(signature)
    return signatures


def implement_function(func):
    # remove ';'
    function_def = func[:len(func) - 1]
    function_def += '\n'
    function_def += '{\n'
    m = PATTERN_RETURN.search(func)
    if m:
        ret_type = m.group(1).strip()
        if RETURN_STATEMENTS.__contains__(ret_type):
            function_def += RETURN_STATEMENTS[ret_type]
        else:
            if ret_type.endswith('*'):
                function_def += '    return nullptr;'
            else:
                print("Unhandled return type: " + ret_type)
    else:
        function_def += '    return nullptr;'
    function_def += '\n'
    function_def += '}\n'
    return function_def


def generate_stub_file(inc_dir, module_name):
    dvpp_header_files = collect_header_files(inc_dir, module_name)
    print("header files has been generated")
    dvpp_content = generate_function(dvpp_header_files, inc_dir)
    print("dvpp_content has been generate")
    return dvpp_content

def generate_function(header_files, inc_dir):
    includes = []
    # generate includes
    includes.append('#include "hi_mpi_tde.h"\n')

    content= includes
    print("include concent build success")
    total = 0
    content.append('\n')
    # generate implement
    for header in header_files:
        if header.find("git") >= 0:
            continue
        if not header.endswith('.h'):
            continue
        content.append("// stub for {}{}".format(header[len(inc_dir):], '\n'))
        functions = collect_functions(header)
        print("inc file:{}, functions numbers:{}".format(header, len(functions)))
        total += len(functions)
        for func in functions:
            content.append("{}{}".format(implement_function(func), '\n'))
            content.append("\n")
    print("implement concent build success")
    print('total functions number is {}'.format(total))
    return content

def gen_code(inc_dir, dvpp_stub_path, module_name):
    if not inc_dir.endswith('/'):
        inc_dir += '/'
    dvpp_content = generate_stub_file(inc_dir, module_name)
    print("dvpp_content have been generated")
    with open(dvpp_stub_path, mode='w') as f:
        f.writelines(dvpp_content)

if __name__ == '__main__':
    inc_dir = sys.argv[1]
    dvpp_stub_path = sys.argv[2]
    module_name = sys.argv[3]
    gen_code(inc_dir, dvpp_stub_path, module_name)
