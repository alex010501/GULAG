from types import SimpleNamespace

import numpy as np
from PySide6.QtCore import QPoint, QPointF, QSettings, Qt
from PySide6.QtGui import QIcon, QWheelEvent
from PySide6.QtWidgets import (
    QApplication, QComboBox, QDialogButtonBox, QDoubleSpinBox, QFileDialog, QGroupBox,
    QInputDialog, QLabel, QMessageBox, QTabBar, QToolButton,
)
import pytest

from gulag import viewport, window
from gulag.appearance import saved_theme
from gulag.navigation import BUILTINS, Bindings, InputDialog, NavigationProfiles


@pytest.fixture
def workbench(qapp, qtbot, monkeypatch, tmp_path):
    # Exercise real camera transforms and Qt input without requiring a GPU in CI.
    monkeypatch.setattr(viewport.gfx, "WgpuRenderer", lambda canvas: SimpleNamespace())
    monkeypatch.setattr(viewport.SceneView, "request_draw", lambda *args: None)
    window.configure_appearance(qapp)
    editor = window.MainWindow(QSettings(str(tmp_path / "workbench.ini"), QSettings.Format.IniFormat))
    qtbot.addWidget(editor)
    editor.resize(1920, 991)
    editor.show()
    return editor


def test_workspace_layout_and_ribbon(workbench):
    area = Qt.DockWidgetArea
    assert workbench.dockWidgetArea(workbench.libraryDock) == area.LeftDockWidgetArea
    assert workbench.dockWidgetArea(workbench.sceneDock) == area.RightDockWidgetArea
    assert workbench.dockWidgetArea(workbench.consoleDock) == area.BottomDockWidgetArea
    assert workbench.sceneDock.geometry().bottom() < workbench.propertiesDock.y()
    assert workbench.tabifiedDockWidgets(workbench.propertiesDock) == [workbench.plotsDock]
    dock_tabs = next(t for t in workbench.findChildren(QTabBar) if t is not workbench.ribbon.tabs and t.isVisible())
    assert [dock_tabs.tabText(i) for i in range(dock_tabs.count())] == ["Properties", "Plots"]
    assert dock_tabs.currentIndex() == 0
    assert workbench.view.width() > workbench.width() * 0.6
    assert workbench.view.height() > workbench.consoleDock.height()
    assert workbench.entities.topLevelItem(0).text(0) == "New scene root"
    assert workbench.entityDetails.text() == ""
    assert workbench.windowTitle() == "USSR"
    expected_logo = QIcon(":/gulag/Icons/USSR_Icon.ico").pixmap(26, 26).toImage()
    assert not expected_logo.isNull()
    assert workbench.windowIcon().pixmap(26, 26).toImage() == expected_logo
    labels = workbench.ribbon.findChildren(QLabel)
    assert all(label.text() != "GULAG" for label in labels)
    assert next(label for label in labels if label.accessibleName() == "Application logo").pixmap().toImage() == expected_logo

    frequency = workbench.findChild(QComboBox, "frequency")
    tabs = workbench.ribbon.tabs
    assert [tabs.tabText(i) for i in range(tabs.count())] == ["Workspace", "Simulation"]
    assert not frequency.isVisible()
    tabs.setCurrentIndex(1)
    assert frequency.isVisible()
    assert [frequency.itemText(i) for i in range(frequency.count())] == [
        "25", "50", "100", "250", "500", "1000"
    ]
    duration = workbench.findChild(QDoubleSpinBox, "duration")
    assert (duration.minimum(), duration.value(), duration.maximum()) == (0.1, 10.0, 120.0)
    assert duration.lineEdit().width() >= duration.fontMetrics().horizontalAdvance("120.0")

    workbench.resize(880, 560)
    assert 0 < workbench.view.width() < 880


def test_console_contents_survive_closing_panel(workbench, qtbot, monkeypatch, tmp_path):
    workbench.command.setText("пример")
    qtbot.keyClick(workbench.command, Qt.Key.Key_Return)
    workbench.consoleDock.close()
    panels = workbench.ribbon.findChild(QToolButton, "panelsButton").menu()
    next(action for action in panels.actions() if action.text() == "Console").trigger()
    assert workbench.consoleDock.isVisible()
    assert workbench.command.text() == ""
    assert workbench.log.toPlainText().endswith("-[COMMAND]-пример")

    destination = tmp_path / "console.log"
    monkeypatch.setattr(QFileDialog, "getSaveFileName", lambda *args: (str(destination), ""))
    workbench.saveLog.click()
    assert destination.read_text(encoding="utf-8") == workbench.log.toPlainText()


