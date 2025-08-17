import os
import sys
import logging
import argparse
import subprocess

logging.basicConfig(
    level=logging.DEBUG,
    format="%(asctime)s [%(levelname)s] %(message)s",
    datefmt="%H:%M:%S"
)

logging.info("Start of KVS project build")

parser = argparse.ArgumentParser()

parser.add_argument('build_type', help='CMake build type: Release or Debug')
parser.add_argument('-b', '--build_tests', action='store_true')
parser.add_argument('-l', '--enable_logs', action='store_true')

logging.info("Parsing passed arguments")

args = parser.parse_args()

build_type = args.build_type
build_tests = args.build_tests
enable_logs = args.enable_logs

if build_type != 'Release' and build_type != 'Debug':
    logging.error("The build_type argument was passed incorrectly, it can only take the values Release or Debug")
    sys.exit(1)

if enable_logs == None:
    logging.warning("The enable_logs argument is not passed, default value will be used: False")
    enable_logs = False

if build_tests == None:
    logging.warning("The build_tests argument is not passed, default value will be used: False")
    build_tests = False

project_dir = os.path.abspath(os.path.dirname(__file__))
build_dir = os.path.join(project_dir, f"kvs_{build_type.lower()}")

logging.info("Creating a build directory")
os.makedirs(build_dir, exist_ok=True)

toolchain_file = os.path.join(build_dir, "build", "generators", "conan_toolchain.cmake")

logging.info("Installing dependencies via Conan")
subprocess.check_call([
    "conan", "install", project_dir,
    "-of", build_dir,
    "--build=missing",
    f"-o build_tests={build_tests}",
    f"-o enable_logs={enable_logs}"
])
logging.info("Dependencies installed successfully")

logging.info("Generating a CMake project")
subprocess.check_call([
    "cmake",
    "-S", project_dir,
    "-B", build_dir,
    "-G", "Visual Studio 17 2022",
    "-A", "x86_64",
    f"-DCMAKE_TOOLCHAIN_FILE={toolchain_file}",
    f"-DCMAKE_BUILD_TYPE={build_type}"
])
logging.info("CMake project generated successfully")

logging.info("Build project")
subprocess.check_call([
    "cmake",
    "--build", build_dir,
    "--config", build_type
])
logging.info("Project build completed successfully")