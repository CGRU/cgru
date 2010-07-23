#!/usr/bin/ruby

require 'mxx_ru/cpp'

#Mxx_ru::Cpp::dll_target {
Mxx_ru::Cpp::macos_bundle_target {
   target_root ""
   target "cgru"

   maya_location = ENV['MAYA_LOCATION']
   raise "MAYA_LOCATION environment variable is not defined." if maya_location == nil
   maya_arch=ENV['MAYA_ARCH']

   include_path "mr_shaders/src"

   define "REQUIRE_IOSTREAM"

   case toolset.tag( "target_os" )
      when "mswin"
         target_ext ".mll"
         define "WINNT"
         define "NT_PLUGIN"
         define "WIN32"
         define "_WINDOWS"
         define "_CRT_SECURE_NO_DEPRECATE"
         include_path "#{maya_location}/include"
         lib_path "#{maya_location}/lib"
         lib "Foundation"
         lib "OpenMaya"
         lib "OpenMayaFX"
         lib "OpenMayaAnim"
         lib "OpenMayaUI"
         lib "OpenMayaRender"
         compiler_option "/GR"
         compiler_option "/GS"
         compiler_option "/EHac"
         linker_option "/export:initializePlugin"
         linker_option "/export:uninitializePlugin"
#         linker_option "/NODEFAULTLIB"
         lib "user32.lib"
         if maya_arch == nil || (maya_arch.empty? || maya_arch.include?("32"))
               define "Bits32_" # suppose it's 32bit if no architecture specified
         else
            if maya_arch.include?("64")
               define "Bits64_"
            end
         end

      when "unix"
         define "UNIX"
         case ENV['UNIXTYPE']
            when "MACOSX"
               puts "Trying to build on MAC..."
               define "MACOSX"
               define "OSMac_"
               target_ext ".bundle"
               include_path "#{maya_location}/devkit/include"
               lib_path "#{maya_location}/Maya.app/Contents/MacOS"
               lib "Foundation"
               lib "OpenMaya"
               lib "OpenMayaFX"
               lib "OpenMayaAnim"
               lib "OpenMayaUI"
               lib "OpenMayaRender"
               include_path "/usr/X11/include"
               include_path "/System/Library/Frameworks/AGL.framework/Versions/A/Headers"
               lib_path "/usr/X11/lib"
               linker_option "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib"
#               linker_option "-bundle"
            else
               define "LINUX"
               include_path "#{maya_location}/include"
               lib_path "#{maya_location}/lib"
               lib "Foundation"
               lib "OpenMaya"
               lib "OpenMayaFX"
               lib "OpenMayaAnim"
               lib "OpenMayaUI"
               lib "OpenMayaRender"
               compiler_option "-fno-strict-aliasing"
               compiler_option "-Wno-deprecated"
               compiler_option "-Wall"
               compiler_option "-O3"
               compiler_option "-Wno-multichar"
               compiler_option "-Wno-comment"
               compiler_option "-Wno-sign-compare"
               compiler_option "-funsigned-char"
               compiler_option "-Wno-reorder"
               compiler_option "-fno-gnu-keywords"
               compiler_option "-ftemplate-depth-25"
               compiler_option "-pthread"
               linker_option "-fno-strict-aliasing"
               linker_option "-Wno-deprecated"
               linker_option "-Wall"
               linker_option "-O3"
               linker_option "-Wno-multichar"
               linker_option "-Wno-comment"
               linker_option "-Wno-sign-compare"
               linker_option "-funsigned-char"
               linker_option "-Wno-reorder"
               linker_option "-fno-gnu-keywords"
               linker_option "-ftemplate-depth-25"
               linker_option "-pthread"
               linker_option "-Wl,-Bsymbolic"
               include_path "/usr/include/X11"
               if  maya_arch == nil || (maya_arch.empty? || maya_arch.include?("32"))
                  compiler_option "-m32" # suppose it's 32bit if no architecture specified
                  linker_option "-m32"
               else
                  if maya_arch.include?("64")
                     define "Bits64_"
                     define "LINUX_64"
                     compiler_option "-m64"
                     linker_option "-m64"
                  end
               end
         end
         define "FUNCPROTO"
         define "_GNU_SOURCE"
         define "_BOOL"
         define "REQUIRE_IOSTREAM"

         lib_path "/usr/lib"
         lib "X11"
      else
         raise "${toolset.tag( 'target_os' )} platform is not (yet) supported."
   end

   cpp_sources Dir.glob( 'src/**/*.cpp' )
}
