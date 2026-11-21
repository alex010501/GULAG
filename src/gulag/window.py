from datetime import datetime
from html import escape
from pathlib import Path
from string import Template

from PySide6.QtCore import QByteArray, QIODevice, QResource, QSaveFile, QSize, QStandardPaths, Qt
from PySide6.QtGui import QFont, QFontDatabase, QIcon
from PySide6.QtUiTools import QUiLoader
from PySide6.QtWidgets import (
    QApplication, QFileDialog, QFrame, QGridLayout, QGroupBox, QInputDialog, QLabel,
    QDialog, QMainWindow, QMenu, QMessageBox, QTabWidget, QTreeWidgetItem,
)

from .appearance import Appearance, saved_theme
from .icons import icon
from .navigation import InputDialog, NavigationProfiles
from .toolbar import Ribbon
from .viewport import SceneView


PACKAGE = Path(__file__).resolve().parent
ABOUT_FILE = PACKAGE / "about.html"
if PACKAGE.parent.name == "src" and (PACKAGE.parent.parent / "pyproject.toml").is_file():
    ABOUT_FILE = PACKAGE.parent.parent / "resources" / "runtime" / "gulag" / "about.html"
APP_TITLE = "USSR"
APP_ICON = ":/gulag/Icons/USSR_Icon.ico"
DOCK_SETTINGS_KEY = "workspace/dockState"
DOCK_STATE_VERSION = 1


class WorkspaceLoader(QUiLoader):
    """Populate the actual main window with the dock widgets from the Designer form."""

    def __init__(self, window):
        super().__init__()
        self.window = window

    def createWidget(self, class_name, parent=None, name=""):
        if parent is None and class_name == "QMainWindow":
            self.window.setObjectName(name)
            return self.window
        return super().createWidget(class_name, parent, name)


def configure_appearance(app):
    """Initialize shared resources, typography and the application's theme controller."""
    appearance = app.findChild(Appearance)
    if appearance is not None:
        return appearance
    if not QResource(APP_ICON).isValid() and not QResource.registerResource(str(PACKAGE / "workbench.rcc")):
        raise RuntimeError("Could not load the GULAG UI resources")
    app.setStyle("Fusion")
    font = QFontDatabase.systemFont(QFontDatabase.SystemFont.GeneralFont)
    # Offscreen Qt on Windows can start with an empty font database.
    # Resolve a text face before the icon font becomes available for matching.
    if not QFontDatabase.families():
        fonts = Path(QStandardPaths.writableLocation(QStandardPaths.StandardLocation.FontsLocation))
        face = QFontDatabase.addApplicationFont(str(fonts / "segoeui.ttf"))
        if face >= 0:
            QFontDatabase.addApplicationFont(str(fonts / "seguisb.ttf"))
        else:
            face = QFontDatabase.addApplicationFont(":/gulag/Fonts/Cruinn_Regular.ttf")
        font = QFont(QFontDatabase.applicationFontFamilies(face)[0])
    font.setWeight(QFont.Weight.Normal)
    font.setPixelSize(13)
    app.setFont(font)
    app.setApplicationDisplayName(APP_TITLE)
    app.setWindowIcon(QIcon(APP_ICON))
    return Appearance(app)


