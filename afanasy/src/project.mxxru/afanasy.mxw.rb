#!/usr/bin/env ruby
# This is a cross-platform build script.
# It should be used with Mxx_ru (http://www.rubyforge.com/projects/mxx-ru/)
#   installed and well-configured for your compiler system
#
# Use 'ruby build.rb' command to build it.
# Frequently used flags:
# --mxx-clean
# --mxx-cpp-debug
# --mxx-cpp-release

require 'rubygems'
gem 'Mxx_ru'
require 'mxx_ru/cpp'

Mxx_ru::Cpp::composite_target {
   # Microsoft compiler shows brief info by default
   Mxx_ru::enable_show_brief if( toolset.name !="vc" )
   # Defaults, can be overriden:
   default_rtti_mode Mxx_ru::Cpp::RTTI_DISABLED
   default_runtime_mode Mxx_ru::Cpp::RUNTIME_RELEASE
   # Requirements:
   rtl_mode( Mxx_ru::Cpp::RTL_SHARED )
   threading_mode( Mxx_ru::Cpp::THREADING_MULTI )

   # How to store data
   global_obj_placement( Mxx_ru::Cpp::CustomSubdirObjPlacement.new( "../../bin", "tmp/tmp" ))

puts "OS="+toolset.tag("target_os")

   # Projects (no support libs are needed here, they will be taken from dependencies)

    case toolset.tag( "target_os" )
        when "unix"
            case ENV['UNIXTYPE']
                when "MACOSX"
                    puts "Trying to build on MAC..."
#                   puts "QTDIR="+ENV['QTDIR']
#                   raise "QTDIR environment variable is not defined." if ENV['QTDIR'] == nil
            end
    end
    required_prj "server.mxx.rb"
    required_prj "cmd.mxx.rb"
    required_prj "talk.mxx.rb"
    required_prj "render.mxx.rb"
    required_prj "watch.mxx.rb"
    required_prj "monitor.mxx.rb"
}
