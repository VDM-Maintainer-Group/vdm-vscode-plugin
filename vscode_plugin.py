#!/usr/bin/env python3
import os, psutil
import time
from pathlib import Path
from pyvdm.interface import CapabilityLibrary, SRC_API

BLACKLIST = ['.git', '/.config/', '/.local/', '/.vscode']
validate = lambda x: ( True not in [_ in x for _ in BLACKLIST] )

class VscodePlugin(SRC_API):
    @staticmethod
    def _gather_record(raw_result):
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

    @staticmethod
    def _dump_record(fh, record):
        for item in record.values():
            if len(item)==1:
                _path = item[0]
                if _path: fh.write(_path+'\n') #fh.write( 'file %s'%_path )
            else:
                try:
                    _path = os.path.commonpath(item)
                    if _path=='/usr': raise Exception() #dirty hack
                except:
                    _path = ''
                if _path: fh.write(_path+'\n') #hf.write( 'workspace %s'%_path )
                pass
            pass
        pass

    def onStart(self):
        self.il = CapabilityLibrary.CapabilityHandleLocal('inotify-lookup')
        self.il.register('code')
        return 0

    def onStop(self):
        self.il.unregister('code')
        return 0

    def onSave(self, stat_file):
        # dump raw_result via il
        raw_result = self.il.dump('code')
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
            pass
        time.sleep(1.5)
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
    raw_result = _plugin.il.dump('code')
    # gathering record from raw_result
    record = _plugin._gather_record(raw_result)
    print( json.dumps(record, indent=4) )
    # write to file
    _plugin._dump_record(sys.stdout, record)
    pass
