#!/usr/bin/env ruby

require 'mxx_ru/cpp/qt4_via_pkg_config'

Mxx_ru::Cpp::lib_target{

   target "afqt"

   required_prj "libafanasy.mxx.rb"

   qt = generator( Mxx_ru::Cpp::Qt4.new( self ) )
   qt.use_modules QT_GUI, QT_CORE, QT_NETWORK, QT_XML

   qt.moc_result_subdir = "../project.mxxru/tmp/libafqt/moc"
   qt.hpp_ext = ".h"

   qt.h2moc "../libafqt/qserver.h"
   qt.h2moc "../libafqt/qserverthread.h"
   qt.h2moc "../libafqt/qthreadclient.h"
   qt.h2moc "../libafqt/qthreadclientsend.h"
   qt.h2moc "../libafqt/qthreadclientup.h"

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
         raise "${toolset.tag( 'target_os' )} platform is not supported."
   end

   cpp_sources Dir.glob( '../libafqt/*.cpp' )
}
