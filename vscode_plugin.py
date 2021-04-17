#!/usr/bin/env python3
import os
from pathlib import Path
from pyvdm.interface import SRC_API
import inotify_lookup as il

HOME_FIX  = '/'+Path.home().name
BLACKLIST = ['.git', '.config', '.vscode']
validate = lambda x: ( True not in [_ in x for _ in BLACKLIST] )

class VscodePlugin(SRC_API):
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
        _record = dict()
        for item in raw_result:
            pid, path = item.split(',', maxsplit=1)
            if path.startswith(HOME_FIX):
                path = '/home' + path
            if validate(path):
                if pid not in _record.keys():
                    _record[pid] = [path]
                else:
                    _record[pid].append(path)
            pass
        # write file
        with open(stat_file, 'w') as f:
            #TODO: could deduce workspace from `code --status` 
            for item in _record.values():
                [f.write(piece) for piece in item]
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
    pass
