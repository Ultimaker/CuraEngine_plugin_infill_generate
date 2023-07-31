import os
import sys
import stat
import platform
from pathlib import Path

from UM.i18n import i18nCatalog
from UM.Logger import Logger
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

    def getPort(self):
        return super().getPort() if not self.isDebug() else int(os.environ["CURAENGINE_INFILL_GENERATE_PORT"])

    def isDebug(self):
        return not hasattr(sys, "frozen") and os.environ.get("CURAENGINE_INFILL_GENERATE_PORT", None) is not None

    def start(self):
        if not self.isDebug():
            super().start()

    def binaryPath(self) -> Path:
        ext = ".exe" if platform.system() == "Windows" else ""
        return Path(__file__).parent.joinpath({ "AMD64": "x86_64", "x86_64": "x86_64" }.get(platform.machine()), platform.system(), f"curaengine_plugin_infill_generate{ext}").resolve()
