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
   for version in `cat "${archive_dir}/version.txt"`; do break; done
   echo "Creating CGRU archive for '${archive}': '${archive_dir}'-'${version}'"

   # Run specific script:
   "./${archive}" "${archive_dir}"
   if [ $? != 0 ]; then
      echo "Failed making archive."
      exit 1
   fi

   # Cleanup files:
   find $PWD -type f -name *.pyc -exec rm -vf {} \;
   find $PWD -type d -name .svn -exec rm -rvf {} \;
   find $PWD -type d -name __pycache__ -exec rm -rvf {} \;

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
