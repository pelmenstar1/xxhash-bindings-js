import subprocess
import os.path
import shutil
import stat

SCRIPT_DIR_PATH = os.path.realpath(os.path.dirname(__file__))
NATIVE_DIR_PATH = os.path.join(SCRIPT_DIR_PATH, '..', 'native')
TEMP_DIR_PATH = os.path.join(SCRIPT_DIR_PATH, ".temp") 

def shutil_onerror(func, path, exc_info):
  # Is the error an access error?
  if not os.access(path, os.W_OK):
    os.chmod(path, stat.S_IWUSR)
    func(path)
  else:
      raise

def clone_repo():
  if os.path.exists(TEMP_DIR_PATH):
    shutil.rmtree(TEMP_DIR_PATH, onerror=shutil_onerror)
  
  os.mkdir(TEMP_DIR_PATH)

  result = subprocess.run(
    ['git', 'clone', '--depth', '1', 'https://github.com/Cyan4973/xxHash/'], 
    shell=True,
    cwd=TEMP_DIR_PATH
  )

  result.check_returncode()
  
def copy_main_file():
  shutil.copyfile(
    os.path.join(TEMP_DIR_PATH, "xxHash", "xxhash.h"),
    os.path.join(NATIVE_DIR_PATH, "xxhash.h")
  )

def update_latest_commit_file():
  result = subprocess.run(
    ['git', 'rev-parse', 'HEAD'],
    shell=True,
    cwd=os.path.join(TEMP_DIR_PATH, 'xxHash'),
    stdout=subprocess.PIPE
  )
  result.check_returncode()
  
  with open(os.path.join(NATIVE_DIR_PATH, 'xxhash-commit.txt'), mode='wb') as f:
    output = result.stdout
    output = output.removesuffix(b'\n')

    f.write(output)

def cleanup():
  shutil.rmtree(TEMP_DIR_PATH, onerror=shutil_onerror)

if __name__ == '__main__':
  clone_repo()
  copy_main_file()
  update_latest_commit_file()
  cleanup()