def test_about_reads_current_resource_and_bindings_on_each_open(workbench, monkeypatch, tmp_path):
    resource = tmp_path / "about.html"
    monkeypatch.setattr(window, "ABOUT_FILE", resource)
    displayed = []
    monkeypatch.setattr(QMessageBox, "about", lambda parent, title, text: displayed.append(text))
    resource.write_text("<p>Перемещение · ${pan}</p>", encoding="utf-8")
    workbench.open_help()
    resource.write_text("<p>Профиль · ${pan} / ${orbit} / ${zoom}</p>", encoding="utf-8-sig")
    workbench.profiles.active = "Touchpad"
    workbench.open_help()
    b = BUILTINS["Touchpad"]
    assert displayed == [f"<p>Перемещение · {BUILTINS['Mouse'].pan}</p>",
                         f"<p>Профиль · {b.pan} / {b.orbit} / {b.zoom}</p>"]
    warnings = []
    monkeypatch.setattr(QMessageBox, "warning", lambda parent, title, text: warnings.append(text))
    resource.write_bytes(b"\xff")
    workbench.open_help()
    assert str(resource) in warnings[0]


def test_custom_dock_layout_persists_and_resets(workbench, qtbot):
    left = Qt.DockWidgetArea.LeftDockWidgetArea
    workbench.removeDockWidget(workbench.plotsDock)
    workbench.addDockWidget(left, workbench.plotsDock)
    workbench.tabifyDockWidget(workbench.libraryDock, workbench.plotsDock)
    workbench.plotsDock.show()
    workbench.plotsDock.raise_()
    workbench.sceneDock.setFloating(True)
    workbench.consoleDock.close()
    workbench.close()

    restored = window.MainWindow(QSettings(workbench.profiles.settings.fileName(), QSettings.Format.IniFormat))
    qtbot.addWidget(restored)
    restored.show()
    assert restored.dockWidgetArea(restored.plotsDock) == left
    assert restored.tabifiedDockWidgets(restored.plotsDock) == [restored.libraryDock]
    assert restored.sceneDock.isFloating()
    assert restored.consoleDock.isHidden()
    panels = restored.ribbon.findChild(QToolButton, "panelsButton").menu()
    next(action for action in panels.actions() if action.text() == "Restore default layout").trigger()
    assert restored.tabifiedDockWidgets(restored.propertiesDock) == [restored.plotsDock]
    assert not restored.sceneDock.isFloating()
    assert restored.consoleDock.isVisible()


def test_scene_tree_hierarchy_and_plots(workbench, monkeypatch):
    groups = {label.text(): label.parentWidget() for label in workbench.ribbon.findChildren(QLabel, "groupLabel")}
    scene_buttons = groups["Scene"].findChildren(QToolButton)
    assert [button.text() for button in scene_buttons] == ["Add folder", "Add object"]
    assert [button.defaultAction() for button in scene_buttons] == workbench.addEntity.menu().actions()
    assert not scene_buttons[1].isEnabled()
    for name, create in (("Manipulator", scene_buttons[0].click), ("Tool", workbench.addEntity.menu().actions()[0].trigger)):
        monkeypatch.setattr(QInputDialog, "getText", lambda *args, label=name: (label, True))
        create()
    item = workbench.entities.currentItem()
    assert item.parent().parent() is workbench.root
    assert item.parent().isExpanded()
    assert workbench.entities.visualItemRect(item).height() <= 20
    assert workbench.entityDetails.text() == "Object: Tool\nParent: Manipulator"
    workbench.plotsDock.close()
    scope, = groups["Plots"].findChildren(QToolButton)
    assert scope.text() == "Scope"
    scope.click()
    assert workbench.plotsDock.isVisible()
    assert workbench.addScope.text() == "Add Scope"
    workbench.addScope.click()
    assert [box.title() for box in workbench.plotsDock.findChildren(QGroupBox)] == ["Scope 1", "Scope 2"]


def test_selection_and_context_threshold(workbench, qtbot):
    view = workbench.view
    start = QPoint(200, 100)
    before = view.camera.world.matrix.copy()
    qtbot.mousePress(view, Qt.MouseButton.LeftButton, pos=start)
    for offset in (QPoint(65, 35), QPoint(-65, -35)):
        qtbot.mouseMove(view, start + offset)
        assert view.selection_box.isVisible()
        assert view.selection_box.forward == (offset.x() > 0)
    qtbot.mouseRelease(view, Qt.MouseButton.LeftButton, pos=start + offset)
    assert not view.selection_box.isVisible()
    np.testing.assert_array_equal(before, view.camera.world.matrix)

    qtbot.mousePress(view, Qt.MouseButton.RightButton, pos=start)
    qtbot.mouseRelease(view, Qt.MouseButton.RightButton, pos=start + QPoint(1, 0))
    assert view.context_menu.isVisible()
    view.context_menu.close()
    qtbot.mousePress(view, Qt.MouseButton.RightButton, pos=start)
    qtbot.mouseMove(view, start + QPoint(45, 12))
    qtbot.mouseMove(view, start)
    qtbot.mouseRelease(view, Qt.MouseButton.RightButton, pos=start)
    assert not view.context_menu.isVisible()


