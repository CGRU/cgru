import json
import os
import time

# https://github.com/mottosso/Qt.py
from Qt import QtCore, QtWidgets
import pyqt_clarisse

import cgrudocs
import cgruconfig
import cgruutils

#
# Create Qt application if it does not exist:
#
App = QtWidgets.QApplication.instance()
if not App: App = QtWidgets.QApplication(["Clarisse"])
#
# Process project and archive file names:
#
Filename, ext = os.path.splitext( ix.application.get_current_project_filename())
Archive = '%s.af@TIME@-%s.render' % ( Filename, cgruconfig.VARS['USERNAME'])
#
# Dialog:
#
Fields = dict()
def createDialog():

    def createLayoutH( i_args):
        h_layout = QtWidgets.QHBoxLayout()
        i_args['layout'].addLayout( h_layout)
        if 'spacing' in i_args: h_layout.addSpacing( i_args['spacing'])
        return h_layout

    def createLabel( i_args):
        label = QtWidgets.QLabel( i_args['label'])
        i_args['layout'].addWidget( label)
        label.setAlignment( QtCore.Qt.AlignRight | QtCore.Qt.AlignVCenter)
        if 'lwidth' in i_args:
            label.setFixedWidth( i_args['lwidth'])
        if 'tip' in i_args:
            label.setToolTip( i_args['tip'])

    def createField( i_args):
        i_args['layout'] = createLayoutH({'layout':i_args['layout']})
        createLabel( i_args)
        if not 'type' in i_args: i_args['type'] = 'str'
        if i_args['type'] == 'int':
            edit = QtWidgets.QSpinBox()
            if 'min' in i_args: edit.setMinimum( i_args['min'])
            else: edit.setMinimum( -1000000000)
            if 'max' in i_args: edit.setMaximum( i_args['max'])
            else: edit.setMaximum( 1000000000)
            if 'val' in i_args: edit.setValue( i_args['val'])
        else:
            edit = QtWidgets.QLineEdit()
            if 'val' in i_args: edit.setText( i_args['val'])
        if 'ewidth' in i_args: edit.setFixedWidth( i_args['ewidth'])
        i_args['layout'].addWidget( edit)
        Fields[i_args['name']] = edit

    def createButton( i_args):
        btn = QtWidgets.QPushButton( i_args['label'])
        i_args['layout'].addWidget( btn)
        if 'width' in i_args: btn.setFixedWidth( i_args['width'])
        btn.pressed.connect( i_args['slot'])
        return btn

    # Top widget:
    dialog = QtWidgets.QWidget()
    dialog.setWindowTitle('Afanasy Submit')
    dialog.setWindowIcon( QtWidgets.QIcon('%s/icons/afanasy.png' % cgruconfig.VARS['CGRU_LOCATION']))
    dialog_layout = QtWidgets.QVBoxLayout( dialog)
    tabs = QtWidgets.QTabWidget( dialog)
    dialog_layout.addWidget( tabs)

    # General Tab:
    tab = QtWidgets.QWidget( tabs)
    tabs.addTab( tab, 'General')
    tab_layout = QtWidgets.QVBoxLayout( tab)
    lwidth = 80
    rwidth = 80

    # Job name:
    h_layout = createLayoutH({'layout':tab_layout})
    createField({'name':'job_name','label':'Job Name','layout':h_layout,'val':os.path.basename( Filename),'lwidth':lwidth})
    # Engine:
    createLabel({'label':'Engine','layout':h_layout,'lwidth':lwidth})
    engines = ['clarisse_node','clarisse_render','cnode','crender']
    combo = QtWidgets.QComboBox()
    combo.setFixedWidth( 150)
    h_layout.addWidget( combo)
    for engine in engines: combo.addItem( engine)
    Fields['engine'] = combo
    # Help button:
    createButton({'label':'Help','slot':showHelp,'layout':h_layout,'width':rwidth})

    # Archive:
    h_layout = createLayoutH({'layout':tab_layout})
    createField({'name':'archive','label':'Archive','layout':h_layout,'val':Archive,'lwidth':lwidth,
            'tip':'"@TIME@" will be replaced with the current time.'})
    createButton({'label':'Browse','slot':browseArchive,'layout':h_layout,'width':rwidth})
    h_layout = createLayoutH({'layout':tab_layout,'spacing':lwidth+10})
    archive_save = QtWidgets.QCheckBox('Export Archive')
    archive_save.setToolTip('Export project file to render archive before job sending.')
    archive_save.setChecked( True)
    h_layout.addWidget( archive_save)
    Fields['archive_save'] = archive_save
    archive_del = QtWidgets.QCheckBox('Delete Archive')
    archive_del.setToolTip('Delete render archive after job deletion.')
    archive_del.setChecked( True)
    h_layout.addWidget( archive_del)
    Fields['archive_del'] = archive_del

    # Images:
    h_layout = createLayoutH({'layout':tab_layout})
    def getAllImages():
        objects = ix.api.OfObjectArray()
        ix.application.get_factory().get_all_objects("Image", objects)
        return objects
    def imageChanged( i_index = 0):
        Fields['output'].setText( getAllImages()[i_index].get_attribute('save_as').get_string())
    h_layout = createLayoutH({'layout':tab_layout})
    createLabel({'label':'Image','layout':h_layout,'lwidth':lwidth})
    combo = QtWidgets.QComboBox()
    for img in getAllImages():
        name = img.get_full_name().replace('project://','')
        combo.addItem( name)
    combo.currentIndexChanged.connect( imageChanged)
    h_layout.addWidget( combo)
    Fields['image'] = combo
    createField({'name':'format','label':'Format','layout':h_layout,'lwidth':60,'ewidth':rwidth,
            'tip':'Output images format.\nIf empty, it will not be changed.'})

    # Output:
    h_layout = createLayoutH({'layout':tab_layout})
    createField({'name':'output','label':'Output','layout':h_layout,'lwidth':lwidth})
    imageChanged()
    createButton({'label':'Browse','slot':browseOutput,'layout':h_layout,'width':rwidth})

    # Frame range:
    frange = ix.application.get_current_frame_range()
    h_layout = createLayoutH({'layout':tab_layout})
    createField({'name':'frame_first','label':'Frame First','type':'int','val':frange[0],'layout':h_layout,'lwidth':lwidth})
    createField({'name':'frame_last','label':'Frame Last','type':'int','val':frange[1],'layout':h_layout,'lwidth':lwidth})
    createField({'name':'frame_step','label':'Frame Step','type':'int','val':1,'min':1,'layout':h_layout,'lwidth':lwidth,'ewidth':rwidth})
    h_layout = createLayoutH({'layout':tab_layout,'spacing':lwidth+10})
    createField({'name':'frames_per_task','label':'Frames Per Task','type':'int','val':2,'min':1,
            'layout':h_layout})
    createField({'name':'frame_sequential','label':'Frame Sequential','type':'int','val':1,
            'layout':h_layout,'ewidth':rwidth})
    tab_layout.addStretch()

    # Settings Tab:
    tab = QtWidgets.QWidget( tabs)
    tabs.addTab( tab, 'Settings')
    tab_layout = QtWidgets.QVBoxLayout( tab)
    lwidth = 120;
    h_layout = createLayoutH({'layout':tab_layout})
    createField({'name':'priority','label':'Priority','type':'int','val':-1,'min':-1,'layout':h_layout,'lwidth':lwidth})
    createField({'name':'capacity','label':'Capacity','type':'int','val':-1,'min':-1,'layout':h_layout,'lwidth':lwidth})
    h_layout = createLayoutH({'layout':tab_layout})
    createField({'name':'max_running_tasks','label':'Max Run Tasks','type':'int','val':-1,'min':-1,'layout':h_layout,'lwidth':lwidth})
    createField({'name':'max_running_tasks_per_host','label':'Per Host','type':'int','val':-1,'min':-1,'layout':h_layout,'lwidth':lwidth})
    createField({'name':'tasks_max_run_time','label':'Max Run Time','type':'int','val':-1,'min':-1,'layout':tab_layout,'lwidth':lwidth,
            'tip':'Maximum time (in hours) that tasks are allowed to run.'})
    # Wait time:
    h_layout = createLayoutH({'layout':tab_layout})
    createLabel({'label':'Wait Time','layout':h_layout,'tip':'Job will wait this time to start.','lwidth':lwidth})
    wait_time = QtWidgets.QDateTimeEdit( QtCore.QDateTime.currentDateTime())
    wait_time.setDisplayFormat('yyyy.MM.dd hh:mm:ss')
    Fields['wait_time'] = wait_time
    h_layout.addWidget( wait_time)
    # Service/Parser:
    createField({'name':'service','label':'Service','val':'clarisse','layout':tab_layout,'lwidth':lwidth})
    createField({'name':'parser','label':'Parser','val':'clarisse','layout':tab_layout,'lwidth':lwidth})
    tab_layout.addStretch()

    # Conditions Tab:
    tab = QtWidgets.QWidget( tabs)
    tabs.addTab( tab, 'Conditions')
    tab_layout = QtWidgets.QVBoxLayout( tab)
    lwidth = 150
    createField({'name':'need_os','label':'Needed OS','layout':tab_layout,'lwidth':lwidth,
            'tip':'Job tasks will run only on clients with specified OS.'})
    createField({'name':'need_properties','label':'Needed Properties','layout':tab_layout,'lwidth':lwidth,
            'tip':'Job tasks will run only on clients with specified properties.'})
    createField({'name':'depend_mask','label':'Depend Mask','layout':tab_layout,'lwidth':lwidth,
            'tip':'Job will wait all jobs from the same user which name matches this regular expression.'})
    createField({'name':'depend_mask_global','label':'Global Depend Mask','layout':tab_layout,'lwidth':lwidth,
            'tip':'Job will wait all jobs from all users which name matches this regular expression.'})
    createField({'name':'hosts_mask','label':'Hosts Mask','layout':tab_layout,'lwidth':lwidth,
            'tip':'Job tasks will run only on hosts which name matches this regular expression.'})
    createField({'name':'hosts_mask_exclude','label':'Exclude Hosts Mask','layout':tab_layout,'lwidth':lwidth,
            'tip':'Job tasks will not run on hosts which name matches this regular expression.'})
    tab_layout.addStretch()

    # Send job buttons:
    h_layout = createLayoutH({'layout':dialog_layout})
    Fields['btn_send'] = createButton({'label':'Send Job','slot':createJob,'layout':h_layout})
    # Paused:
    paused = QtWidgets.QCheckBox('Paused')
    paused.setToolTip('Job will be sent with "OFFLINE" state.\nTasks execution is paused.')
    paused.setFixedWidth( rwidth+10)
    h_layout.addWidget( paused)
    Fields['paused'] = paused

    # Info line:
    line = QtWidgets.QLineEdit()
    dialog_layout.addWidget( line)
    line.setReadOnly( True)
    Fields['info'] = line

    dialog.show()

    return dialog

