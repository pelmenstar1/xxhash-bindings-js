import subprocess
import os
import os.path
import shutil

import multiprocessing
from termcolor import colored
import enum

NODE_TEST_VERSIONS = [
  '18.20.5',
  '19.9.0',
  '20.18.1',
  '21.7.3',
  '22.12.0',
  '23.5.0',
]

SCRIPT_DIR_PATH = os.path.realpath(os.path.dirname(__file__))
NATIVE_DIR_PATH = os.path.join(SCRIPT_DIR_PATH, "../native/")
TEMP_DIR = os.path.join(SCRIPT_DIR_PATH, '.cross-testing')
PACKAGES_DIR = os.path.join(SCRIPT_DIR_PATH, "../packages/")

REQUIRED_PACKAGES = ['@types', 'allnative', 'minimum', 'tests']

class Stage(enum.Enum):
  INSTALL_NODE = 0
  COPY = 1  
  INSTALL_PACKAGES = 2
  NATIVE_BUILD = 3
  TESTS = 4
  FINISHED = 5

STAGE_DESCRIPTIONS: dict[Stage, str] = {
  Stage.INSTALL_NODE: "Installing node",
  Stage.COPY: "Copying artifacts",
  Stage.INSTALL_PACKAGES: "Installing packages",
  Stage.NATIVE_BUILD: "Building native code",
  Stage.TESTS: "Running tests",
  Stage.FINISHED: "Success",
}

def update_stage(ver: str, value: Stage):
   print(f"{colored(ver, 'green')}: {STAGE_DESCRIPTIONS[value]}")

def run_silent(args: list[str], cwd: str | None = None, shell: bool = False):
  proc = subprocess.run(
    args, 
    stdout=subprocess.PIPE, 
    stderr=subprocess.PIPE, 
    cwd=cwd, 
    shell=shell,
    universal_newlines=True,
    encoding='utf8')

  if proc.returncode != 0:
    print(proc.stdout)
    print(proc.stderr)
    exit(-1)

def get_current_node_version() -> str:
  proc = subprocess.run(['node', '--version'], stdout=subprocess.PIPE, universal_newlines=True)
  proc.check_returncode()

  # strip last \n symbol
  return proc.stdout[:-1]

def nvm(args: list[str]) -> None:
  run_silent([shutil.which('nvm')] + args, shell=True)

def install_node(version: str) -> None:
  nvm(['install', version])

def use_node(version: str) -> None:
  nvm(['use', version])

def copy_artifacts(version: str) -> None:
  def ignore(_1, _2):
     return ['build', 'dist']

  dest_dir = os.path.join(TEMP_DIR, version)

  for package_name in REQUIRED_PACKAGES:
    shutil.copytree(
       os.path.join(PACKAGES_DIR, package_name), 
       os.path.join(dest_dir, "packages", package_name), 
       ignore=ignore,
       dirs_exist_ok=True)
    
  shutil.copytree(
    NATIVE_DIR_PATH, 
    os.path.join(dest_dir, "native"), 
    dirs_exist_ok=True)

  shutil.copy(
    os.path.join(PACKAGES_DIR, "tsconfig.base.json"),
    os.path.join(dest_dir, "packages"))

  shutil.copy(
    os.path.join(SCRIPT_DIR_PATH, '../package.cross.json'),
    os.path.join(dest_dir, 'package.json')
  )

  # Create empty yarn.lock file to make yarn work.
  # It marks that this project is a separate one and not is part of global project.
  with open(os.path.join(dest_dir, 'yarn.lock'), 'w') as f:
     pass
  
def yarn(args: list[str], cwd: str, shell: bool = False) -> None:
  run_silent([shutil.which('yarn')] + args, cwd=cwd, shell=shell)

def corepack_enable(cwd: str) -> None:
  run_silent([shutil.which('corepack'), 'enable'], cwd=cwd)

def build_native(cwd: str) -> None:
  yarn(['node-gyp', 'configure'], cwd=cwd, shell=True)
  yarn(['node-gyp', 'build', '-j', str(multiprocessing.cpu_count())], cwd=cwd, shell=True)

def run_tests_for_node(version: str) -> None:
  ver_dir = os.path.join(TEMP_DIR, version)

  update_stage(version, Stage.INSTALL_NODE)
  install_node(version)
  use_node(version)

  update_stage(version, Stage.COPY)
  copy_artifacts(version)

  update_stage(version, Stage.INSTALL_PACKAGES)
  corepack_enable(ver_dir)
  yarn(['install'], ver_dir)

  update_stage(version, Stage.NATIVE_BUILD)
  build_native(os.path.join(ver_dir, 'packages', 'allnative'))
  build_native(os.path.join(ver_dir, 'packages', 'minimum'))
  
  update_stage(version, Stage.TESTS)
  yarn(['test'], ver_dir)

  update_stage(version, Stage.FINISHED)

def cleanup():
  shutil.rmtree(TEMP_DIR)

def main():
  node_ver = get_current_node_version()

  try:
    for version in NODE_TEST_VERSIONS:
      run_tests_for_node(version)

    print(colored("Success", 'green'))
    cleanup()
  finally:
    use_node(node_ver)

if __name__ == '__main__':
  main()