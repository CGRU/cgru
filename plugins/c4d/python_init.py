# -*- coding: utf-8 -*-
import os
import cgruutils

c4d_plugin_folder = ''
try:
	# Get the location of the C4D-Plugin-Folder
	c4d_plugin_folder = os.environ['C4D_PLUGIN_LOCATION']
except KeyError:
	print("ERROR: The variable C4D_PLUGIN_LOCATION is not set! So C4D will "
		  "may not work properly!")
	exit()

cgru_c4d_plugin_folder = ''
try:
	# Get the location of the Plugin-Folders where to Plugins should get copied
	# from
	cgru_c4d_plugin_folder = os.environ['C4D_CGRU_SCRIPTS_LOCATION']
except KeyError:
	print("ERROR: The variable C4D_CGRU_SCRIPTS_LOCATION is not set! So C4D "
		  "will may not work properly!")
	exit()


# Now combine all the Folders where the Plugins should get copied from
all_c4d_plugin_folders = [cgru_c4d_plugin_folder]

try:
	# Look if additional plugin folders are set
	# and add them if that is the case
	cgru_additional_plugin_folders = \
		os.environ['C4D_ADDITIONAL_PLUGIN_FOLDERS']
	print(cgru_additional_plugin_folders)
	all_c4d_plugin_folders.extend(cgru_additional_plugin_folders.split(";"))
except KeyError:
	pass

# Give the user a some information about what is happening
print("\nSource Plugin-Folders:")
for folder in all_c4d_plugin_folders:
	print("   -%s" % folder)


# Check first of Plugin-Directory exists where the plugins should get copied to
if os.path.isdir(c4d_plugin_folder):
	copied_plugins = \
		cgruutils.copy_directory(all_c4d_plugin_folders, c4d_plugin_folder)

	if copied_plugins == 0:
		print("\nAll Plugins were already up to date!\n")
	else:
		print("\n%d Files got updated!\n" % copied_plugins)
else:
	print (
		'\nERROR: The specified C4D-Plugin-Directory(%s) could not get '
		'found so the plugins did not get copied to it!\n' % c4d_plugin_folder
	)
