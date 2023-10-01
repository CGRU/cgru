from ConfigParser import ConfigParser
import json
import logging
import os
import time
from logging.handlers import TimedRotatingFileHandler
from textwrap import dedent, wrap

import af
import afcommon
import lwsdk
import lwsdk.pris
import lwsdk.pris.layout.cs

__lwver__ = "11"


class AfanasySubmitter(lwsdk.IGeneric):
    """
    A simple submitter interface for CGRU's Afanasy render farm manager.

    Config settings are saved alongside the user's LighWave configs.
    """

    def __init__(self, context):
        super(AfanasySubmitter, self).__init__()

        # Setup logging
        log_dir = os.path.join(
            lwsdk.LWDirInfoFunc(lwsdk.LWFTYPE_SETTING), "afanasy_submitter_logs"
        )
        log_file = os.path.join(log_dir, "log.log")
        if not os.path.exists(log_dir):
            os.makedirs(log_dir)
        logging.basicConfig(level=logging.DEBUG)
        self.log = logging.getLogger(__name__)
        file_handler = TimedRotatingFileHandler(log_file)
        file_handler.setFormatter(
            logging.Formatter(
                fmt="%(levelname)-8s : %(asctime)s - %(message)s",
                datefmt="%Y-%m-%d %H:%M:%S",
            )
        )
        file_handler.setLevel(logging.DEBUG)
        self.log.handlers = []
        self.log.addHandler(file_handler)
        self.log.debug("Logger initalized")

        self._ctl_width = 75

    def create_job(self, scene):
        """
        Build the command to be executed

        returns:
            A tuple containing the success and data returned from the server

        """

        self.log.info("Creating afanasy job")

        filename = os.path.normpath(scene.filename)
        filename_dir, filename_tail = os.path.split(filename)
        filename_stem, filename_ext = os.path.splitext(filename_tail)
        temp_filename = os.path.join(
            filename_dir,
            "{}_{}{}".format(
                filename_stem, str(time.time()).replace(".", "_"), filename_ext
            ),
        )

        if lwsdk.pris.layout.cs.savescenecopy(temp_filename) == 0:
            self.log.critical("Could not save scene copy to {}".format(temp_filename))
            lwsdk.pris.error("Could not save scene copy to {}".format(temp_filename))

        cmd = [
            "lwsn_af.cmd",
            "-3",
            '-c"{}"'.format(self.config_dir_ctl.get_str()),
            '-d"{}"'.format(lwsdk.LWDirInfoFunc(lwsdk.LWFTYPE_CONTENT)),
            '"{}"'.format(temp_filename),
            "@#@",
            "@#@",
            str(self.frame_step_ctl.get_int()),
        ]

        # If this is a "rez" configured environment supply the same environment to
        # the render command
        if "REZ_USED_RESOLVE" in os.environ:
            cmd = ["rez-env", os.environ["REZ_USED_RESOLVE"], "--"] + cmd

        self.log.debug("Using command: {}".format(" ".join(cmd)))

        job = af.Job(self.job_name_ctl.get_str())
        job.setBranch(self.branch_ctl.get_str())
        # job.setFolder("input", )
        # job.setNativeOS()

        block = af.Block("lightwave", "lightwave")
        block.setParser("lightwave")
        block.setCommand(" ".join(cmd))
        block.setNumeric(
            self.frame_start_ctl.get_int(),
            self.frame_end_ctl.get_int(),
            self.frames_per_task_ctl.get_int(),
        )
        block.setWorkingDirectory(os.path.split(scene.filename)[0])
        block.setTaskMaxRunTime(str(self.timeout_ctl.get_int()))

        if self.images_ctl.get_str():
            images = self.images_ctl.get_str().replace("%", "%%")
            block.setFiles(
                afcommon.patternFromDigits(afcommon.patternFromStdC(images)).split(";")
            )
            job.setFolder("output", os.path.pardir(images))

        if self.host_mask_ctl.get_str():
            block.setHostsMask(self.host_mask_ctl.get_str())

        job.setPriority(str(self.priority_ctl.get_int()))
        job.setPostDeleteFiles(str(temp_filename))
        job.blocks.append(block)

        self.log.debug(str(job))
        self.log.info("Submitting")

        return job.send(verbose=True)

    def help_popup(self, ctl, data):
        self.log.debug("Creating help popup panel")
        ui = lwsdk.LWPanels()
        panel = ui.create("Afanasy submitter")
        panel.text_ctl(
            "Afanasy Submitter",
            wrap(
                dedent(
                    """
                Config settings saved to {}
                If you wish to see preview images show up in Afanasy's farm manager interface
                then you must supply an output pattern for the images. For example:
                T:\\path\\to\\output\\frame.%03d.exr

                Logs are written to the config path.
                """.format(
                        self.config_path
                    )
                )
            ),
        )
        self.log.debug("Opening help popup panel")
        panel.open(lwsdk.PANF_BLOCKING)

    def process(self, generic_access):
        ui = lwsdk.LWPanels()
        scene = lwsdk.LWSceneInfo()
        panel = ui.create("Afanasy submitter")
        config_parser = ConfigParser(allow_no_value=True)

        # Get defaults
        self.job_name = os.path.splitext(os.path.split(scene.filename)[1])[0]
        self.frame_start = scene.frameStart
        self.frame_end = scene.frameEnd
        self.frame_step = scene.frameStep
        self.af_job_py_path = os.path.join(
            os.environ["CGRU_LOCATION"], "afanasy", "python", "afjob.py"
        )

        # Read config
        self.config_path = os.path.join(
            lwsdk.LWDirInfoFunc(lwsdk.LWFTYPE_SETTING), "afanasy_submitter.cfg"
        )
        self.log.debug("Attempting to read configs from {}".format(self.config_path))
        if os.path.exists(self.config_path):
            self.log.debug("Found {}".format(self.config_path))
            config_parser.read(self.config_path)
        else:
            self.log.debug(
                "Could not find {}. Generating file".format(self.config_path)
            )
            lwsdk.pris.info(
                "Afanasy Submitter",
                "Creating default config file {}".format(self.config_path),
            )
            config_parser.add_section("defaults")
            config_parser.set("defaults", "frames_per_task", "10")
            config_parser.set("defaults", "priority", "99")
            config_parser.set(
                "defaults", "config_dir", lwsdk.LWDirInfoFunc(lwsdk.LWFTYPE_SETTING)
            )
            config_parser.set("defaults", "branch", "")
            config_parser.set("defaults", "host_mask", "")
            config_parser.set("defaults", "timeout", "999")
            config_parser.set("defaults", "images", "")

            with open(self.config_path, "w") as f:
                self.log.debug("Writing config file {}".format(self.config_path))
                config_parser.write(f)

        self.log.debug("Creating panel")

        self.job_name_ctl = panel.str_ctl("Job Name", self._ctl_width)
        self.job_name_ctl.set_str(self.job_name)
        self.frame_start_ctl = panel.int_ctl("frame_start")
        self.frame_start_ctl.set_int(self.frame_start)
        self.frame_end_ctl = panel.int_ctl("frame_end")
        self.frame_end_ctl.set_int(self.frame_end)
        self.frame_step_ctl = panel.int_ctl("frame_step")
        self.frame_step_ctl.set_int(self.frame_step)
        self.frames_per_task_ctl = panel.int_ctl("frames_per_task")
        self.frames_per_task_ctl.set_int(
            int(config_parser.get("defaults", "frames_per_task"))
        )

        self.priority_ctl = panel.slider_ctl("Priority", self._ctl_width, 1, 200)
        self.priority_ctl.set_int(int(config_parser.get("defaults", "priority")))
        self.branch_ctl = panel.str_ctl("Branch", self._ctl_width)
        self.branch_ctl.set_str(config_parser.get("defaults", "branch"))
        self.config_dir_ctl = panel.dir_ctl("Config Dir", self._ctl_width)
        self.config_dir_ctl.set_str(config_parser.get("defaults", "config_dir"))
        self.host_mask_ctl = panel.str_ctl("Hosts Mask", self._ctl_width)
        self.host_mask_ctl.set_str(config_parser.get("defaults", "host_mask"))
        self.timeout_ctl = panel.int_ctl("Timeout")
        self.timeout_ctl.set_int(int(config_parser.get("defaults", "timeout")))
        self.images_ctl = panel.file_ctl("Preview Images", self._ctl_width)
        self.images_ctl.set_str(config_parser.get("defaults", "images"))

        self.save_settings_ctl = panel.button_ctl("Save Settings")
        self.save_settings_ctl.set_event(self.save_settings, config_parser)
        self.help_ctl = panel.button_ctl("Help")
        self.help_ctl.set_event(self.help_popup)

        self.log.debug("Opening panel")

        if panel.open(lwsdk.PANF_BLOCKING | lwsdk.PANF_CANCEL):
            success, data = self.create_job(scene)

            if not success != 0:
                self.log.error("Submission failed")
                lwsdk.pris.error("Submission failed")
            else:
                self.log.info("Submission succeeded")
                self.log.debug("Output:\n{}".format(json.dumps(data)))
                lwsdk.pris.info("Submission succeeded", json.dumps(data))
        else:
            self.log.info("Submission cancelled")

        ui.destroy(panel)
        return lwsdk.AFUNC_OK

    def save_settings(self, ctl, config_parser):
        self.log.debug("Saving config file {}".format(self.config_path))
        lwsdk.pris.info(
            "Afanasy Submitter", "Saving config file {}".format(self.config_path)
        )

        config_parser.set("defaults", "priority", self.priority_ctl.get_int())
        config_parser.set("defaults", "config_dir", self.config_dir_ctl.get_str())
        config_parser.set("defaults", "host_mask", self.host_mask_ctl.get_str())
        config_parser.set("defaults", "timeout", self.timeout_ctl.get_int())
        config_parser.set("defaults", "images", self.images_ctl.get_str())

        with open(self.config_path, "w") as f:
            config_parser.write(f)
            self.log.info("Configs saved to {}".format(self.config_path))


ServerTagInfo = [
    ("Afanasy Submitter", lwsdk.SRVTAG_USERNAME | lwsdk.LANGID_USENGLISH),
    ("Afanasy Submit", lwsdk.SRVTAG_BUTTONNAME | lwsdk.LANGID_USENGLISH),
    ("Utilities/Python", lwsdk.SRVTAG_MENU | lwsdk.LANGID_USENGLISH),
]

ServerRecord = {
    lwsdk.GenericFactory("LW_AfanasySubmitter", AfanasySubmitter): ServerTagInfo
}
