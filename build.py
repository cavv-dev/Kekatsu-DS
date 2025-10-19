#!/usr/bin/env python3

import sys
import os
import shutil
from architectds.architectds import *

APP_VERSION = '1.2.0'
NDS_FILE_NAME = 'Kekatsu.nds'
RELEASE_DIR = 'release'

arm9 = Arm9Binary(
    sourcedirs=['source'],
    defines=['USE_FILE32API'],
    libs=['dswifi9', 'nds9',
          'mbedcrypto', 'mbedtls',
          'mbedx509', 'curl',
          'png', 'z'],
    libdirs=['${BLOCKSDS}/libs/dswifi', '${BLOCKSDS}/libs/libnds',
             '${BLOCKSDSEXT}/mbedtls', '${BLOCKSDSEXT}/libcurl']
)

arm9.add_grit(['source/gfx', 'source/gui/gfx'], out_dir='')
arm9.add_data(['source/lang'], out_dir='')
arm9.generate_elf()

nds = NdsRom(
    nds_path=NDS_FILE_NAME,
    binaries=[arm9],
    game_title='Kekatsu',
    game_subtitle='DS(i) content downloader',
    game_author='Cavv',
    game_icon='icon.png'
)
nds.generate_nds()
nds.run_command_line_arguments()

if 'release' in sys.argv:
    if not os.path.exists(RELEASE_DIR):
        os.mkdir(RELEASE_DIR)

    if os.path.exists(NDS_FILE_NAME):
        shutil.move(NDS_FILE_NAME, os.path.join(RELEASE_DIR, NDS_FILE_NAME))

    version_file = os.path.join(RELEASE_DIR, 'version.txt')
    with open(version_file, 'w') as f:
        f.write(APP_VERSION + '\n')
elif '--clean' in sys.argv:
    if os.path.exists(RELEASE_DIR):
        shutil.rmtree(RELEASE_DIR)
