"""Light and dark workbench colors shared by Qt and the 3D viewport."""

from pathlib import Path
from string import Template

from PySide6.QtCore import QObject, Qt, Signal
from PySide6.QtGui import QColor, QPalette


THEME_KEY = "appearance/theme"
THEME_MODES = {
    "System": Qt.ColorScheme.Unknown,
    "Light": Qt.ColorScheme.Light,
    "Dark": Qt.ColorScheme.Dark,
}

# Each pair is (light, dark). The cube's seams also serve as edge targets.
COLORS = {
    "window": ("#f4f6f5", "#242b2a"),
    "panel": ("#fcfdfc", "#202725"),
    "field": ("#ffffff", "#1a201f"),
    "bar": ("#edf2ef", "#2c3532"),
    "button": ("#f7f9f8", "#303936"),
    "hover": ("#e5eee9", "#37453e"),
    "selected": ("#d7ebe1", "#274c40"),
    "line": ("#ccd6d0", "#444f49"),
    "strong_line": ("#9bada3", "#687c70"),
    "text": ("#263a31", "#e2eae5"),
    "muted": ("#52665a", "#b0bdb5"),
    "disabled": ("#718078", "#96a59c"),
    "accent": ("#26765f", "#85cdb1"),
    "on_accent": ("#ffffff", "#15372a"),
    "tooltip": ("#273936", "#dce6e0"),
    "tooltip_text": ("#f8faf9", "#203129"),
    "cube_center": ("#eff5f2", "#475755"),
    "cube_rear": ("#dbe7e1", "#596967"),
    "cube_right": ("#c9d9d1", "#4d5c5a"),
    "cube_front": ("#d3e1da", "#546360"),
    "cube_left": ("#e4ede8", "#62716f"),
    "cube_seam": ("#93a99e", "#b9c9c1"),
    "cube_text": ("#263d32", "#ffffff"),
    "cube_active": ("#007a6d", "#31baac"),
    "cube_hover": ("#009385", "#47e2d3"),
    "cube_on_accent": ("#ffffff", "#123932"),
    "sky_bottom": ("#cbd0cf", "#222a2a"),
    "sky_top": ("#e7eae8", "#353e3e"),
    "ground": ("#f0f1ec", "#394240"),
    "grid_major": ("#b2bbb8", "#677771"),
    "grid_minor": ("#d8ded9", "#4a5751"),
    "axis_x": ("#b2504b", "#ea8178"),
    "axis_y": ("#358460", "#69c796"),
    "axis_z": ("#4774af", "#81abed"),
    "origin": ("#405057", "#b8c8c0"),
    "box_forward": ("#2679d8", "#72b4ff"),
    "box_reverse": ("#157e4d", "#69d99e"),
}


def saved_theme(settings):
    mode = settings.value(THEME_KEY, "System", type=str)
    return mode if mode in THEME_MODES else "System"


class Appearance(QObject):
    changed = Signal(object)

    def __init__(self, app):
        super().__init__(app)
        self.app = app
        self.mode = "System"
        self.dark = None
        self.sheet = Template(Path(__file__).with_name("workbench.qss").read_text(encoding="utf-8"))
        app.styleHints().colorSchemeChanged.connect(self._apply_scheme)

    def set_mode(self, mode):
        self.mode = mode
        hints = self.app.styleHints()
        hints.setColorScheme(THEME_MODES[mode])
        # Some platform plugins do not support an explicit native override.
        self._apply_scheme(hints.colorScheme())

    def _apply_scheme(self, scheme):
        dark = self.mode == "Dark" or (self.mode == "System" and scheme == Qt.ColorScheme.Dark)
        if dark == self.dark:
            return
        self.dark = dark
        self.colors = {name: pair[int(dark)] for name, pair in COLORS.items()}
        palette = QPalette()
        role = QPalette.ColorRole
        for key, roles in (
            ("window", (role.Window,)), ("panel", (role.AlternateBase,)),
            ("field", (role.Base,)), ("button", (role.Button,)),
            ("text", (role.Text, role.WindowText, role.ButtonText, role.BrightText)),
            ("accent", (role.Highlight, role.Accent, role.Link)),
            ("on_accent", (role.HighlightedText,)), ("muted", (role.PlaceholderText,)),
            ("line", (role.Mid, role.Midlight)), ("strong_line", (role.Dark,)),
            ("hover", (role.Light,)), ("tooltip", (role.ToolTipBase,)),
            ("tooltip_text", (role.ToolTipText,)),
        ):
            for color_role in roles:
                palette.setColor(color_role, QColor(self.colors[key]))
        for color_role in (role.WindowText, role.Text, role.ButtonText):
            palette.setColor(QPalette.ColorGroup.Disabled, color_role, QColor(self.colors["disabled"]))
        self.app.setPalette(palette)
        self.app.setStyleSheet(self.sheet.substitute(self.colors))
        self.changed.emit(self.colors)
