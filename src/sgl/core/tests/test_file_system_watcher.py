# SPDX-License-Identifier: Apache-2.0

import pytest
from sgl import FileSystemWatcherChange, FileSystemWatchEvent, FileSystemWatchDesc, FileSystemWatcher
import time
import os

def test_desc_bindings():
    watcher = FileSystemWatchDesc()
    watcher.directory = "hello"
    assert str(watcher.directory) == "hello"

def test_ev_bindings():
    watcher = FileSystemWatchEvent()
    assert watcher.change == FileSystemWatcherChange.invalid

def test_start_stop_watch_descriptor():
    watcher = FileSystemWatcher()
    desc = FileSystemWatchDesc()
    desc.directory = os.path.dirname(__file__)
    desc.recursive = True
    watcher.add_watch(desc)
    assert watcher.watch_count == 1
    watcher.remove_watch(desc.directory)
    assert watcher.watch_count == 0

def test_start_stop_watch_args():
    watcher = FileSystemWatcher()
    dir = os.path.dirname(__file__)
    watcher.add_watch(dir)
    assert watcher.watch_count == 1
    watcher.remove_watch(dir)
    assert watcher.watch_count == 0



if __name__ == "__main__":
    pytest.main([__file__, "-v"])
