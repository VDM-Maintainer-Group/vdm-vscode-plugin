#!/usr/bin/env python3
import os, psutil
from pathlib import Path
from pyvdm.interface import SRC_API
import inotify_lookup as il

BLACKLIST = ['.git', '/.config/', '/.local/', '/.vscode']
validate = lambda x: ( True not in [_ in x for _ in BLACKLIST] )

class VscodePlugin(SRC_API):
    def fix_path(self, _fake:str) -> str:
        mount_points = filter( lambda x:x.fstype=='ext4', psutil.disk_partitions() )
        mount_points = [x.mountpoint for x in mount_points]
        for _root in mount_points:
            _tmp = Path(_root+_fake)
            try:
                if _tmp.exists():
                    return _tmp.as_posix()
            except:
                pass
            pass
        return ''

    def _gather_record(self, raw_result):
        record = dict()
        for item in raw_result:
            pid, path = item.split(',', maxsplit=1)
            path = path.rstrip('\u0000')
            if path and validate(path):
                if pid not in record.keys():
                    record[pid] = [path]
                else:
                    record[pid].append(path)
            pass
        return record

    def _dump_record(self, fh, record):
        for item in record.values():
            if len(item)==1:
                _path = item[0]
                _path = self.fix_path(_path)
                if _path: fh.write(_path) #fh.write( 'file %s'%_path )
            else:
                try:
                    _path = os.path.commonpath(item)
                    _path = self.fix_path(_path)
                except:
                    _path = ''
                if _path: fh.write(_path) #hf.write( 'workspace %s'%_path )
                pass
            pass
        pass

    def onStart(self):
        il.register('code')
        return 0

    def onStop(self):
        il.unregister('code')
        return 0

    def onSave(self, stat_file):
        # dump raw_result via il
        raw_result = il.dump('code')
        # gathering record from raw_result
        record = self._gather_record(raw_result)
        # write to file
        with open(stat_file, 'w') as f:
            self._dump_record(f, record)
            pass
        return 0

    def onResume(self, stat_file):
        with open(stat_file, 'r') as f:
            items = f.readlines()
            for piece in items:
                os.system( 'code --new-window %s'%piece )
        return 0

    def onClose(self):
        os.system('killall code')
        return 0
    pass

if __name__ == '__main__':
    import sys, json
    _plugin = VscodePlugin()
    _plugin.onStart()

    # dump raw_result via il
    raw_result = il.dump('code')
    # gathering record from raw_result
    record = _plugin._gather_record(raw_result)
    print( json.dumps(record, indent=4) )
    # write to file
    _plugin._dump_record(sys.stdout, record)
    pass
