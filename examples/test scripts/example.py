#!/usr/bin/env python

import af

job = af.Job('example job')

block = af.Block('block of tasks')
block.setWorkingDirectory('/home')

task = af.Task('simple task')
task.setCommand('ls -l')

block.tasks.append(task)
job.blocks.append(block)

job.send()
