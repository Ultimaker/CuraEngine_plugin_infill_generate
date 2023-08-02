import os
import sys
import stat
import platform
from pathlib import Path

from UM.i18n import i18nCatalog
from UM.Logger import Logger
from UM.Settings.ContainerRegistry import ContainerRegistry
from UM.Settings.DefinitionContainer import DefinitionContainer
from cura.BackendPlugin import BackendPlugin

catalog = i18nCatalog("cura")

class CuraEngineInfillGenerate(BackendPlugin):
    def __init__(self):
        super().__init__()
        if not self.isDebug():
            if not self.binaryPath().exists():
                Logger.error(f"Could not find CuraEngineInfillGenerate binary at {self.binaryPath().as_posix()}")
            if platform.system() != "Windows" and self.binaryPath().exists():
                st = os.stat(self.binaryPath())
                os.chmod(self.binaryPath(), st.st_mode | stat.S_IEXEC)

            self._plugin_command = [self.binaryPath().as_posix()]

        self._supported_slots = [200]  # ModifyPostprocess SlotID
        ContainerRegistry.getInstance().containerLoadComplete.connect(self._on_container_load_complete)

    def _on_container_load_complete(self, container_id) -> None:
        if not ContainerRegistry.getInstance().isLoaded(container_id):
            # skip containers that could not be loaded, or subsequent findContainers() will cause an infinite loop
            return
        try:
            container = ContainerRegistry.getInstance().findContainers(id = container_id)[0]
        except IndexError:
            # the container no longer exists
            return
        if not isinstance(container, DefinitionContainer):
            # skip containers that are not definitions
            return
        if container.getMetaDataEntry("type") == "extruder":
            # skip extruder definitions
            return
        for infill_patterns in ("infill_pattern", "support_pattern", "support_interface_pattern", "support_roof_pattern", "support_bottom_pattern", "roofing_pattern", "top_bottom_pattern", "ironing_pattern"):
            for definition in container.findDefinitions(key = infill_patterns):
                definition.extend_category("logo", "UM Logo", plugin_id=self.getPluginId(), plugin_version=self.getVersion())

    def getPort(self):
        return super().getPort() if not self.isDebug() else int(os.environ["CURAENGINE_INFILL_GENERATE_PORT"])

    def isDebug(self):
        return not hasattr(sys, "frozen") and os.environ.get("CURAENGINE_INFILL_GENERATE_PORT", None) is not None

    def start(self):
        # TODO: Only start the plugin if any of the extended enums are actually used.
        if not self.isDebug():
            super().start()

    def binaryPath(self) -> Path:
        ext = ".exe" if platform.system() == "Windows" else ""
        return Path(__file__).parent.joinpath({ "AMD64": "x86_64", "x86_64": "x86_64" }.get(platform.machine()), platform.system(), f"curaengine_plugin_infill_generate{ext}").resolve()