def showHelp():
	cgrudocs.showSoftware('clarisse')

def displayInfo( i_text):
    Fields['info'].setText( i_text)

def browseArchive():
    filename, fltr = QtWidgets.QFileDialog.getSaveFileName( None,'Select Archive', Fields['archive'].text())
    if filename and len(filename): Fields['archive'].setText(filename)

def browseOutput():
    filename, fltr = QtWidgets.QFileDialog.getSaveFileName( None,'Select Output', Fields['output'].text())
    if filename and len(filename): Fields['output'].setText(filename)

def processArchive():
    global Archive
    Archive = str( Fields['archive'].text())
    Archive = Archive.replace('@TIME@',time.strftime('%y%m%d%H%M%S'))
    
    if Fields['archive_save'].isChecked():
        if not checkCreateFolder( os.path.dirname( Archive),'Archive'):
            Archive = None
            return
        print('AF: Saving archive to render:')
        print( Archive)
        if not ix.application.export_render_archive( Archive):
            Archive = None

def checkCreateFolder( i_folder, i_name):
    i_folder = os.path.normpath( i_folder)
    if os.path.isdir( i_folder):
        return True
    if QtWidgets.QMessageBox.question( None,
            '%s Folder' % i_name,
            '%s\ndoes not exist, create it?' % i_folder,
            QtWidgets.QMessageBox.Abort | QtWidgets.QMessageBox.Ok ) == QtWidgets.QMessageBox.Ok:
        os.makedirs( i_folder)
        return True
    return False

