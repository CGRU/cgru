#!/usr/bin/env ruby

require 'mxx_ru/cpp/qt4_via_pkg_config'

Mxx_ru::Cpp::exe_target{

    target "afserver"

    lib "pthread"
    required_prj "libafsql.mxx.rb"

    case toolset.tag( "target_os" )
        when "unix"
            define "UNIX"

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
