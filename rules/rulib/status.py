import json
import os

import rulib

class Status:

    def __init__(self, i_uid = None, i_path = None):

        self.path = rulib.functions.getRuFilePath('status.json', i_path)
        self.muser = i_uid
        if self.muser is None:
            self.muser = rulib.functions.getCurUser()
        self.mtime = rulib.functions.getCurSeconds()

        data = rulib.functions.readObj(self.path)
        if data is None:
            self.data = dict()
        if not 'status' in data:
            self.data = dict()
        else:
            self.data = data['status']

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
                name = '_'.join(tags.sort())
            task['name'] = name
            task['tags'] = tags
            task['artists'] = []
            task['flags'] = []
            task['progress'] = 0
            task['cuser'] = self.muser
            task['ctime'] = self.mtime

            if not 'tasks' in self.data:
                self.data['tasks'] = dict()
            self.data['tasks'][name] = task

        else:
            task['muser'] = self.muser
            task['mtime'] = self.mtime

        if artists is not None and type(artists) is list:
            task['artists'] = artists
        if flags is not None and type(flags) is list:
            task['flags'] = flags
        if annotation is not None and type(annotation) is str:
            task['annotation'] = annotation
        if deleted is not None:
            task['deleted'] = deleted

        progress_changed = True
        if progress is not None and type(progress) is int:
            if progress < -1: progress = -1
            elif progress > 100: progress = 100
            if task['progress'] == progress:
                progress_changed = False
            task['progress'] = progress

        # Calculate status progress
        progresses = dict()
        if progress_changed:
            avg_progress = 0.0
            num_tasks = 0

            for t in self.data['tasks']:
                task = self.data['tasks'][t]
                if 'deleted' in task and task['deleted']:
                    continue

                koeff = 1.0
                if t in rulib.RULES_TOP['tags'] and 'koeff' in rulib.RULES_TOP['tags'][t]:
                    koeff = rulib.RULES_TOP['tags'][t]['koeff']

                avg_progress += koeff * task['progress']
                num_tasks += koeff

            avg_progress = int(avg_progress / num_tasks)
            progresses[self.path] = avg_progress
            self.data['progress'] = avg_progress

        # Remove status tags, artists and flags if the task has the same
        for arr in ['tags','artists','flags']:
            if self.data[arr] and len(self.data[arr]):
                for item in task[arr]:
                    if item in self.data[arr]:
                        self.data[arr].remove(item)

        return task


    def save(self):
        self.data['mtime'] = self.mtime
        self.data['muser'] = self.muser

        rulib.functions.writeObj(self.path, {"status":self.data})
