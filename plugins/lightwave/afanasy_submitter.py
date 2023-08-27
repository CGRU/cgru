from ConfigParser import ConfigParser
import logging
import os
import sys
from logging.handlers import TimedRotatingFileHandler
from subprocess import PIPE, Popen
from textwrap import dedent, wrap

import lwsdk

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
            lwsdk.LWMessageFuncs().info(
                "Afanasy Submitter",
                "Creating default config file {}".format(self.config_path),
            )
            config_parser.add_section("defaults")
            config_parser.set("defaults", "priority", "99")
            config_parser.set(
                "defaults", "config_dir", lwsdk.LWDirInfoFunc(lwsdk.LWFTYPE_SETTING)
            )
            config_parser.set(
                "defaults", "content_dir", lwsdk.LWDirInfoFunc(lwsdk.LWFTYPE_CONTENT)
            )
            config_parser.set(
                "defaults",
                "lwsn_path",
                "{}\\lwsn.exe".format(os.path.split(sys.executable)[0]),
            )
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

        self.priority_ctl = panel.slider_ctl("Priority", self._ctl_width, 1, 200)
        self.priority_ctl.set_int(int(config_parser.get("defaults", "priority")))
        self.config_dir_ctl = panel.dir_ctl("Config Dir", self._ctl_width)
        self.config_dir_ctl.set_str(config_parser.get("defaults", "config_dir"))
        self.content_dir_ctl = panel.str_ctl("Content Dir", self._ctl_width)
        self.content_dir_ctl.set_str(config_parser.get("defaults", "content_dir"))
        self.lwsn_path_ctl = panel.file_ctl("lwsn_path", self._ctl_width)
        self.lwsn_path_ctl.set_str(config_parser.get("defaults", "lwsn_path"))
        self.host_mask_ctl = panel.str_ctl("host_mask", self._ctl_width)
        self.host_mask_ctl.set_str(config_parser.get("defaults", "host_mask"))
        self.timeout_ctl = panel.int_ctl("timeout")
        self.timeout_ctl.set_int(int(config_parser.get("defaults", "timeout")))
        self.images_ctl = panel.file_ctl("Preview Images", self._ctl_width)
        self.images_ctl.set_str(config_parser.get("defaults", "images"))

        self.save_settings_ctl = panel.button_ctl("Save Settings")
        self.save_settings_ctl.set_event(self.save_settings, config_parser)
        self.help_ctl = panel.button_ctl("Help")
        self.help_ctl.set_event(self.help_popup)

        self.log.debug("Opening panel")

        if panel.open(lwsdk.PANF_BLOCKING | lwsdk.PANF_CANCEL):
            # Create lwsn wrapper to unset PYTHONHOME
            args = [
                "rez-env",
                "cgru",
                "cgru_settings",
                "--",
                "python",
                self.af_job_py_path,
                scene.filename,
                str(self.frame_start_ctl.get_int()),
                str(self.frame_end_ctl.get_int()),
                "-by",
                str(self.frame_step_ctl.get_int()),
                "-fpt",
                str(self.frame_step_ctl.get_int()),
                "-extrargs",
                '{lwsn_path} -3 -c"{config_dir}" -d"{content_dir}" "{scene_path}"'.format(
                    lwsn_path=self.lwsn_path_ctl.get_str(),
                    config_dir=self.config_dir_ctl.get_str(),
                    content_dir=self.content_dir_ctl.get_str(),
                    scene_path=scene.filename,
                ),
                "-pwd",
                os.path.split(scene.filename)[0],
                "-name",
                self.job_name_ctl.get_str(),
                "-execmaxruntime",
                str(self.timeout_ctl.get_int()),
                "-priority",
                str(self.priority_ctl.get_int()),
                "-type",
                "lightwave",
                "-exec",
                os.path.join(os.path.dirname(__file__), "lwsn.bat"),
            ]

            if self.images_ctl.get_str():
                args.extend(["-images", self.images_ctl.get_str().replace("%", "%%")])

            if self.host_mask_ctl.get_str():
                args.extend(["-hostsmask", self.host_mask_ctl.get_str()])

            self.log.debug(args)
            self.log.debug("Launching: {}".format(" ".join(args)))

            proc = Popen(args=args, universal_newlines=True, stdout=PIPE, stderr=PIPE)
            stdout, stderr = proc.communicate()
            if proc.returncode != 0:
                self.log.error(
                    "Submission failed. Process returned exit code: {}\n{}".format(
                        proc.returncode, stderr
                    )
                )
                lwsdk.LWMessageFuncs().error(
                    "Submission failed",
                    "Process returned exit code: {}\n{}".format(
                        proc.returncode, stderr
                    ),
                )
            else:
                self.log.info("Submission succeeded")
                self.log.debug("Output:\n{}".format(stdout))
        else:
            self.log.info("Submission cancelled")

        ui.destroy(panel)
        return lwsdk.AFUNC_OK

    def save_settings(self, ctl, config_parser):
        self.log.debug("Saving config file {}".format(self.config_path))
        lwsdk.LWMessageFuncs().info(
            "Afanasy Submitter", "Saving config file {}".format(self.config_path)
        )

        config_parser.set("defaults", "priority", self.priority_ctl.get_int())
        config_parser.set("defaults", "config_dir", self.config_dir_ctl.get_str())
        config_parser.set("defaults", "content_dir", self.content_dir_ctl.get_str())
        config_parser.set("defaults", "lwsn_path", self.lwsn_path_ctl.get_str())
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
