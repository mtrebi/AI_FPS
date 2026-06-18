#!/usr/bin/env python

import os
import re
import string
import sys
import xml.etree.ElementTree as ET
import shelve
import pickle
from time import *
import calendar
import fileinput
import stat

# user-dependent

# If set to True will make the changes in original source files.
# otherwise it will just print out the changes that need to be done
MAKE_CHANGES_IN_SOURCE_FILES = True

# if left unchanged with recurrently parse the current location
SRC_PATH_OS = './'

"""
Can be filled with specific subfolder names to narrow down the search. For example:
directories = ['YourGame/Source/',
               'YourGame/Config/'
               ]
If empty (default) will parse all the C++, *.Build.cs and ini files found 
"""
directories = []


##########################################
# from this point on there be dragons
##########################################

source_file = re.compile("^\w+\.(?:inl|cpp|h|ini)$", flags=re.I)
build_cs_file = re.compile(".*\.(Build.cs)$", flags=re.I)
ignore_dirs = re.compile('[\\\\/](?:Thirdparty|Intermediate)[\\\\/]*', flags=re.I)
engine_code_pattern = re.compile('[\\\\/]Engine[\\\\/]', flags=re.I)
include_pattern = re.compile("^#include\s+", flags=re.I)
script_ref_pattern = re.compile("\/Script\/Engine\.", flags=re.I)

change_patterns = [
    ('AbstractNavData', '', 'NavigationSystem'),
    ('CrowdManagerBase', '', 'NavigationSystem'),
    ('NavArea', 'NavAreas/', 'NavigationSystem'),
    ('NavAreaMeta_SwitchByAgent', 'NavAreas/', 'NavigationSystem'),
    ('NavAreaMeta', 'NavAreas/', 'NavigationSystem'),
    ('NavArea_Default', 'NavAreas/', 'NavigationSystem'),
    ('NavArea_LowHeight', 'NavAreas/', 'NavigationSystem'),
    ('NavArea_Null', 'NavAreas/', 'NavigationSystem'),
    ('NavArea_Obstacle', 'NavAreas/', 'NavigationSystem'),
    ('NavCollision', '', 'NavigationSystem'),
    ('NavigationQueryFilter', 'NavFilters/', 'NavigationSystem'),
    ('RecastFilter_UseDefaultArea', 'NavFilters/', 'NavigationSystem'),
    ('NavigationGraph', 'NavGraph/', 'NavigationSystem'),
    ('NavigationGraphNode', 'NavGraph/', 'NavigationSystem'),
    ('NavigationGraphNodeComponent', 'NavGraph/', 'NavigationSystem'),
    ('NavLinkComponent', '', 'NavigationSystem'),
    ('NavLinkCustomComponent', '', 'NavigationSystem'),
    ('NavLinkCustomInterface', '', 'NavigationSystem'),
    ('NavLinkHostInterface', '', 'NavigationSystem'),
    ('NavLinkRenderingComponent', '', 'NavigationSystem'),
    ('NavLinkRenderingProxy', '', 'NavigationSystem'),
    ('NavLinkTrivial', '', 'NavigationSystem'),
    ('NavMeshBoundsVolume', 'NavMesh/', 'NavigationSystem'),
    ('NavMeshPath', 'NavMesh/', 'NavigationSystem'),
    ('NavMeshRenderingComponent', 'NavMesh/', 'NavigationSystem'),
    ('NavTestRenderingComponent', 'NavMesh/', 'NavigationSystem'),
    ('PImplRecastNavMesh', 'NavMesh/', 'NavigationSystem'),
    ('RecastHelpers', 'NavMesh/', 'NavigationSystem'),
    ('RecastNavMesh', 'NavMesh/', 'NavigationSystem'),
    ('RecastNavMeshDataChunk', 'NavMesh/', 'NavigationSystem'),
    ('RecastNavMeshGenerator', 'NavMesh/', 'NavigationSystem'),
    ('RecastQueryFilter', 'NavMesh/', 'NavigationSystem'),
    ('NavModifierComponent', '', 'NavigationSystem'),
    ('NavModifierVolume', '', 'NavigationSystem'),
    ('NavNodeInterface', '', 'NavigationSystem'),
    ('NavRelevantComponent', '', 'NavigationSystem'),
    ('NavigationData', '', 'NavigationSystem'),
    ('NavigationInvokerComponent', '', 'NavigationSystem'),
    ('NavigationOctree', '', 'NavigationSystem'),
    ('NavigationPath', '', 'NavigationSystem'),
    ('NavigationPathGenerator', '', 'NavigationSystem'),
    ('NavigationSystem', '', 'NavigationSystem', 'NavigationSystemV1'),
    ('NavigationSystemModule', '', 'NavigationSystem'),
    ('NavigationSystemTypes', '', 'NavigationSystem'),
    ('NavigationTestingActor', '', 'NavigationSystem'),
    ('NavLinkProxy', 'Navigation/', 'AIModule'),
]

