#!/usr/bin/env python3
import os, re, time, psutil
import json
from pathlib import Path
from pyvdm.interface import CapabilityLibrary, SRC_API

DBG = 0
SLOT = 0.80

BLACKLIST = ['/Code/User/settings.json', '/.vscode/settings.json', '/.git/refs/remotes/',
            '/usr/lib/', '/usr/local/lib/',
            '/lib/python',
            '/target/rls/']
VALIDATE  = lambda x: ( True not in [_ in x for _ in BLACKLIST] )

WINDOW_TITLE = '%s - Visual Studio Code'

class VscodePlugin(SRC_API):
    @staticmethod
    def _gather_records(raw_result):    
        records = list()

        ## collect and filter the raw data
        _processes = dict()
        for item in raw_result:
            pid, path = item.split(',', maxsplit=1)
            if VALIDATE(path):
                pid  = int(pid)
                path = path.rstrip('\u0000')
                #
                if pid not in _processes.keys():
                    _processes[pid] = [path]
                else:
                    _processes[pid].append(path)
            pass
        if DBG: print( json.dumps(_processes, indent=4) )

        ## gather records
        for pid, items in _processes.items():
            if not psutil.pid_exists(pid): #maybe workspace
                _path = os.path.commonpath(items)
                records.append( _path )
                if DBG: print(f'workspace [{pid}]: {_path}')
            elif '--type=fileWatcher' in psutil.Process(pid).cmdline(): #maybe files
                for _path in items:
                    if VALIDATE(_path) and Path(_path).is_file():
                        records.append( _path )
                        if DBG: print(f'file [{pid}]: {_path}')
            pass

        return records
    
    def _associate_with_window(self, records) -> list:
        results = list()

        windows = self.xm.get_windows_by_name('Visual Studio Code')
        windows.sort(key=lambda x:x['xid'])

        for _path in records:
            _name  = Path(_path).name
            _title = WINDOW_TITLE%_name
            for w in windows:
                if _title in w['name']:
                    results.append({
                        'name': _name,
                        'path': _path,
                        'window': {
                            'desktop':w['desktop'],
                            'states':w['states'],
                            'xyhw':w['xyhw']
                        }
                    }) 
                    continue
            pass

        return results

    def _rearrange_window(self, records: list):
        _time = time.time()
        _limit = len(records) * SLOT
        while len(records)!=0 and time.time()-_time<_limit:
            time.sleep(SLOT)

            windows = self.xm.get_windows_by_name('Visual Studio Code')
            windows.sort(key=lambda x:x['xid'])

            def _window_set(stat)->bool:
                _title = WINDOW_TITLE%stat['name']
                for idx,w in enumerate(windows):
                    if _title in w['name']:
                        s = stat['window']
                        self.xm.set_window_by_xid( w['xid'], s['desktop'], s['states'], s['xyhw'] )
                        windows.pop(idx)
                        return True
                return False
            
            records = [ stat for stat in records if not _window_set(stat) ]
            pass
        if DBG: print( 'Time elapsed: %.3f.'%(time.time()-_time) )
        pass

    def onStart(self):
        self.xm = CapabilityLibrary.CapabilityHandleLocal('x11-manager')
        self.il = CapabilityLibrary.CapabilityHandleLocal('inotify-lookup')
        self.il.register('code')
        return 0

    def onStop(self):
        self.il.unregister('code')
        return 0

    def onSave(self, stat_file):
        ## dump raw_result via il
        raw_results = self.il.dump('code')
        ## gathering record from raw_results
        records = self._gather_records(raw_results)
        ## associate with X window
        records = self._associate_with_window(records)
        ## write to file
        with open(stat_file, 'w') as f:
            json.dump(records, f)
            pass
        return 0

    def onResume(self, stat_file):
        ## load stat file with failure check
        with open(stat_file, 'r') as f:
            _file = f.read().strip()
        if len(_file)==0:
            return 0
        else:
            try:
                records = json.loads(_file)
            except:
                return -1
        ## open windows and wait for title
        for item in records:
            os.system( 'code --new-window "%s"'%item['path'] )
        pass
        ## rearrange windows by title
        self._rearrange_window(records)
        time.sleep(SLOT)
        return 0

    def onClose(self):
        ## force close all
        os.system('killall code')
        return 0
    pass

if __name__ == '__main__':
    _plugin = VscodePlugin()
    _plugin.onStart()

    raw_results = _plugin.il.dump('code')
    ## gathering record from raw_results
    records = _plugin._gather_records(raw_results)
    ## associate with X window
    records = _plugin._associate_with_window(records)
    print( json.dumps(records, indent=4) )
    pass
