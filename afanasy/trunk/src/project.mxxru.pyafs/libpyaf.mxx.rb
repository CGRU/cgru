#!/usr/bin/env ruby

Mxx_ru::Cpp::dll_target{

   target 'pyaf'
   target_prefix ''
   required_prj 'libafapi.mxx.rb'

   define "CGRU_REVISION=#{ENV['CGRU_REVISION']}"

   case toolset.tag('target_os')

      when 'unix'

         linker_option ENV['AF_LFLAGS']

         case ENV['UNIXTYPE']
            when 'MACOSX'
               define 'MACOSX'
               linker_option '-prebind -dynamiclib -single_module'
            else
               define 'LINUX'
               compiler_option '-fno-strict-aliasing'
         end

      when 'mswin'
         define 'WINNT'
         target_ext '.pyd'
         lib_path "#{ENV['PYTHONDIR']}/libs"
         lib "#{ENV['PYTHONLIB']}"
         lib 'wsock32'
         lib 'Advapi32'
         lib 'User32'
         lib 'Ws2_32'
         lib 'ole32'
      else
         raise "#{toolset.tag('target_os')} platform is not supported."
   end

   cpp_sources Dir.glob('../libpyaf/*.cpp' )
}
