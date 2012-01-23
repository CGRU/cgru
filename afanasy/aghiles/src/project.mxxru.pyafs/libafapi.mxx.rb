#!/usr/bin/env ruby

Mxx_ru::Cpp::lib_target{

   target "afapi"
#   implib_path("")
   required_prj "libafanasy.mxx.rb"

   case toolset.tag( "target_os" )
      when "unix"
         define "UNIX"
         case ENV['UNIXTYPE']
            when "MACOSX"
               define "MACOSX"
               linker_option "-prebind -dynamiclib -single_module"
            else
               define "LINUX"
         end

      when "mswin"
         define "WINNT"
      else
         raise "#{toolset.tag('target_os')} platform is not supported."
   end

   cpp_sources Dir.glob( '../libafapi/*.cpp' )
}
