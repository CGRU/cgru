# -*- coding: utf-8 -*-
"""CGRU/Afanasy Fusion job submission module.
"""
import time

import af
import afcommon
import logging
import os

import fusionscript


logger = logging.getLogger(__name__)


class RenderCommandBuilder(object):
    """Generates render commands."""

    @classmethod
    def build(cls, job_data):
        """Build and return render command."""
        if "REZ_USED_REQUEST" in os.environ:
            # this is a rez configured environment
            # use the same rez request to build the render command
            logger.debug("Using REZ_USED_REQUEST to build the render command")
            cmd_buffer = ["rez-env {} -- fusion".format(os.environ["REZ_USED_REQUEST"])]
        else:
            # use the default command
            cmd_buffer = ["fusion"]

        # Fusion [<filename.comp> | <filename.dfq>] [-config] [-manager] [-quiet]
        # [-clean] [-about] [-render [-frames <frameset>] [-start <frame>]
        # [-end <frame>] [-step <step>] [-quit]] [-status] [-join <host>]
        # [-log <filename>] [-cleanlog] [-verbose] [-quietlicense] [-version]
        # [-pri high | above | normal | below | idle] [-args [...]]
        # [-execute <script string>]

        cmd_buffer.append(job_data["comp_file_full_path"])
        cmd_buffer.append("-quiet")
        cmd_buffer.append("-render")
        cmd_buffer.append("-start @#@")
        cmd_buffer.append("-end @#@")
        cmd_buffer.append("-verbose")

        return " ".join(cmd_buffer)


class Dispatcher(object):
    """Dispatches the render task."""

    def __init__(self):
        self.fusion = fusionscript.scriptapp("Fusion")
        self.comp = self.fusion.GetCurrentComp()

    def get_all_saver_nodes(self, skip_disabled=False):
        """Return all saver nodes.

        Args:
            skip_disabled (bool): Skip disabled nodes.

        Returns:
            list: All saver nodes.
        """
        # get all non disabled savers
        all_saver_nodes = self.comp.GetToolList(False, "Saver").values()
        if skip_disabled:
            all_saver_nodes = list(
                filter(
                    lambda x: x.GetAttrs("TOOLB_PassThrough") is True, all_saver_nodes
                )
            )
        return all_saver_nodes

    @classmethod
    def generate_timestamp(cls):
        """Return a timestamp."""
        return "{}{}".format(
            time.strftime("%y%m%d-%H%M%S-"),
            "{:0.5f}".format(time.time()).split(".")[-1],
        )

    def dispatch(self, job_data):
        """Dispatch job to Afanasy.

        Args:
            job_data (dict): Job data.
        """
        job_name = job_data["Job Name"]
        job = af.Job(job_name)

        block = af.Block("main", "fusion")
        start_frame = job_data["start_frame"]
        end_frame = job_data["last_frame"]
        frames_per_task = job_data["frames_per_task"]
        by_frame = job_data["by_frame"]
        block.setNumeric(
            int(start_frame), int(end_frame), int(frames_per_task), int(by_frame)
        )

        if job_data["submit_each_saver_individually"]:
            # get all non disabled savers
            all_saver_nodes = self.get_all_saver_nodes(skip_disabled=True)
            # for each saver node
            # disable all the other savers node
            # and duplicate the current comp file

        command = RenderCommandBuilder.build(job_data)
        block.setCommand(command)

        # job.setCmdPost('deletefiles -s "%s"' % os.path.abspath(filename))
        # if pause:
        #     job.offline()
        job.blocks.append(block)
        status, data = job.send()
        if not status:
            raise RuntimeError("Something went wrong!")


class UI(object):
    """Submission UI."""

    def __init__(self):
        self.fusion = fusionscript.scriptapp("Fusion")
        self.comp = self.fusion.GetCurrentComp()
        self.dialog = None
        self.setup_ui()

    def message_box(self, title, message):
        """Show message box."""
        self.comp.AskUser(
            title,
            {
                1: "Message",
                2: "Text",
                "Name": "Message",
                "ReadOnly": True,
                "Default": message,
                "Lines": 2,
                "Wrap": True,
            },
        )

    def setup_ui(self):
        attributes = self.comp.GetAttrs()
        comp_full_path = attributes["COMPS_FileName"]
        comp_file_name = os.path.basename(comp_full_path)
        first_frame = attributes["COMPN_RenderStartTime"]
        last_frame = attributes["COMPN_RenderEndTime"]
        message = "Comp is ready for submission"

        dialog = {
            1: {
                1: "job_name",
                2: "Text",
                "Name": "Job Name",
                "Lines": 1,
                "Wrap": False,
                "Default": comp_file_name.split(".")[0],
            },
            2: {
                1: "start_frame",
                2: "Slider",
                "Name": "Start Frame",
                "Default": first_frame,
                "SoftMax": last_frame,
                "Integer": True,
            },
            3: {
                1: "last_frame",
                2: "Slider",
                "Name": "Last Frame",
                "SoftMax": 2 * last_frame,
                "Default": last_frame,
                "Integer": True,
            },
            4: {
                1: "by_frame",
                2: "Slider",
                "Name": "By Frame",
                "Default": 1,
                "Integer": True,
            },
            5: {
                1: "frames_per_task",
                2: "Slider",
                "Name": "Frames Per Task",
                "Default": 5,
                "Min": 1,
                "Integer": True,
            },
            6: {
                1: "hosts_mask",
                2: "Text",
                "Name": "Hosts Mask",
                "Lines": 1,
                "Wrap": False,
                "Default": "",
            },
            7: {
                1: "submit_each_saver_individually",
                2: "Checkbox",
                "Name": "Submit Each Saver Individually",
                "Default": 1,
            },
            8: {
                1: "status",
                2: "Text",
                "Name": "Status",
                "Lines": 1,
                "Wrap": False,
                "Default": message,
                "ReadOnly": True,
            },
        }

        result = self.comp.AskUser("Submit to Afanasy", dialog)
        if not result:
            return

        result.update(
            {"comp_file_full_path": comp_full_path, "comp_file_name": comp_file_name}
        )

        dispatcher = Dispatcher()
        try:
            dispatcher.dispatch(result)
        except RuntimeError as e:
            self.message_box("Submit to Afanasy", "{}".format(e))
        else:
            self.message_box("Submit to Afanasy", "Successfully Submitted.")


UI()
