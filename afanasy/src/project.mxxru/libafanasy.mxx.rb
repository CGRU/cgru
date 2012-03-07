#!/usr/bin/env ruby

Mxx_ru::Cpp::lib_target{

    target "afanasy"

    define "CGRU_REVISION=#{ENV['CGRU_REVISION']}"

    case toolset.tag( "target_os" )
        when "unix"
            define "UNIX"
            compiler_option( "-fPIC", MxxRu::Cpp::Target::OPT_UPSPREAD)
            compiler_option( "-Wall", MxxRu::Cpp::Target::OPT_UPSPREAD)
            IO.popen("#{ENV['AF_PYTHON']}-config --includes"){|f| compiler_option(f.gets.chop, MxxRu::Cpp::Target::OPT_UPSPREAD )}
            IO.popen("#{ENV['AF_PYTHON']}-config --ldflags"){|f| linker_option(f.gets.chop, MxxRu::Cpp::Target::OPT_UPSPREAD )}

            case ENV['UNIXTYPE']
                when "MACOSX"
                    define "MACOSX"
#                   compiler_option("-w", MxxRu::Cpp::Target::OPT_UPSPREAD)
                    linker_option "-prebind -dynamiclib -single_module"

                else
                    define "LINUX"
            end
        when "mswin"
            define "WINNT"
            include_path("#{ENV['PYTHONDIR']}/include", MxxRu::Cpp::Target::OPT_UPSPREAD )
            lib_path "#{ENV['PYTHONDIR']}/libs"
            lib "#{ENV['PYTHONLIB']}"
            lib "wsock32"

        else
            raise "#{toolset.tag('target_os')} platform is not supported."
    end

    cpp_sources Dir.glob( '../libafanasy/**/*.cpp' )
}
