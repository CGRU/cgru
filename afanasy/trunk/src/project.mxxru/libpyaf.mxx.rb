#!/usr/bin/env ruby

require 'mxx_ru/cpp/qt4_via_pkg_config'

Mxx_ru::Cpp::dll_target{

   target "pyaf"
   target_prefix ""
   required_prj "libafapi.mxx.rb"

   qt = generator( Mxx_ru::Cpp::Qt4.new( self ) )
   qt.use_modules QT_CORE, QT_NETWORK, QT_XML

   define "CGRU_REVISION=#{ENV['CGRU_REVISION']}"

   case toolset.tag( "target_os" )

      when "unix"
         case ENV['UNIXTYPE']

            when "MACOSX"
               define "MACOSX"
               linker_option "-prebind -dynamiclib -single_module"

            else
               define "LINUX"
               compiler_option "-fno-strict-aliasing"

         end

      when "mswin"
         define "WINNT"
         target_ext '.pyd'
      else
         raise "#{toolset.tag('target_os')} platform is not supported."
   end

   cpp_sources Dir.glob('../libpyaf/*.cpp' )
}
