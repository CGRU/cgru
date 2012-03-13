#!/usr/bin/env ruby

require 'mxx_ru/cpp/qt4_via_pkg_config'

Mxx_ru::Cpp::exe_target {

   target "afrender"

   required_prj "libafqt.mxx.rb"

   qt = generator( Mxx_ru::Cpp::Qt4.new( self ) )
   qt.use_modules QT_GUI, QT_CORE, QT_NETWORK, QT_XML

   qt.moc_result_subdir = "../project.mxxru/tmp/render/moc/"
   qt.hpp_ext = ".h"
   qt.h2moc "../render/childprocess.h"
   qt.h2moc "../render/qobject.h"
   qt.h2moc "../render/taskprocess.h"

   case toolset.tag("target_os")
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
         raise "#{toolset.tag('target_os')} platform is not supported."
   end

   cpp_sources Dir.glob('../render/*.cpp')
}
