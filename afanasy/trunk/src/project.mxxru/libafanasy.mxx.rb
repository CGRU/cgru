#!/usr/bin/env ruby

require 'mxx_ru/cpp/qt4_via_pkg_config'

Mxx_ru::Cpp::lib_target{

   target "afanasy"

   qt = generator( Mxx_ru::Cpp::Qt4.new( self ) )
   qt.use_modules QT_CORE, QT_NETWORK, QT_XML

   case toolset.tag( "target_os" )
      when "unix"

         compiler_option( "-fPIC", MxxRu::Cpp::Target::OPT_UPSPREAD)
         compiler_option( "-Wall", MxxRu::Cpp::Target::OPT_UPSPREAD)
         IO.popen("#{ENV['AF_PYTHON']}-config --includes"){|f| compiler_option(f.gets.chop, MxxRu::Cpp::Target::OPT_UPSPREAD )}
         IO.popen("#{ENV['AF_PYTHON']}-config --ldflags"){|f| linker_option(f.gets.chop, MxxRu::Cpp::Target::OPT_UPSPREAD )}

         case ENV['UNIXTYPE']
            when "MACOSX"
               define "MACOSX"
#               compiler_option("-w", MxxRu::Cpp::Target::OPT_UPSPREAD)
               linker_option "-prebind -dynamiclib -single_module"

            else
               define "LINUX"
         end
      when "mswin"
         define "WINNT"
         include_path("#{ENV['MINGWDIR']}/include",  MxxRu::Cpp::Target::OPT_UPSPREAD )
         lib_path(    "#{ENV['MINGWDIR']}/lib")
         include_path("#{ENV['QTDIR']}/include",     MxxRu::Cpp::Target::OPT_UPSPREAD)
         lib_path(    "#{ENV['QTDIR']}/lib")
         include_path("#{ENV['PYTHONDIR']}/include", MxxRu::Cpp::Target::OPT_UPSPREAD )
         lib_path "#{ENV['PYTHONDIR']}/libs"
         lib "#{ENV['PYTHONLIB']}"
         lib "wsock32"

      else
         raise "#{toolset.tag('target_os')} platform is not supported."
   end

   cpp_sources Dir.glob( '../libafanasy/**/*.cpp' )
}
