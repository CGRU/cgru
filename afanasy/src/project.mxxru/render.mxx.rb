#!/usr/bin/env ruby

Mxx_ru::Cpp::exe_target {

    target "afrender"

    required_prj "libafqt.mxx.rb"

    case toolset.tag("target_os")
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
            lib "Advapi32"
            lib "User32"
        else
            raise "#{toolset.tag('target_os')} platform is not supported."
    end

    cpp_sources Dir.glob('../render/*.cpp')
}
