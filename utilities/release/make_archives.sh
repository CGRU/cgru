#!/bin/bash

cd archives

for archive in ./*; do
	[ -L "$archive" ] && continue
	[ -d "$archive" ] && continue
	[ -f "$archive" ] || continue
	[ -x "$archive" ] || continue

	archive_dir="cgru-${archive}"
	if [ -L "${archive_dir}" ]; then
		archive_dir=$(readlink "${archive_dir}")
		[ -e "$archive_dir" ] || continue
	else
		[ -d "${archive_dir}" ] && rm -rf "${archive_dir}"
		pushd .. > /dev/null
		./export.sh "archives/${archive_dir}"
		popd > /dev/null
	fi
	for version in $(cat -v "${archive_dir}/version.txt" | sed -e "s/\^M//g"); do break; done
	echo "Creating CGRU archive for '${archive}': '${archive_dir}'-'${version}'"

	# Cleanup archive folders:
	echo "Clearing '${archive}':"
	find "${archive_dir}" -type d -name ".git" -exec rm -rvf {} \;
	find "${archive_dir}" -type f -name ".gitignore" -exec rm -vf {} \;
	find "${archive_dir}" -type d -name ".svn" -exec rm -rvf {} \;
	find "${archive_dir}" -type f -name "*.pyc" -exec rm -vf {} \;
	find "${archive_dir}" -type d -name "__pycache__" -exec rm -rvf {} \;
	find "${archive_dir}" -type f -name "config.json" -exec rm -vf {} \;
	find "${archive_dir}" -type f -name "farm.json" -exec rm -vf {} \;
	rm -fv ${archive_dir}/software_setup/locate_*

	# Run specific script:
	"./${archive}" "${archive_dir}"
	if [ $? != 0 ]; then
		echo "Failed making archive."
		exit 1
	fi

	# Archivate:
	case ${archive} in
	linux )
		archive_file="${PWD}/cgru.${version}.${archive}.tar.gz"
		archive_cmd="tar -cvzf \"${archive_file}\" -C \"$(dirname ${archive_dir})\" \"$(basename ${archive_dir})\""
		;;
	windows )
		archive_file="cgru.${version}.${archive}.zip"
		archive_cmd="7za a -r -y -tzip \"${archive_file}\" \"${archive_dir}\""
		;;
	esac

	if [ ! -z "$archive_cmd" ]; then
		[ -f "${archive_file}" ] && rm -fv "${archive_file}"
		echo "$archive_cmd"
		eval "$archive_cmd"
		if [ $? != 0 ]; then
			echo "Error creating archive."
			exit 1
		fi
		chmod a+rw "${archive_file}"
	fi

done

echo "Done."; exit 0
