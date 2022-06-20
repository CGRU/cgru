# -*- coding: utf-8 -*-
"""CGRU/Afanasy Fusion job submission module.
"""
import copy
import json
import tempfile
import logging
import os
import time

import af
import afcommon

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
            cmd_buffer = ["rez-env {} -- fusion_render_node".format(os.environ["REZ_USED_REQUEST"])]
        else:
            # use the default command
            cmd_buffer = ["fusion_render_node"]

        # Fusion [<filename.comp> | <filename.dfq>] [-config] [-manager] [-quiet]
        # [-clean] [-about] [-render [-frames <frameset>] [-start <frame>]
        # [-end <frame>] [-step <step>] [-quit]] [-status] [-join <host>]
        # [-log <filename>] [-cleanlog] [-verbose] [-quietlicense] [-version]
        # [-pri high | above | normal | below | idle] [-args [...]]
        # [-execute <script string>]

        cmd_buffer.append(job_data["comp_file_full_path"])
        cmd_buffer.append("-render")
        cmd_buffer.append("-start @#@")
        cmd_buffer.append("-end @#@")
        cmd_buffer.append("-verbose")
        cmd_buffer.append("-quiet")
        cmd_buffer.append("-quietlicense")
        cmd_buffer.append("-clean")
        cmd_buffer.append("-quit")

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
                    lambda x: x.GetAttrs("TOOLB_PassThrough") is False, all_saver_nodes
                )
            )
        return all_saver_nodes

    @classmethod
    def generate_timestamp(cls):
        """Return a timestamp.

        Returns:
            str: The timestamp.
        """
        return "{}{}".format(
            time.strftime("%y%m%d-%H%M%S-"),
            "{:0.5f}".format(time.time()).split(".")[-1],
        )

    def generate_new_file_path_for_saver(
        self, saver_node, original_file_path="", postfix=""
    ):
        """Generate a new file path for the given saver.

        Args:
            saver_node (node): A Saver node.
            original_file_path (str): Original file path.
            postfix (str): Any postfix

        Returns:
            str: The new file path.
        """
        comp_full_path = original_file_path
        if not comp_full_path:
            # use the current file path.
            attributes = self.comp.GetAttrs()
            comp_full_path = attributes["COMPS_FileName"]
        path, ext = os.path.splitext(comp_full_path)
        saver_name = saver_node.GetAttrs("TOOLS_Name")
        # replace any empty spaces with "_"
        saver_name = saver_name.replace(" ", "_")
        if postfix:
            return_val = "{}-{}.{}{}".format(path, saver_name, postfix, ext)
        else:
            return_val = "{}-{}{}".format(path, saver_name, ext)
        return return_val

    def dispatch(self, job_data):
        """Dispatch job to Afanasy.

        Args:
            job_data (dict): Job data.
        """
        job_name = job_data["job_name"]
        job = af.Job(job_name)

        start_frame = job_data["start_frame"]
        end_frame = job_data["last_frame"]
        frames_per_task = job_data["frames_per_task"]
        by_frame = job_data["by_frame"]
        submit_paused = job_data["submit_paused"]

        timestamp = self.generate_timestamp()

        # save the current scene first
        self.comp.Save()
        if job_data["submit_each_saver_individually"]:
            # get all non disabled savers
            original_file_path = job_data["comp_file_full_path"]
            all_saver_nodes = self.get_all_saver_nodes(skip_disabled=True)
            for saver_node in all_saver_nodes:
                # disable all the other savers node
                _ = [
                    node.SetAttrs({"TOOLB_PassThrough": True})
                    for node in all_saver_nodes
                ]
                # re enabled this one
                saver_node.SetAttrs({"TOOLB_PassThrough": False})

                # and duplicate the current comp file
                saver_comp_file_path = self.generate_new_file_path_for_saver(
                    saver_node, original_file_path=original_file_path, postfix=timestamp
                )
                self.comp.Save(saver_comp_file_path)

                # create a block with this saver's name
                block = af.Block(saver_node.GetAttrs("TOOLS_Name"), "fusion")
                block.setNumeric(
                    int(start_frame),
                    int(end_frame),
                    int(frames_per_task),
                    int(by_frame),
                )
                temp_job_data = copy.copy(job_data)
                temp_job_data["comp_file_full_path"] = saver_comp_file_path
                command = RenderCommandBuilder.build(temp_job_data)
                block.setCommand(command)
                block.setCmdPost(
                    'deletefiles -s "%s"' % os.path.abspath(saver_comp_file_path)
                )
                job.blocks.append(block)

            # re-enable all the saver nodes
            _ = [
                node.SetAttrs({"TOOLB_PassThrough": False}) for node in all_saver_nodes
            ]
            # re-save it under the original filename
            self.comp.Save(original_file_path)

        if submit_paused:
            job.offline()
        status, data = job.send()
        if not status:
            raise RuntimeError("Something went wrong!")


class UI(object):
    """Submission UI."""

    settings_file_name = "Afanasy_Fusion_Job_Settings.json"

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

    def get_settings_file_path(self):
        """Return settings file path.

        Returns:
            str: Settings file path.
        """
        return os.path.join(tempfile.gettempdir(), self.settings_file_name)

    def save_settings(self, settings):
        """Save UI settings.

        Args:
            settings (dict): Settings to write.
        """
        settings_file_path = self.get_settings_file_path()
        with open(settings_file_path, "w") as f:
            json.dump(settings, f)

    def load_settings(self):
        """Load UI settings.

        Returns:
            dict: Settings dictionary.
        """
        settings_file_path = self.get_settings_file_path()
        if not os.path.isfile(settings_file_path):
            return {}

        with open(settings_file_path, "r") as f:
            settings = json.load(f)

        return settings

    def setup_ui(self):
        """Setup UI."""
        settings = self.load_settings()

        message = settings.get("message", "Comp is ready for submission")
        by_frame = settings.get("by_frame", 1)
        frames_per_task = settings.get("frames_per_task", 5)
        hosts_mask = settings.get("hosts_mask", "")
        submit_each_saver_individually = settings.get("submit_each_saver_individually", 1)
        submit_paused = settings.get("submit_paused", 1)

        attributes = self.comp.GetAttrs()
        comp_full_path = attributes["COMPS_FileName"]
        comp_file_name = os.path.basename(comp_full_path)
        first_frame = attributes["COMPN_RenderStartTime"]
        last_frame = attributes["COMPN_RenderEndTime"]

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
                "Default": by_frame,
                "Integer": True,
            },
            5: {
                1: "frames_per_task",
                2: "Slider",
                "Name": "Frames Per Task",
                "Default": frames_per_task,
                "Min": 1,
                "Integer": True,
            },
            6: {
                1: "hosts_mask",
                2: "Text",
                "Name": "Hosts Mask",
                "Lines": 1,
                "Wrap": False,
                "Default": hosts_mask,
            },
            7: {
                1: "submit_each_saver_individually",
                2: "Checkbox",
                "Name": "Submit Each Saver Individually",
                "Default": submit_each_saver_individually,
            },
            8: {
                1: "submit_paused",
                2: "Checkbox",
                "Name": "Submit Paused",
                "Default": submit_paused,
            },
            9: {
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

        # save settings
        self.save_settings(result)

        dispatcher = Dispatcher()
        try:
            dispatcher.dispatch(result)
        except RuntimeError as e:
            self.message_box("Submit to Afanasy", "{}".format(e))
        else:
            self.message_box("Submit to Afanasy", "Successfully Submitted.")


UI()
