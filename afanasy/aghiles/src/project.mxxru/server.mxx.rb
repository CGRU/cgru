#!/usr/bin/env ruby

require 'mxx_ru/cpp/qt4_via_pkg_config'

Mxx_ru::Cpp::exe_target{

   target "afserver"

   lib "pthread"
   required_prj "libafnetwork.mxx.rb"
   required_prj "libafsql.mxx.rb"

   qt = generator( Mxx_ru::Cpp::Qt4.new( self ) )
   qt.use_modules QT_CORE, QT_SQL

   case toolset.tag( "target_os" )
      when "unix"

         case ENV['UNIXTYPE']
            when "MACOSX"
               define "MACOSX"
               linker_option "-prebind"
            else
               define "LINUX"
         end
      when "mswin"
         define "WINNT"
     else
         raise "${toolset.tag( 'target_os' )} platform is not supported."
   end

   cpp_sources Dir.glob('../server/**/*.cpp' )
}