class MainWindow(QMainWindow):
    """The desktop scene editor."""

    def __init__(self, settings=None):
        super().__init__()
        self.setWindowTitle(APP_TITLE)
        self.resize(1280, 720)
        self.profiles = NavigationProfiles(settings)
        self.appearance = configure_appearance(QApplication.instance())
        self.appearance.set_mode(saved_theme(self.profiles.settings))
        self._default_layout = None
        ui = WorkspaceLoader(self).load(str(PACKAGE / "workspace.ui"))
        self.setDockNestingEnabled(True)
        self.setTabPosition(Qt.DockWidgetArea.AllDockWidgetAreas, QTabWidget.TabPosition.North)
        self.setCorner(Qt.Corner.BottomLeftCorner, Qt.DockWidgetArea.LeftDockWidgetArea)
        self.setCorner(Qt.Corner.BottomRightCorner, Qt.DockWidgetArea.RightDockWidgetArea)
        self.splitDockWidget(ui.sceneDock, ui.propertiesDock, Qt.Orientation.Vertical)
        self.tabifyDockWidget(ui.propertiesDock, ui.plotsDock)
        self.panels = (ui.libraryDock, ui.sceneDock, ui.propertiesDock, ui.plotsDock, ui.consoleDock)
        for dock, symbol in zip(self.panels, ("library", "scene", "properties", "scope", "console")):
            dock.setWindowIcon(icon(symbol))
            dock.toggleViewAction().setIcon(icon(symbol))
        for layout in (ui.plotsLayout, ui.consoleLayout, ui.libraryLayout, ui.treeLayout,
                       ui.propertyLayout):
            layout.setContentsMargins(10, 10, 10, 8)
            layout.setSpacing(7)
        ui.commandRow.setSpacing(8)
        ui.command.setFixedHeight(28)
        ui.command.setPlaceholderText("Enter a command")
        ui.viewportLayout.setContentsMargins(0, 0, 0, 0)
        self.view = SceneView(ui.viewportHost, self.appearance.colors)
        self.appearance.changed.connect(self.view.apply_theme)
        ui.viewportLayout.addWidget(self.view)
        self.statusBar().setSizeGripEnabled(False)
        self._apply_profile()
        ui.treeLayout.setContentsMargins(4, 3, 4, 4)
        ui.treeLayout.setSpacing(2)
        ui.entities.setIndentation(14)
        ui.entities.setIconSize(QSize(13, 13))
        ui.entities.setUniformRowHeights(True)
        self.root = QTreeWidgetItem(ui.entities, ["New scene root"])
        self.root.setIcon(0, icon("cube"))
        self.root.setExpanded(True)
        ui.entities.currentItemChanged.connect(self._show_properties)
        entity_menu = QMenu(ui.addEntity)
        entity_menu.addAction(icon("folder", size=24), "Add folder", self.add_folder)
        add_object = entity_menu.addAction(icon("cube", size=24), "Add object")
        add_object.setEnabled(False)
        add_object.setToolTip("Object types are pending alignment with COMRaDE.")
        ui.addEntity.setIcon(icon("plus", size=14))
        ui.addScope.setIcon(icon("plus", size=14))
        ui.addEntity.setMenu(entity_menu)
        self.ribbon = Ribbon(self, entity_menu.actions())
        self.setMenuWidget(self.ribbon)
        ui.addScope.clicked.connect(self.add_scope)
        ui.command.returnPressed.connect(self._submit_command)
        ui.enter.clicked.connect(self._submit_command)
        ui.saveLog.clicked.connect(self._save_log)

    def showEvent(self, event):
        super().showEvent(event)
        if self._default_layout is None:
            self._size_default_panels()
            self.propertiesDock.raise_()
            self._default_layout = self.saveState(DOCK_STATE_VERSION)
            saved = self.profiles.settings.value(DOCK_SETTINGS_KEY)
            if isinstance(saved, QByteArray) and not self.restoreState(saved, DOCK_STATE_VERSION):
                self.restore_default_layout()
            self.view.setFocus()

    def _size_default_panels(self):
        self.resizeDocks([self.libraryDock, self.sceneDock],
                         [self.width() * 17 // 100, self.width() // 5], Qt.Orientation.Horizontal)
        self.resizeDocks([self.consoleDock], [self.height() // 6], Qt.Orientation.Vertical)
        self.resizeDocks([self.sceneDock, self.propertiesDock], [4, 6], Qt.Orientation.Vertical)

    def restore_default_layout(self):
        self.restoreState(self._default_layout, DOCK_STATE_VERSION)
        self._size_default_panels()
        self.propertiesDock.raise_()

    def closeEvent(self, event):
        state = self.saveState(DOCK_STATE_VERSION)
        super().closeEvent(event)
        if event.isAccepted() and self._default_layout is not None:
            self.profiles.settings.setValue(DOCK_SETTINGS_KEY, state)
            self.profiles.settings.sync()

    def _apply_profile(self):
        b = self.view.bindings = self.profiles.bindings
        self.statusBar().showMessage(f"{self.profiles.active}    |    Pan  ·  {b.pan}      "
                                     f"Orbit  ·  {b.orbit}      Zoom  ·  {b.zoom}")

    def open_settings(self):
        if InputDialog(self.profiles, self).exec() == QDialog.DialogCode.Accepted:
            self.appearance.set_mode(saved_theme(self.profiles.settings))
            self._apply_profile()

    def open_help(self):
        try:
            template = Template(ABOUT_FILE.read_text(encoding="utf-8-sig"))
        except (OSError, UnicodeError) as exc:
            QMessageBox.warning(self, "About / Help", f"Could not read {ABOUT_FILE}\n{exc}")
            return
        b = self.profiles.bindings
        text = template.safe_substitute(pan=escape(b.pan), orbit=escape(b.orbit), zoom=escape(b.zoom))
        QMessageBox.about(self, "About / Help", text)

    def add_folder(self):
        name, ok = QInputDialog.getText(self, "Add Folder", "Entity name")
        if ok and name.strip():
            parent = self.entities.currentItem() or self.root
            folder = QTreeWidgetItem(parent, [name.strip()])
            folder.setIcon(0, icon("folder"))
            parent.setExpanded(True)
            self.sceneDock.show()
            self.sceneDock.raise_()
            self.entities.setCurrentItem(folder)

    def _show_properties(self, item, previous):
        text = ""
        if item is not None and item.parent() is not None:
            text = f"Object: {item.text(0)}\nParent: {item.parent().text(0)}"
        self.entityDetails.setText(text)

    def add_scope(self):
        self.plotsDock.show()
        self.plotsDock.raise_()
        layout = self.plotsLayout
        scope = QGroupBox(f"Scope {layout.count() - 1}")
        scope.setMinimumHeight(120)
        axes = QGridLayout(scope)
        plot = QFrame()
        plot.setObjectName("plotArea")
        axes.addWidget(QLabel("Amplitude"), 0, 0, alignment=Qt.AlignmentFlag.AlignTop)
        axes.addWidget(plot, 0, 1)
        axes.addWidget(QLabel("Time"), 1, 1, alignment=Qt.AlignmentFlag.AlignCenter)
        axes.setColumnStretch(1, 1)
        axes.setRowStretch(0, 1)
        layout.insertWidget(layout.count() - 1, scope)

    def _submit_command(self):
        text = self.command.text().strip()
        if text:
            self.log.appendPlainText(f"[{datetime.now():%H:%M:%S}]-[COMMAND]-{text}")
            self.command.clear()

    def _save_log(self):
        filename, _ = QFileDialog.getSaveFileName(self, "Save log", "console.log", "Log (*.log);;All files (*)")
        if filename:
            file = QSaveFile(filename)
            data = self.log.toPlainText().encode("utf-8")
            if not (file.open(QIODevice.OpenModeFlag.WriteOnly) and file.write(data) == len(data)
                    and file.commit()):
                QMessageBox.warning(self, "Save log", file.errorString())
