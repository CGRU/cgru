#!/usr/bin/env ruby

require 'rubygems'
gem 'Mxx_ru'
require 'mxx_ru/cpp'
require 'mxx_ru/cpp/qt'

Mxx_ru::Cpp::exe_target( "prj.rb" ){
   # Microsoft compiler shows brief info by default
   Mxx_ru::enable_show_brief if( toolset.name !="vc" )

   qt = generator( Mxx_ru::Cpp::Qt_gen.new( self ) )
   qt.moc_result_subdir = "moc"
   qt.uic_result_subdir = "uic"
   qt.hpp_ext = ".h"
   qt.h2moc "object.h"
   qt.h2moc "process.h"

   lib "pthread"
   IO.popen("pkg-config QtGui QtCore QtNetwork --cflags"){|f| compiler_option(f.gets.chop)}
   IO.popen("pkg-config QtGui QtCore QtNetwork --libs"){|f| linker_option(f.gets.chop)}

   target( "exe" )
   cpp_source( "../src/parser.cpp" )
   cpp_source( "process.cpp" )
   cpp_source( "object.cpp" )
   cpp_source( "main.cpp" )
}