def enableJobSend(): Fields['btn_send'].setEnabled( True)

def enableJobSendDelayed( i_seconds = 1): QtCore.QTimer.singleShot( i_seconds * 1000, enableJobSend)

def createJob():

    # Just to prevent double click and double job:
    Fields['btn_send'].setEnabled( False)

    # Import Afanasy Python module:
    af = None
    try:
    	af = __import__('af', globals(), locals(), [])
    except Exception as exc:
        displayInfo( exc)
        return

    # Process archive:
    processArchive()
    if Archive is None:
        displayInfo('Unable to save render archive.')
        enableJobSendDelayed()
        return

    # Check images folder:
    if not checkCreateFolder( os.path.dirname( Fields['output'].text()),'Output'):
        displayInfo('Images folder does not exist.')
        enableJobSendDelayed()
        return

    # Job object:
    job = af.Job( Fields['job_name'].text())
    job.setPriority( Fields['priority'].value())
    job.setMaxRunningTasks( Fields['max_running_tasks'].value())
    job.setMaxRunTasksPerHost( Fields['max_running_tasks_per_host'].value())
    job.setNeedOS( Fields['need_os'].text())
    job.setNeedProperties( Fields['need_properties'].text())
    job.setDependMask( Fields['depend_mask'].text())
    job.setDependMaskGlobal( Fields['depend_mask_global'].text())
    job.setHostsMask( Fields['hosts_mask'].text())
    job.setHostsMaskExclude( Fields['hosts_mask_exclude'].text())
    job.setWaitTime( Fields['wait_time'].dateTime().toTime_t())
    job.setFolder('input', os.path.dirname( Archive))
    job.setFolder('output', os.path.dirname( Fields['output'].text()))
    if Fields['archive_del'].isChecked():
        job.setPostDeleteFiles( Archive)
    if Fields['paused'].isChecked():
        job.offline()

    # Block (of tasks) object:
    block = af.Block( os.path.basename(Fields['image'].currentText()))
    block.setNumeric(
            Fields['frame_first'].value(),
            Fields['frame_last'].value(),
            Fields['frames_per_task'].value(),
            Fields['frame_step'].value())
    block.setSequential( Fields['frame_sequential'].value())
    block.setService( Fields['service'].text())
    block.setParser( Fields['parser'].text())
    block.setCapacity( Fields['capacity'].value())
    block.setTasksMaxRunTime( Fields['tasks_max_run_time'].value() * 3600)
    block.setCommand( genCommand())
    job.blocks.append( block)

    # Print job object in stdout for debug:
    job.output()

    # Send job JSON object to server:
    status, data = job.send()
    if status:
        displayInfo( json.dumps(data))
        enableJobSendDelayed()
    else:
        if Fields['archive_save'].isChecked():
            print('AF: Removing render archive.')
            os.remove( Archive)
        displayInfo('Unable send job to afserver.')

def genCommand():
    cmd = Fields['engine'].currentText()
    cmd += ' "%s"' % Archive
    cmd += ' -image "%s"' % Fields['image'].currentText()
    cmd += ' -start_frame @#@ -end_frame @#@ -frame_step %d' % Fields['frame_step'].value()
    cmd += ' -output "%s"' % Fields['output'].text()
    if len( Fields['format'].text()):
        cmd += ' -format "%s"' % Fields['format'].text()
    cmd += ' -log_width 0'
        # Zero log width disables output truncation at all.
        # This needed for correct output parsing.
    return cmd


# Create dialog:
Dialog = createDialog()
App.aboutToQuit.connect( Dialog.close)
displayInfo('CGRU Version: %s' % cgruconfig.VARS['CGRU_VERSION'])

# instead of calling app.exec_() like you would do normally in PyQt,
# you call pyqt_clarisse.exec_(app).
pyqt_clarisse.exec_(App)

