"""Persistent input profiles for the 3D viewport."""

from dataclasses import asdict, dataclass
import json

from PySide6.QtCore import QSettings, Qt
from PySide6.QtWidgets import (
    QComboBox, QDialog, QDialogButtonBox, QFormLayout, QHBoxLayout, QInputDialog,
    QLabel, QPushButton, QVBoxLayout,
)

from .appearance import THEME_KEY, THEME_MODES, saved_theme


@dataclass(frozen=True)
class Bindings:
    pan: str
    orbit: str
    zoom: str

    def valid(self):
        return (self.pan in DRAGS and self.orbit in DRAGS and self.zoom in ZOOMS
                and len({self.pan, self.orbit, self.zoom}) == 3)


MODIFIERS = {
    Qt.KeyboardModifier.NoModifier: "",
    Qt.KeyboardModifier.ShiftModifier: "Shift + ",
    Qt.KeyboardModifier.ControlModifier: "Ctrl + ",
    Qt.KeyboardModifier.AltModifier: "Alt + ",
}
DRAGS = [prefix + button for prefix in MODIFIERS.values() for button in ("Middle drag", "Right drag")]
ZOOMS = [prefix + "Wheel" for prefix in MODIFIERS.values()] + DRAGS
BUILTINS = {
    "Mouse": Bindings("Middle drag", "Right drag", "Wheel"),
    "Touchpad": Bindings("Shift + Right drag", "Right drag", "Wheel"),
}


def gesture(trigger, modifiers):
    prefix = MODIFIERS.get(modifiers)
    return prefix + trigger if prefix is not None else None


class NavigationProfiles:
    key = "viewport/inputProfiles"

    def __init__(self, settings=None):
        self.settings = settings if settings is not None else QSettings("GULAG", "Workbench")
        self.custom = {}
        self.active = "Mouse"
        try:
            data = json.loads(self.settings.value(self.key, "{}"))
        except (TypeError, ValueError):
            return
        if not isinstance(data, dict):
            return
        profiles = data.get("profiles", {})
        if isinstance(profiles, dict):
            for name, fields in profiles.items():
                if (name.strip() and name not in BUILTINS and isinstance(fields, dict)
                        and all(isinstance(fields.get(k), str) for k in ("pan", "orbit", "zoom"))):
                    bindings = Bindings(fields["pan"], fields["orbit"], fields["zoom"])
                    if bindings.valid():
                        self.custom[name] = bindings
        active = data.get("active")
        if isinstance(active, str) and active in self.all:
            self.active = active

    @property
    def all(self):
        return BUILTINS | self.custom

    @property
    def bindings(self):
        return self.all[self.active]

    def save(self, active, custom):
        payload = {"active": active, "profiles": {n: asdict(b) for n, b in custom.items()}}
        self.settings.setValue(self.key, json.dumps(payload, ensure_ascii=False))
        self.settings.sync()
        if self.settings.status() != QSettings.Status.NoError:
            raise OSError("Could not save the settings.")
        self.active, self.custom = active, custom.copy()


class InputDialog(QDialog):
    def __init__(self, profiles, parent=None):
        super().__init__(parent)
        self.profiles = profiles
        self.drafts = profiles.custom.copy()
        self.setWindowTitle("Settings")
        self.setMinimumWidth(410)
        layout = QVBoxLayout(self)
        layout.setContentsMargins(22, 20, 22, 16)
        layout.setSpacing(14)
        appearance = QLabel("Appearance")
        appearance.setObjectName("dialogHeading")
        layout.addWidget(appearance)
        appearance_form = QFormLayout()
        self.theme = QComboBox()
        self.theme.setObjectName("themeMode")
        self.theme.addItems(THEME_MODES)
        self.theme.setCurrentText(saved_theme(profiles.settings))
        appearance_form.addRow("Theme", self.theme)
        layout.addLayout(appearance_form)
        heading = QLabel("Navigation input")
        heading.setObjectName("dialogHeading")
        layout.addWidget(heading)
        form = QFormLayout()
        form.setVerticalSpacing(10)
        self.profile = QComboBox()
        self.profile.addItems(profiles.all)
        self.profile.setCurrentText(profiles.active)
        copy = QPushButton("Create preset…")
        copy.clicked.connect(self._copy_profile)
        row = QHBoxLayout()
        row.addWidget(self.profile, 1)
        row.addWidget(copy)
        form.addRow("Preset", row)
        self.fields = {}
        for key, label in (("pan", "Pan"), ("orbit", "Orbit"), ("zoom", "Zoom")):
            combo = QComboBox()
            combo.addItems(ZOOMS if key == "zoom" else DRAGS)
            self.fields[key] = combo
            form.addRow(label, combo)
            combo.currentTextChanged.connect(self._edited)
        layout.addLayout(form)
        self.hint = QLabel()
        self.hint.setWordWrap(True)
        layout.addWidget(self.hint)
        self.buttons = QDialogButtonBox(QDialogButtonBox.StandardButton.Ok | QDialogButtonBox.StandardButton.Cancel)
        self.buttons.accepted.connect(self.accept)
        self.buttons.rejected.connect(self.reject)
        layout.addWidget(self.buttons)
        self.profile.currentTextChanged.connect(self._load_profile)
        self._load_profile(profiles.active)

    def _load_profile(self, name):
        bindings = (BUILTINS | self.drafts)[name]
        for key, combo in self.fields.items():
            combo.blockSignals(True)
            combo.setCurrentText(getattr(bindings, key))
            combo.blockSignals(False)
            combo.setEnabled(name not in BUILTINS)
        self._edited()

    def _edited(self):
        name = self.profile.currentText()
        if name not in BUILTINS:
            self.drafts[name] = Bindings(**{k: c.currentText() for k, c in self.fields.items()})
        valid = all(b.valid() for b in self.drafts.values())
        self.buttons.button(QDialogButtonBox.StandardButton.Ok).setEnabled(valid)
        if not valid:
            invalid = next(n for n, b in self.drafts.items() if not b.valid())
            self.hint.setText(f"Choose a different gesture for each action in {invalid}.")
        elif name in BUILTINS:
            self.hint.setText("Create a preset to change these bindings.")
        else:
            self.hint.setText("Left drag draws a selection box. Right click opens the scene menu.")

    def _copy_profile(self):
        name, ok = QInputDialog.getText(self, "Create input preset", "Preset name")
        name = name.strip()
        if not ok:
            return
        existing = BUILTINS | self.drafts
        if not name or name.casefold() in {n.casefold() for n in existing}:
            self.hint.setText("Enter a unique preset name.")
            return
        self.drafts[name] = existing[self.profile.currentText()]
        self.profile.addItem(name)
        self.profile.setCurrentText(name)

    def accept(self):
        if all(b.valid() for b in self.drafts.values()):
            try:
                # Flush the theme and navigation edits together when OK is pressed.
                self.profiles.settings.setValue(THEME_KEY, self.theme.currentText())
                self.profiles.save(self.profile.currentText(), self.drafts)
            except OSError as exc:
                self.hint.setText(str(exc))
                return
            super().accept()
