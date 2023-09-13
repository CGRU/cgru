import json
import os
import traceback

import rulib

def getStatusFilePath(i_path = None):
    return rulib.functions.getRuFilePath('status.json', i_path)

def getStatusData(i_path = None, out = None):
    data = rulib.functions.readObj(getStatusFilePath(i_path), out)
    if data is None:
        return None
    if 'status' in data:
        return data['status']
    error = 'Status data not found'
    if out is not None: out['error'] = error
    else: print(error)
    return  None

def saveStatusData(i_path, i_data, out=None):
    path = getStatusFilePath(i_path)
    rufolder = os.path.dirname(path)
    if not os.path.isdir(rufolder):
        try:
            os.makedirs(rufolder)
        except PermissionError:
            err = 'Permissions denied to make folder for status: ' + rufolder
            if out is not None: out['error'] = err
            else: print(err)
            return
        except:
            err = 'Unable to create folder for status: ' + rufolder
            err += '\n%s' % traceback.format_exc()
            if out is not None: out['error'] = err
            else: print(err)
            return
    rulib.functions.writeObj(path, {'status':i_data}, out)

class Status:

    def __init__(self, uid=None, path=None):

        if uid is None:
            uid = rulib.functions.getCurUser()

        self.path = rulib.functions.getRootPath(path)

        self.data = getStatusData(self.path)
        if self.data is None:
            self.data = dict()

        self.data['muser'] = uid
        self.data['mtime'] = rulib.functions.getCurSeconds()

        self.progress_changed = False

    def __repr__(self):
        return json.dumps(self.data)

    def __str__(self):
        return json.dumps({"status":self.data})

    def findTask(self, name=None, tags=None, out=None):
        if 'tasks' not in self.data:
            return None
        if len(self.data['tasks']) == 0:
            return None
        if name is not None:
            if name in self.data['tasks']:
                return self.data['tasks'][name]
            return None

        if tags is None or not type(tags) is list or len(tags) == 0:
            error = 'No task name or tags list specified.'
            if out is None: print(error)
            else: out['error'] = error
            return None

        for task in self.data['tasks']:
            task = self.data['tasks'][task]
            if len(tags) != len(task['tags']):
                continue
            for tag in tags:
                if not tag in task['tags']:
                    task = None
                    break
            if task is not None:
                return task

        return None


    def set(self, tags=None, tags_keep=None, artists=None, artists_keep=None, flags=None, flags_keep=None, progress=None, annotation=None, color=None, out=None):
        # Store original progress to compare later
        # and find out that is was changed.
        _progress = self.data.get('progress')

        if annotation is not None and type(annotation) is str:
            self.data['annotation'] = annotation

        if color is not None and type(color) is list:
            if len(color) == 3:
                self.data['color'] = color
            else:
                if 'color' in self.data:
                    del self.data['color']

        if progress is not None and type(progress) is int:
            if progress < -1: progress = -1
            elif progress > 100: progress = 100
            self.data['progress'] = progress

        self.setItems('flags',   items_keep=flags_keep,   items_add=flags)
        self.setItems('tags',    items_keep=tags_keep,    items_add=tags)
        self.setItems('artists', items_keep=artists_keep, items_add=artists)

        # If shot progress is 100% all tasks should be 100% done.
        if self.data.get('progress') == 100 and 'tasks' in self.data:
            for t in self.data['tasks']:
                task = self.data['tasks'][t]
                if task.get('progress') != 100:
                    task['progress'] = 100;
                    task['changed'] = True
                if 'flags' in task and 'done' not in task['flags']:
                    task['flags'] = ['done']
                    task['changed'] = True

        # If shot has OMIT flags, all tasks should be omitted
        if 'flags' in self.data and 'omit' in self.data['flags'] and 'tasks' in self.data:
            for t in self.data['tasks']:
                task = self.data['tasks'][t]
                if ('flags' not in task) or ('omit' not in task['flags']) or (task.get('progress') != -1):
                    # Skip done tasks
                    if task.get('progress') == 100:
                        continue
                    task['flags'] = ['omit']
                    task['progress'] = -1
                    task['changed'] = True

        # If progress was changed we should update upper progress:
        if _progress != self.data.get('progress'):
            self.progress_changed = True

        self.data['changed'] = True

        return self.data


    def setItems(self, item_name, items_keep=None, items_add=None):
        if items_keep is None and items_add is None:
            return
        if items_keep is None:
            items_keep = []
        if items_add is None:
            items_add = []
        if not type(items_keep) is list:
            return
        if not type(items_add) is list:
            return

        if not item_name in self.data:
            self.data[item_name] = []

        # Remove items:
        _items = []
        for i in self.data[item_name]:
            if i in items_keep or i in items_add:
                _items.append(i)
        self.data[item_name] = _items

        # Add items:
        for i in items_add:
            # Skip items that are already set
            if i in self.data[item_name]:
                continue

            if item_name == 'flags' and i in rulib.RULES_TOP[item_name]:
                # Flag can limit minimum and maximum progress percentage:
                p_min = rulib.RULES_TOP[item_name][i].get('p_min')
                p_max = rulib.RULES_TOP[item_name][i].get('p_max')
                progress = self.data.get('progress')

                if (p_min is not None) and ((progress is None) or (progress < p_min)):
                    progress = p_min;

                if (p_max is not None) and ((progress is None) or (p_max < 0) or (progress > p_max)):
                    progress = p_max;

                if progress is not None:
                    self.data['progress'] = progress

                # Flag can be exclusive, so we should delete other items:
                mode = rulib.RULES_TOP[item_name][i].get('mode')
                if mode == 'stage' or mode == 'super':
                    self.data[item_name] = []

            self.data[item_name].append(i)


    def setTask(self, name=None, tags=None, artists=None, flags=None, progress=None, annotation=None, deleted=None, out=None):

        task = self.findTask(name, tags)

        if task is None:
            # Create a new task:
            task = dict()
            if tags is None or not type(tags) is list or len(tags) == 0:
                error = 'No new task tags list specified.'
                if out is None: print(error)
                else: out['error'] = error
                return None
            if name is None:
                if tags is None or len(tags) == 0:
                    error = 'Task should have name or tags.'
                    if out is None: print(error)
                    else: out['error'] = error
                    return None
                tags.sort()
                name = '_'.join(tags)
            task['name'] = name
            task['tags'] = tags
            task['artists'] = []
            task['flags'] = []
            task['progress'] = 0
            task['cuser'] = self.data['muser']
            task['ctime'] = self.data['mtime']

            if not 'tasks' in self.data:
                self.data['tasks'] = dict()
            self.data['tasks'][name] = task

        else:
            task['muser'] = self.data['muser']
            task['mtime'] = self.data['mtime']

        if artists is not None and type(artists) is list:
            task['artists'] = artists
        if flags is not None and type(flags) is list:
            task['flags'] = flags
        if annotation is not None and type(annotation) is str:
            task['annotation'] = annotation
        if deleted:
            task['deleted'] = True
        elif 'deleted' in task:
            del task['deleted']

        if progress is not None and type(progress) is int:
            if progress < -1: progress = -1
            elif progress > 100: progress = 100
        elif 'progress' in task:
            progress = task['progress']
        else:
            progress = 0

        # Task flags can detemine task progress (eg done=100%)
        if flags is not None and len(flags):
            for flag in flags:
                if not flag in rulib.RULES_TOP['flags']:
                    continue
                p_min = None
                p_max = None
                if 'p_min' in rulib.RULES_TOP['flags'][flag]:
                    p_min = rulib.RULES_TOP['flags'][flag]['p_min']
                if 'p_max' in rulib.RULES_TOP['flags'][flag]:
                    p_max = rulib.RULES_TOP['flags'][flag]['p_max']
                if p_min is not None and progress < p_min: progress = p_min
                if p_max is not None and progress > p_max: progress = p_max

        # Set task progress if it changes:
        if not 'progress' in task or task['progress'] != progress:
            task['progress'] = progress

            # Calculate status progress - tasks progress average
            avg_progress = 0.0
            num_tasks = 0

            for t in self.data['tasks']:
                _task = self.data['tasks'][t]
                if 'deleted' in _task and _task['deleted']:
                    continue

                koeff = 1.0
                if t in rulib.RULES_TOP['tags'] and 'koeff' in rulib.RULES_TOP['tags'][t]:
                    koeff = rulib.RULES_TOP['tags'][t]['koeff']

                avg_progress += koeff * _task['progress']
                num_tasks += koeff

            if num_tasks > 0:
                avg_progress = round(avg_progress / num_tasks)
                # Set status progress if it changes:
                if 'progress' not in self.data or self.data['progress'] != avg_progress:
                    self.data['progress'] = avg_progress
                    self.progress_changed = True
                else:
                    self.progress_changed = False


        # Remove status tags, artists and flags if the task has the same
        for arr in ['tags','artists','flags']:
            if arr in self.data and len(self.data[arr]):
                for item in task[arr]:
                    if item in self.data[arr]:
                        self.data[arr].remove(item)

        # Set task changed.
        # It needed for news to know what was changed in status.
        task['changed'] = True

        return task


    def prepareDataForSave(self):
        if 'changed' in self.data:
            del self.data['changed']
        if 'tasks' in self.data:
            for t in self.data['tasks']:
                if 'changed' in self.data['tasks'][t]:
                    del self.data['tasks'][t]['changed']


    def save(self, out=dict()):
        self.prepareDataForSave()

        saveStatusData(self.path, self.data, out)

        out['status'] = self.data


