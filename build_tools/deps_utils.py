#type: ignore

import json, hashlib, pathlib, shutil, platform

from utils import *
import sys

components = []
rebuild_control = {}
previous_hashes = {}
hash_path = join_path(build_cache_dir, "hashes.json")
if os.path.exists(hash_path):
    with open(hash_path, "r") as file:
        previous_hashes = json.load(file)

def hash_component(name):
    build_tools_dir = join_path(cwd, "build_tools")
    data = ""
    data += platform.system()

    #with open(join_path(build_tools_dir, "main.py"), "r", encoding="utf-8") as file:
        #data += file.read()

    with open(join_path(build_tools_dir, "deps_build_cmds.py"), "r", encoding="utf-8") as file:
        data += file.read()

    with open(join_path(cmake_dir, "dependencies", name, "CMakeLists.txt"), "r", encoding="utf-8") as file:
        data += file.read()

    return hashlib.sha256(data.encode('utf-8')).hexdigest()

def merge_subdir(src_subdir, dest_root):
    try:
        dest_subdir = dest_root / src_subdir.name
        dest_subdir.mkdir(parents=True, exist_ok=True)

        for item in src_subdir.rglob('*'):
            relative_path = item.relative_to(src_subdir)
            dest_item = dest_subdir / relative_path

            if item.is_dir():
                dest_item.mkdir(parents=True, exist_ok=True)
            else:
                dest_item.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(item, dest_item)

        ts_print(f"Merged '{src_subdir}' -> '{dest_subdir}'")
    except Exception as error:
        ts_print(f"Error merging '{src_subdir}': {error}")
        Context.abort = True
        sys.exit(-1)

def merge_all_subdirs(src_root, dest_root):
    src_root = pathlib.Path(src_root)
    dest_root = pathlib.Path(dest_root)

    if not src_root.exists():
        ts_print(f"Source directory '{src_root}' does not exist.")
        return

    threads = []

    for src_subdir in src_root.iterdir():
        if not src_subdir.is_dir():
            continue  # Skip files at the root level

        thread = CustomThreading(target=merge_subdir, args=(src_subdir, dest_root))
        thread.start()
        threads.append(thread)

    for thread in threads:
        thread.join()

def selectively_clean_extern():
    if os.path.exists(extern_dir):
        def should_keep(path):
            return ('STB' in path or
                    'shader_build_tools' in path or 'nlohmann' in path)

        for dirpath, dirnames, filenames in os.walk(extern_dir, topdown=False):
            full_dirpath = os.path.abspath(dirpath)
            # Remove files not in keep paths
            for filename in filenames:
                full_path = join_path(full_dirpath, filename)
                if not should_keep(full_path):
                    os.remove(full_path)

            # Remove directories if they're empty and not part of keep paths
            for dirname in dirnames:
                full_subdir = join_path(full_dirpath, dirname)
                if not should_keep(full_subdir) and not os.listdir(full_subdir):
                    os.rmdir(full_subdir)

        # Optionally remove directories that became empty after cleaning
        if not should_keep(extern_dir) and not os.listdir(extern_dir):
            os.rmdir(extern_dir)

def selective_removal(directory, keep_items):
    # Use a list as a queue to process directories iteratively
    dirs_to_process = [directory]

    while dirs_to_process:
        current_dir = dirs_to_process.pop(0)

        for item in os.listdir(current_dir):
            item_path = join_path(current_dir, item)

            if os.path.isfile(item_path):
                # Check if the file should be deleted
                if not any(item.endswith(keep) for keep in keep_items):
                    os.remove(item_path)

            elif os.path.isdir(item_path):
                # Add subdirectory to the queue for later processing
                if not item_path.endswith(".mypy_cache"):
                    dirs_to_process.append(item_path)

        # After processing, try to remove empty directories
        for item in os.listdir(current_dir):
            item_path = join_path(current_dir, item)
            if os.path.isdir(item_path) and not os.listdir(item_path):
                try:
                    os.rmdir(item_path)
                except Exception as e:
                    ts_print(f"Could not delete directory: '{item_path}': {e}")