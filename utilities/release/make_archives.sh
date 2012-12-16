#!/bin/bash

cd archives

for archive in `ls`; do
   [ -L "$archive" ] && continue
   [ -d "$archive" ] && continue
   [ -f "$archive" ] || continue
   [ -x "$archive" ] || continue

   archive_dir="cgru-${archive}"
   if [ -L "${archive_dir}" ]; then
      archive_dir=`readlink "${archive_dir}"`
   else
      [ -d "${archive_dir}" ] && rm -rf "${archive_dir}"
      pushd .. > /dev/null
      ./export.sh "archives/${archive_dir}"
      popd > /dev/null
   fi
   for version in `cat -v "${archive_dir}/version.txt" | sed -e "s/\^M//g"`; do break; done
   echo "Creating CGRU archive for '${archive}': '${archive_dir}'-'${version}'"

   # Cleanup archive folders:
   echo "Clearing '${archive}':"
   find "${archive_dir}" -type d -name .git -exec rm -rvf {} \;
   find "${archive_dir}" -type f -name .gitignore -exec rm -vf {} \;
   find "${archive_dir}" -type d -name .svn -exec rm -rvf {} \;
   find "${archive_dir}" -type f -name *.pyc -exec rm -vf {} \;
   find "${archive_dir}" -type d -name __pycache__ -exec rm -rvf {} \;
   find "${archive_dir}" -type f -name config.json -exec rm -vf {} \;
   find "${archive_dir}" -type f -name farm.json -exec rm -vf {} \;
   rm -fv ${archive_dir}/software_setup/locate_*

   # Run specific script:
   "./${archive}" "${archive_dir}"
   if [ $? != 0 ]; then
      echo "Failed making archive."
      exit 1
   fi

   # Archivate:
   archivefile="cgru.${version}.${archive}.7z"
   [ -f "${archivefile}" ] && rm -fv "${archivefile}"
   7za a -r -y -t7z "${archivefile}" "${archive_dir}"
   if [ $? != 0 ]; then
      echo "Error creating archive."
      exit 1
   fi
   chmod a+rw "${archivefile}"
done

echo "Done."; exit 0