for i in range(len(change_patterns)):
    new_name_in_script = None
    if len(change_patterns[i]) == 3:
        file_name, include_dir, new_module = change_patterns[i]
    elif len(change_patterns[i]) == 4:
        file_name, include_dir, new_module, new_name_in_script = change_patterns[i]

    as_include_pattern = re.compile('\#include.*\W({}\.h)"'.format(file_name), flags=re.I)
    script_pattern = re.compile('(/Script/)Engine(\.{})'.format(file_name), flags=re.I)
    if new_name_in_script:
        script_replace = r'\1{}.{}'.format(new_module, new_name_in_script)
    else:
        script_replace = r'\1{}\2'.format(new_module)
    change_patterns[i] = as_include_pattern, include_dir, script_pattern, script_replace

game_code_patterns = [
    (re.compile('UNavigationSystem::InitializeForWorld\('), r'FNavigationSystem::AddNavigationSystemToWorld(*'),
    (re.compile('(\w+).GetNavigationSystem\(\)->'), r'FNavigationSystem::GetCurrent<UNavigationSystemV1>(&\1)->'),
    (re.compile('(\w+(\(\))?)->GetNavigationSystem\(\)->'), r'FNavigationSystem::GetCurrent<UNavigationSystemV1>(\1)->'),
    (re.compile('=\s*(\w+).GetNavigationSystem\(\)'), r'= FNavigationSystem::GetCurrent<UNavigationSystemV1>(&\1)'),
    (re.compile('=\s*(\w+(\(\))?)->GetNavigationSystem\(\)'), r'= FNavigationSystem::GetCurrent<UNavigationSystemV1>(\1)'),
    (re.compile('(\W*)UNavigationSystem(\W)'), r'\1UNavigationSystemV1\2'),
    (re.compile('(\W*)GetMainNavData([\(\W]*)'), r'\1GetDefaultNavDataInstance\2'),
    (re.compile('FNavigationSystem::ECreateIfEmpty'), r'FNavigationSystem::ECreateIfMissing'),
]

build_cs_pattern = (re.compile('(\s*)("AIModule")\s*(,?)([\s\n]*)'), r'\1"NavigationSystem",\4\1\2\3\4')
navigation_system_module_pattern = re.compile(r'"NavigationSystem"')
aimodule_pattern = re.compile(r'"AIModule"')


if directories is None or len(directories) == 0:
    # start with current dir
    directories = ['']


def chomp(line):
    line = str.split(line, '\n')[0]
    return str.split(line, '\r')[0]


def check_line(line, engine_code=True):
    # check if it's an include line
    line = chomp(line)
    if re.search(include_pattern, line):
        for pattern, replace, _, _ in change_patterns:
            match_object = re.search(pattern, line)
            if match_object:
                new_line = '#include "{}{}"'.format(replace, match_object.group(1))
                if new_line != line:
                    return line, new_line
    elif re.search(script_ref_pattern, line):
        for _, _, pattern, replace in change_patterns:
            match_object = re.search(pattern, line)
            if match_object:
                new_line = re.sub(pattern, replace, line)
                if new_line != line:
                    return line, new_line
    elif not engine_code:
        work_line = line
        something_found = False
        for pattern, replace in game_code_patterns:
            match_object = re.search(pattern, work_line)
            if match_object:
                new_line = re.sub(pattern, replace, work_line)
                if new_line != work_line:
                    something_found = True
                    work_line = new_line

        if something_found and work_line != line:
            return line, work_line

    return None


