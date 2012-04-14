#!/usr/bin/env ruby

Mxx_ru::Cpp::exe_target{

    target "afcmd"

    required_prj "libafsql.mxx.rb"

    qt = generator( Mxx_ru::Cpp::Qt4.new( self ) )
    qt.use_modules QT_CORE, QT_SQL

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

    cpp_sources Dir.glob( '../cmd/**/*.cpp' )
}