def updateUpperProgresses(i_path, i_progresses, out):
    #print(i_path)
    path = ''
    folders = []
    for folder in i_path.split('/'):
        if len(folder) == 0: continue
        path += '/' + folder
        folders.append(path)

    out['progresses'] = dict()

    folders.reverse()
    for folder in folders:
        #print(folder)
        try:
            listdir = os.listdir(rulib.functions.getAbsPath(folder))
        except:
            return

        progress_sum = 0
        progress_count = 0
        for entry in listdir:
            if entry == rulib.RUFOLDER:
                continue

            progress = 0
            sdata = None
            path = os.path.join(folder, entry)
            sdata = getStatusData(path)

            # Skip aux folders:
            if rulib.functions.isAuxFolder(path, sdata):
                continue

            if path in i_progresses:
                progress = i_progresses[path]
            else:
                if sdata and 'progress' in sdata and type(sdata['progress']) is int:
                    progress = sdata['progress']

            # Clamp progress to 100%
            if progress >= 100:
                progress = 100

            # If status has tasks assuming that it is a shot.
            # We want to count only 100% done shots.
            # if sdata and 'tasks' in sdata:
            #    if progress < 100:
            #        progress = 0;

            progress_sum += progress
            progress_count += 1

        if progress_count == 0:
            return

        progress_avg = int(progress_sum / progress_count)
        i_progresses[folder] = progress_avg

        #print("%s %d%%" % (folder, progress_avg))

        sdata = getStatusData(folder)
        if sdata is None:
            sdata = {'progress':progress_avg}
        else:
            sdata['progress'] = progress_avg
        saveStatusData(folder, sdata)

        out['progresses'][folder] = progress_avg