def find_changes(full_file_path):
    changes = []
    file = open(full_file_path, 'r', encoding='utf-8')
    try:
        lines = file.readlines()
    except UnicodeDecodeError as e:
        #print('Failed to parse {} ({})'.format(full_file_path, e))
        lines = []
    file.close()
    is_engine_code = (re.search(engine_code_pattern, full_file_path) is not None)
    line_number = 0
    for line in lines:
        line_number += 1
        change = check_line(line, is_engine_code)
        if change:
            changes.append((line_number, *change))
    return changes


def replace_in_file(full_file_path):
    changes = find_changes(full_file_path)
    if changes is None or len(changes) == 0:
        return []

    error_message = None
    line_number = 0
    changes = []
    with fileinput.FileInput(full_file_path, inplace=True, backup='.bak') as file:
        is_engine_code = (re.search(engine_code_pattern, full_file_path) is not None)
        try:
            for line in file:
                line_number += 1
                change = check_line(line, is_engine_code)
                if change:
                    changes.append((line_number, *change))
                    new_line = change[1]
                    line = new_line + '\n'
                print(line, end='')
        except PermissionError as e:
            error_message = 'Unable to update file due to permission error {}: {}'.format(full_file_path, e)
            pass
        except UnicodeDecodeError as e:
            error_message = 'Failed to parse {}'.format(full_file_path)
            pass
    if error_message:
        print(error_message)
    return changes


def fix_build_cs(full_file_path, apply_changes=False):
    changes = []
    is_engine_code = (re.search(engine_code_pattern, full_file_path) is not None)
    if not is_engine_code:
        file = open(full_file_path, 'r', encoding='utf-8')
        try:
            lines = file.readlines()
        except UnicodeDecodeError as e:
            print('Failed to parse {} ({})'.format(full_file_path, e))
            lines = []
        file.close()

        all_lines = '\n'.join(lines)
        change = None
        if re.search(navigation_system_module_pattern, all_lines) is None\
                and re.search(aimodule_pattern, all_lines):
            pattern, replace = build_cs_pattern
            line_number = 0
            for line in lines:
                line_number += 1
                if re.search(pattern, line) is not None:
                    new_line = re.sub(pattern, replace, line)
                    if new_line != line:
                        change = (line_number, line, new_line)
                        break;

        if change is not None and apply_changes:
            error_message = None
            with fileinput.FileInput(full_file_path, inplace=True, backup='.bak') as file:
                try:
                    line_number = 0
                    line_to_change, old_line, new_line = change
                    for line in file:
                        line_number += 1
                        if line_number == line_to_change:
                            assert (line == old_line)
                            print(new_line, end='')
                        else:
                            print(line, end='')
                except PermissionError as e:
                    error_message = 'Unable to update file due to permission error {}: {}'.format(full_file_path, e)
                    pass
                except UnicodeDecodeError as e:
                    error_message = 'Failed to parse {}'.format(full_file_path)
                    pass
            if error_message:
                print(error_message)

        if change:
            changes = [change]

    return changes


def traverse(dirName, apply_changes=False):
    action = find_changes if not apply_changes else replace_in_file
    for root, dirs, files in os.walk(dirName):
        if re.search(ignore_dirs, root):
            continue
        for f in files:
            changes = []
            if re.search(source_file, f) is not None:
                full_path = os.path.join(root, f)
                changes = action(full_path)
            elif re.search(build_cs_file, f) is not None:
                full_path = os.path.join(root, f)
                changes = fix_build_cs(full_path, apply_changes)

            if len(changes):
                print('===================================================================================\n{}\n-----------------------------------------------------------------------------------'.format(full_path))
                for line_num, line, new_line in changes:
                    print('{}: {} -> {}'.format(line_num, line, new_line))


if __name__ == '__main__':
    for dirName in directories:
        traverse(SRC_PATH_OS + dirName, apply_changes=MAKE_CHANGES_IN_SOURCE_FILES)