@pytest.mark.parametrize("name,button,mod", [
    ("Mouse", Qt.MouseButton.MiddleButton, Qt.KeyboardModifier.NoModifier),
    ("Touchpad", Qt.MouseButton.RightButton, Qt.KeyboardModifier.ShiftModifier),
])
def test_navigation_presets(workbench, qtbot, name, button, mod):
    view = workbench.view
    view.bindings = BUILTINS[name]
    start, end = QPoint(180, 80), QPoint(215, 100)
    before = view.camera.world.forward.copy()
    qtbot.mousePress(view, button, mod, pos=start)
    qtbot.mouseMove(view, end)
    qtbot.mouseRelease(view, button, mod, pos=end)
    assert np.linalg.norm(view.pivot) > 0
    np.testing.assert_allclose(before, view.camera.world.forward)
    pivot = view.pivot.copy()
    qtbot.mousePress(view, Qt.MouseButton.RightButton, pos=start)
    qtbot.mouseRelease(view, Qt.MouseButton.RightButton, pos=end)
    assert not np.allclose(before, view.camera.world.forward)
    np.testing.assert_allclose(pivot, view.pivot)
    assert not view.context_menu.isVisible()

    for angle, pixels in ((120, 0), (0, 16)):
        before_zoom = view.camera.zoom
        event = QWheelEvent(QPointF(start), QPointF(view.mapToGlobal(start)), QPoint(0, pixels),
                            QPoint(0, angle), Qt.MouseButton.NoButton, Qt.KeyboardModifier.NoModifier,
                            Qt.ScrollPhase.ScrollUpdate, False)
        QApplication.sendEvent(view, event)
        assert view.camera.zoom > before_zoom
    np.testing.assert_allclose(pivot, view.pivot)


@pytest.mark.parametrize("orthographic", [True, False])
def test_flat_view_cube_reaches_every_direction(workbench, qtbot, orthographic):
    view = workbench.view
    pivot = np.array([0.6, 0.2, -0.4])
    view.pivot = pivot.copy()
    view.camera.zoom = 1.8
    view.projection_action.setChecked(orthographic)
    radius = np.linalg.norm(view.camera.local.position - pivot)
    seen = set()
    for hemisphere in (1, -1):
        view.cube.caps[hemisphere].click()
        for radius_px, z in ((25, hemisphere), (44, 0)):
            for x in (-1, 0, 1):
                for y in (-1, 0, 1):
                    if (x, y, z) == (0, 0, 0):
                        continue
                    # Points on the actual diagram, including the shared seams
                    # and their endpoints. Qt chooses the target by its hit mask.
                    point = QPoint(64 + x * radius_px, 64 - y * radius_px)
                    button = view.cube.childAt(point)
                    qtbot.mouseClick(button, Qt.MouseButton.LeftButton, pos=button.mapFrom(view.cube, point))
                    direction = (x, y, z)
                    assert view.cube.active == direction
                    np.testing.assert_allclose(view.camera.world.forward, -np.array(direction) / np.linalg.norm(direction), atol=1e-7)
                    assert np.linalg.norm(view.camera.local.position - pivot) == pytest.approx(radius)
                    assert view.camera.zoom == 1.8
                    assert (view.camera.fov == 0) == orthographic
                    np.testing.assert_allclose(view.pivot, pivot)
                    seen.add(direction)
    assert len(seen) == 26


def test_projection_switch_preserves_framing_and_changes_depth(workbench, qtbot):
    view = workbench.view
    camera = view.camera
    camera.set_view_size(view.width(), view.height())
    start, end = QPoint(210, 160), QPoint(250, 180)
    for button in (Qt.MouseButton.MiddleButton, Qt.MouseButton.RightButton):
        qtbot.mousePress(view, button, pos=start)
        qtbot.mouseRelease(view, button, pos=end)
    view._zoom(0.7)
    pivot = view.pivot.copy()
    pose = camera.world.matrix.copy()
    zoom = camera.zoom
    sample = pivot + 1.3 * camera.world.right + 0.6 * camera.world.up
    farther = sample + camera.world.forward * 2.0

    def screen_xy(point):
        clip = camera.camera_matrix @ np.append(point, 1)
        return clip[:2] / clip[3]

    on_pivot_plane = screen_xy(sample)
    np.testing.assert_allclose(screen_xy(farther), on_pivot_plane)
    toggle = next(b for b in view.quick_tools.findChildren(QToolButton) if b.defaultAction() is view.projection_action)
    qtbot.mouseClick(toggle, Qt.MouseButton.LeftButton)
    assert camera.fov > 0 and not toggle.isChecked()
    assert view.view_label.text().endswith("Perspective") and view.cube.active is None
    np.testing.assert_allclose(screen_xy(sample), on_pivot_plane)
    assert np.linalg.norm(screen_xy(farther)) < np.linalg.norm(on_pivot_plane)
    np.testing.assert_array_equal(camera.world.matrix, pose)
    np.testing.assert_array_equal(view.pivot, pivot)
    assert camera.zoom == zoom

    qtbot.mouseClick(toggle, Qt.MouseButton.LeftButton)
    assert toggle.isChecked() and camera.fov == 0
    assert view.view_label.text().endswith("Orthographic")
    np.testing.assert_allclose(screen_xy(farther), on_pivot_plane)
    qtbot.mouseClick(toggle, Qt.MouseButton.LeftButton)
    view.reset_view()
    assert camera.fov > 0 and not toggle.isChecked()


