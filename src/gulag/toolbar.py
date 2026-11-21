from PySide6.QtCore import QLocale, QSize, Qt
from PySide6.QtWidgets import (
    QComboBox, QDoubleSpinBox, QFrame, QHBoxLayout, QLabel, QMenu, QScrollArea, QStackedWidget,
    QTabBar, QToolButton, QVBoxLayout, QWidget,
)

from .icons import icon


class Ribbon(QWidget):
    """Page selector and grouped tool panels above the workbench."""

    def __init__(self, window, scene_actions):
        super().__init__(window)
        self.setObjectName("ribbon")
        layout = QVBoxLayout(self)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.setSpacing(0)
        header = QFrame()
        header.setObjectName("appHeader")
        header.setFixedHeight(44)
        top = QHBoxLayout(header)
        top.setContentsMargins(15, 0, 10, 0)
        top.setSpacing(8)
        logo = QLabel()
        logo.setAccessibleName("Application logo")
        logo.setPixmap(window.windowIcon().pixmap(26, 26))
        top.addWidget(logo)
        top.addSpacing(5)
        self.tabs = QTabBar()
        self.tabs.setObjectName("ribbonTabs")
        self.tabs.setDrawBase(False)
        top.addWidget(self.tabs)
        top.addStretch()
        for symbol, text, slot in (("settings", "Settings", window.open_settings),
                                   ("help", "About / Help", window.open_help)):
            button = QToolButton()
            button.setText(text)
            button.setAccessibleName(text)
            button.setToolTip(text)
            button.setToolButtonStyle(Qt.ToolButtonStyle.ToolButtonTextBesideIcon)
            button.setIcon(icon(symbol))
            button.clicked.connect(slot)
            top.addWidget(button)
        layout.addWidget(header)
        self.pages = QStackedWidget()
        self.pages.setFixedHeight(93)
        layout.addWidget(self.pages)
        self.tabs.currentChanged.connect(self.pages.setCurrentIndex)

        workspace = self._page("Workspace")
        for title, tools in (
            ("Document", (("new", "New"), ("open", "Open"), ("save", "Save"), ("saveAs", "Save as"))),
            ("History", (("undo", "Undo"), ("redo", "Redo"))),
            ("Clipboard", (("cut", "Cut"), ("copy", "Copy"), ("paste", "Paste"))),
        ):
            group = self._group(workspace, title)
            for symbol, caption in tools:
                self._button(group, symbol, caption)
        group = self._group(workspace, "Scene")
        for action in scene_actions:
            self._tool_button(group).setDefaultAction(action)
        group = self._group(workspace, "Plots")
        self._button(group, "scope", "Scope", window.add_scope)
        group = self._group(workspace, "Layout")
        panels_menu = QMenu(self)
        for dock in window.panels:
            panels_menu.addAction(dock.toggleViewAction())
        panels_menu.addSeparator()
        panels_menu.addAction("Restore default layout", window.restore_default_layout)
        panels = self._button(group, "panels", "Panels", menu=panels_menu)
        panels.setObjectName("panelsButton")

        sim = self._page("Simulation")
        group = self._group(sim, "Playback")
        self._button(group, "play", "Play")
        self._button(group, "stop", "Stop")
        group = self._group(sim, "Timing")
        frequency = QComboBox()
        frequency.setObjectName("frequency")
        frequency.addItems(["25", "50", "100", "250", "500", "1000"])
        duration = QDoubleSpinBox()
        duration.setObjectName("duration")
        duration.setLocale(QLocale.c())
        duration.setDecimals(1)
        duration.setRange(0.1, 120.0)
        duration.setValue(10.0)
        duration.setFrame(False)
        duration.setButtonSymbols(QDoubleSpinBox.ButtonSymbols.NoButtons)
        for title, field, units in (("Frequency", frequency, "Hz"), ("Duration", duration, "s")):
            label = QLabel(title)
            label.setBuddy(field)
            field.setAccessibleName(title)
            field.setFixedSize(78, 28)
            group.addWidget(label)
            group.addWidget(field)
            group.addWidget(QLabel(units))
            group.addSpacing(12)
        self.tabs.setCurrentIndex(0)

    def _page(self, title):
        content = QWidget()
        content.setObjectName("ribbonPage")
        row = QHBoxLayout(content)
        row.setAlignment(Qt.AlignmentFlag.AlignLeft)
        row.setContentsMargins(10, 4, 10, 4)
        row.setSpacing(0)
        scroll = QScrollArea()
        scroll.setFrameShape(QFrame.Shape.NoFrame)
        scroll.setWidgetResizable(True)
        scroll.setVerticalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAlwaysOff)
        scroll.setWidget(content)
        self.pages.addWidget(scroll)
        self.tabs.addTab(title)
        return row

    @staticmethod
    def _group(page, title):
        frame = QFrame()
        frame.setObjectName("toolGroup")
        column = QVBoxLayout(frame)
        column.setContentsMargins(8, 1, 8, 0)
        column.setSpacing(0)
        tools = QHBoxLayout()
        tools.setSpacing(3)
        column.addLayout(tools, 1)
        label = QLabel(title)
        label.setObjectName("groupLabel")
        label.setAlignment(Qt.AlignmentFlag.AlignCenter)
        column.addWidget(label)
        page.addWidget(frame)
        return tools

    @staticmethod
    def _tool_button(group):
        button = QToolButton()
        button.setIconSize(QSize(24, 24))
        button.setToolButtonStyle(Qt.ToolButtonStyle.ToolButtonTextUnderIcon)
        group.addWidget(button)
        return button

    def _button(self, group, symbol, text, slot=None, *, menu=None):
        button = self._tool_button(group)
        button.setText(text)
        button.setIcon(icon(symbol, size=24))
        if menu is not None:
            button.setMenu(menu)
            button.setPopupMode(QToolButton.ToolButtonPopupMode.InstantPopup)
        elif slot is None:
            button.setEnabled(False)
            button.setToolTip(f"{text} · backend connection required")
        else:
            button.clicked.connect(slot)
        return button
