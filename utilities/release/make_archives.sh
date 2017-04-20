#!/bin/bash

cd archives

for archive_dir in `ls`; do
	[ -d "$archive_dir" ] || continue

	if [[ "${archive_dir}" =~ cgru.* ]]; then
		echo "Skipping processed archive '${archive_dir}'"
		continue
	fi

	for version in `cat -v "${archive_dir}/version.txt" | sed -e "s/\^M//g"`; do break; done
	echo "Creating CGRU archive for '${archive_dir}' '${version}'"

	# Cleanup archive dir:
	echo "Clearing archive folder '${archive_dir}':"
	find "${archive_dir}" -type d -name .git -exec rm -rvf {} \;
	find "${archive_dir}" -type f -name .gitignore -exec rm -vf {} \;
	find "${archive_dir}" -type d -name .svn -exec rm -rvf {} \;
	find "${archive_dir}" -type f -name *.pyc -exec rm -vf {} \;
	find "${archive_dir}" -type d -name __pycache__ -exec rm -rvf {} \;
	find "${archive_dir}" -type f -name config.json -exec rm -vf {} \;
	find "${archive_dir}" -type f -name farm.json -exec rm -vf {} \;
	rm -fv ${archive_dir}/software_setup/locate_*

	# Run specific script:
	archive_script="${archive_dir}.sh"
	if [ -x "${archive_script}" ]; then
		echo "Executing archive specific script '${archive_script}':"
		"./${archive_script}" "${archive_dir}"
		if [ $? != 0 ]; then
			echo "Failed executing archive specific script '${archive_script}'."
			exit 1
		fi
	fi

	# Rename Archive Dir:
	archive_name="${archive_dir}"
	archive_dir="cgru.${version}"
	[ -d "${archive_dir}" ] && rm -rvf "${archive_dir}"
	echo "Renaming archive folder:" 
	mv -v "${archive_name}" "${archive_dir}"

	# Archivate:
	case ${archive_name} in
	windows)
		archive_file="${archive_dir}.${archive_name}.zip"
		archive_cmd="7za a -r -y -tzip \"${archive_file}\" \"${archive_dir}\""
		;;
	*)
		archive_file="${archive_dir}.${archive_name}.tar.gz"
		archive_cmd="tar -cvzf \"${archive_file}\" \"${archive_dir}\""
		;;
	esac

	if [ ! -z "$archive_cmd" ]; then
		[ -f "${archive_file}" ] && rm -fv "${archive_file}"
		echo $archive_cmd
		eval $archive_cmd
		if [ $? != 0 ]; then
			echo "Error creating archive."
			exit 1
		fi
		chmod a+rw "${archive_file}"
	fi

	# Rename archive folder as processed:
	processed="${archive_dir}.${archive_name}"
	echo "Renaming archive folder as processed:" 
	[ -d "${processed}" ] && rm -rvf "${processed}"
	mv -v "${archive_dir}" "${processed}"

done

echo "Done."; exit 0
