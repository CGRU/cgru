#!/usr/bin/env ruby

require 'mxx_ru/cpp/qt4_via_pkg_config'

Mxx_ru::Cpp::exe_target {

    target "afwatch"

    required_prj "libafqt.mxx.rb"

    qt = generator( Mxx_ru::Cpp::Qt4.new( self ) )
    qt.use_modules QT_GUI, QT_CORE, QT_NETWORK, QT_XML

    qt.moc_result_subdir = "../project.mxxru/tmp/watch/moc"
    qt.hpp_ext = ".h"
    qt.h2moc "../watch/actionid.h"
    qt.h2moc "../watch/buttonmonitor.h"
    qt.h2moc "../watch/colorwidget.h"
    qt.h2moc "../watch/ctrlrenders.h"
    qt.h2moc "../watch/ctrlsortfilter.h"
    qt.h2moc "../watch/dialog.h"
    qt.h2moc "../watch/filewidget.h"
    qt.h2moc "../watch/fontwidget.h"
    qt.h2moc "../watch/labelversion.h"
    qt.h2moc "../watch/listitems.h"
    qt.h2moc "../watch/listjobs.h"
    qt.h2moc "../watch/listmonitors.h"
    qt.h2moc "../watch/listnodes.h"
    qt.h2moc "../watch/listrenders.h"
    qt.h2moc "../watch/listtalks.h"
    qt.h2moc "../watch/listtasks.h"
    qt.h2moc "../watch/listusers.h"
    qt.h2moc "../watch/modelnodes.h"
    qt.h2moc "../watch/numberwidget.h"
    qt.h2moc "../watch/wndcustomizegui.h"

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
            linker_option "/SUBSYSTEM:WINDOWS"
        else
            raise "#{toolset.tag('target_os')} platform is not supported."
    end

    cpp_sources Dir.glob('../watch/*.cpp')
}
