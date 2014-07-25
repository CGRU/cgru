# Copyright (c) 2009, Evan Fosmark
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
# this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# The views and conclusions contained in the software and documentation are
# those of the authors and should not be interpreted as representing official
# policies, either expressed or implied, of the FreeBSD Project.

import os
import time
import errno


class FileLockException(Exception):
	pass


class FileLock(object):
	""" A file locking mechanism that has context-manager support so
		you can use it in a with statement. This should be relatively cross
		compatible as it doesn't rely on msvcrt or fcntl for the locking.
	"""

	__slots__ = ('fd', 'is_locked', 'lockfile', 'file_name', 'timeout',
				 'delay')

	def __init__(self, file_name, timeout=10, delay=.05):
		""" Prepare the file locker. Specify the file to lock and optionally
			the maximum timeout and the delay between each attempt to lock.
		"""
		self.is_locked = False
		self.lockfile = os.path.abspath(
			os.path.expanduser(os.path.expandvars("%s.lock" % file_name)))
		self.file_name = file_name
		self.timeout = timeout
		self.delay = delay

	def acquire(self):
		"""Acquire the lock, if possible. If the lock is in use, it check again
		every `wait` seconds. It does this until it either gets the lock or
		exceeds `timeout` number of seconds, in which case it throws an
		exception.
		"""
		start_time = time.time()
		pid = os.getpid()
		while True:
			try:
				self.fd = os.open(self.lockfile,
								  os.O_CREAT | os.O_EXCL | os.O_RDWR)
				os.write(self.fd, "%d" % pid)
				break
			except OSError as e:
				if e.errno != errno.EEXIST:
					raise
				if (time.time() - start_time) >= self.timeout:
					raise FileLockException("Timeout occured.")
				time.sleep(self.delay)
		self.is_locked = True

	def release(self):
		"""Get rid of the lock by deleting the lockfile.
		When working in a `with` statement, this gets automatically
		called at the end.
		"""
		if self.is_locked:
			os.close(self.fd)
			os.unlink(self.lockfile)
			self.is_locked = False

	def __enter__(self):
		"""Activated when used in the with statement.
		Should automatically acquire a lock to be used in the with block.
		"""
		if not self.is_locked:
			self.acquire()
		return self

	def __exit__(self, type, value, traceback):
		"""Activated at the end of the with statement.
		It automatically releases the lock if it isn't locked.
		"""
		if self.is_locked:
			self.release()

	def __del__(self):
		"""Make sure that the FileLock instance doesn't leave a lockfile
		lying around.
		"""
		self.release()
