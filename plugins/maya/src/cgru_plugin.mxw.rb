#!/usr/bin/env ruby
# This is a cross-platform build script.
# It should be used with Mxx_ru (http://www.rubyforge.com/projects/mxx-ru/)
#   installed and well-configured for your compiler system
#
# Use 'ruby build.rb' command to build it.
# NOTE: MAYA_LOCATION environment variable should be set to actual value.
# Frequently used flags:
# --mxx-clean
# --mxx-cpp-debug
# --mxx-cpp-release

require 'rubygems'
#gem 'Mxx_ru'
require 'mxx_ru/cpp'

Mxx_ru::Cpp::composite_target {
   # Microsoft compiler shows brief info by default
   Mxx_ru::enable_show_brief if( toolset.name !="vc" )
   # Defaults, can be overriden:
   default_rtti_mode Mxx_ru::Cpp::RTTI_DISABLED
   default_runtime_mode Mxx_ru::Cpp::RUNTIME_RELEASE
   # Requirements:
   rtl_mode( Mxx_ru::Cpp::RTL_STATIC )
   threading_mode( Mxx_ru::Cpp::THREADING_MULTI )

   # How to store data
   raise "MAYA_VERSION environment variable is not defined." if ENV['MAYA_VERSION'] == nil
   global_obj_placement( Mxx_ru::Cpp::CustomSubdirObjPlacement.new( "../../maya/mll/#{ENV['MAYA_VERSION']}#{ENV['MAYA_ARCH']}", "tmp/#{ENV['MAYA_VERSION']}#{ENV['MAYA_ARCH']}/#{mxx_runtime_mode}" ))

   # Projects
   required_prj "cgru_plugin.mxx.rb"
}