def test_custom_profile_persists_only_after_ok(workbench, qtbot, monkeypatch):
    profiles = workbench.profiles
    dialog = InputDialog(profiles, workbench)
    qtbot.addWidget(dialog)
    monkeypatch.setattr(QInputDialog, "getText", lambda *args: ("CAD mouse", True))
    dialog._copy_profile()
    dialog.fields["pan"].setCurrentText("Right drag")
    assert not dialog.buttons.button(QDialogButtonBox.StandardButton.Ok).isEnabled()
    dialog.fields["pan"].setCurrentText("Alt + Middle drag")
    dialog.fields["zoom"].setCurrentText("Ctrl + Wheel")
    assert profiles.active == "Mouse" and not profiles.custom
    dialog.accept()
    restored = NavigationProfiles(QSettings(profiles.settings.fileName(), QSettings.Format.IniFormat))
    assert restored.active == "CAD mouse"
    assert restored.bindings == Bindings("Alt + Middle drag", "Right drag", "Ctrl + Wheel")
    canceled = InputDialog(restored, workbench)
    qtbot.addWidget(canceled)
    canceled.fields["orbit"].setCurrentText("Shift + Right drag")
    canceled.reject()
    assert restored.bindings.orbit == "Right drag"


def test_theme_dialog_preserves_scene_and_restores_choice(workbench, qtbot, monkeypatch):
    settings = workbench.profiles.settings
    view = workbench.view
    view.projection_action.setChecked(False)
    view.set_view((-1, 1, -1))
    view.camera.zoom = 2
    pose = view.camera.world.matrix.copy()
    workbench.consoleDock.hide()
    assert saved_theme(settings) == "System"

    def choose(mode, confirm):
        def exec_dialog(dialog):
            assert [dialog.theme.itemText(i) for i in range(dialog.theme.count())] == ["System", "Light", "Dark"]
            assert dialog.theme.currentText() == saved_theme(settings)
            dialog.theme.setCurrentText(mode)
            if confirm:
                dialog.accept()
            else:
                dialog.reject()
            return dialog.result()
        monkeypatch.setattr(InputDialog, "exec", exec_dialog)
        workbench.open_settings()

    choose("Light", True)
    light_face = view.cube.grab().toImage().pixelColor(45, 49)
    light_floor = view.ground_material.color.hex
    choose("Dark", False)
    assert saved_theme(settings) == "Light" and not workbench.appearance.dark
    choose("Dark", True)
    assert saved_theme(settings) == "Dark" and workbench.appearance.dark
    assert view.cube.grab().toImage().pixelColor(45, 49).lightness() < light_face.lightness()
    assert view.ground_material.color.hex != light_floor
    np.testing.assert_array_equal(view.camera.world.matrix, pose)
    assert view.cube.active == (-1, 1, -1) and view.camera.zoom == 2 and view.camera.fov > 0
    assert workbench.consoleDock.isHidden()

    workbench.close()
    reopened = window.MainWindow(QSettings(settings.fileName(), QSettings.Format.IniFormat))
    qtbot.addWidget(reopened)
    assert reopened.appearance.mode == "Dark" and reopened.appearance.dark
    assert reopened.view.ground_material.color.hex == view.ground_material.color.hex


def test_system_theme_tracks_color_scheme(workbench, qapp):
    hints = qapp.styleHints()
    appearance = workbench.appearance
    for scheme, expected in ((Qt.ColorScheme.Dark, True), (Qt.ColorScheme.Light, False),
                             (Qt.ColorScheme.Unknown, False)):
        hints.colorSchemeChanged.emit(scheme)
        assert appearance.dark is expected
        assert workbench.view.cube.colors == appearance.colors
    appearance.set_mode("Dark")
    hints.colorSchemeChanged.emit(Qt.ColorScheme.Light)
    assert appearance.dark
    appearance.set_mode("System")
    assert appearance.dark == (hints.colorScheme() == Qt.ColorScheme.Dark)
