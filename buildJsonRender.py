from distutils.command.build import build
from logging import root
import os, sys, platform
import shutil
import urllib.request
import py7zr
root_path = os.path.dirname(os.path.abspath(__file__))
output_path = os.path.join(root_path, "output")
'''
try:
    shutil.rmtree(output_path)
except:
    pass
'''

try:
    os.mkdir(output_path)
except:
    pass

libclang_path = os.path.join(root_path, "vendors")
sys_type = platform.system()
print("{} building start.....".format(sys_type))
sys.stdout.flush()
libclang_remote_path = ""
if "Linux" == sys_type:
    libclang_remote_path = "https://download.qt.io/development_releases/prebuilt/libclang/libclang-release_130-based-linux-Ubuntu20.04-gcc9.3-x86_64.7z"
elif "Windows" == sys_type:
    libclang_remote_path = "https://download.qt.io/development_releases/prebuilt/libclang/libclang-release_130-based-windows-vs2019_64.7z"
elif "Darwin" == sys_type:
    libclang_remote_path = "https://download.qt.io/development_releases/prebuilt/libclang/libclang-release_130-based-macos-universal.7z"

if not os.path.exists(os.path.join(libclang_path, "libclang.7z")):
    print("{} downloading clang start.....".format(libclang_remote_path))
    sys.stdout.flush()
    urllib.request.urlretrieve(libclang_remote_path, os.path.join(libclang_path, "libclang.7z"))
os.chdir(output_path)
with py7zr.SevenZipFile(os.path.join(libclang_path, "libclang.7z"), 'r') as archive:
    print("{} unzip clang start.....".format(os.path.join(libclang_path, "libclang.7z")))
    sys.stdout.flush()
    archive.extractall(path=libclang_path)
print("go to {} and cmake start.....".format(output_path))
sys.stdout.flush()
os.chdir(output_path)

build_type = "Debug"
if len(sys.argv) > 1:
    build_type = sys.argv[1]
os.system("cmake -DAST_DUMP_JSON=ON -DCMAKE_BUILD_TYPE={} -GNinja ../".format(build_type))
os.system("ninja")