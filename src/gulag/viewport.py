import math

import numpy as np
import pygfx as gfx
from PySide6.QtCore import QEvent, QRect, Qt
from PySide6.QtGui import QAction, QColor, QIcon, QPainter, QPen
from PySide6.QtWidgets import QApplication, QGridLayout, QLabel, QMenu, QWidget
from rendercanvas.pyside6 import QRenderWidget

from .navigation import BUILTINS, gesture
from .view_controls import QuickTools, ViewCube, view_name


class SelectionBox(QWidget):
    """Transient, mouse-transparent feedback in logical viewport coordinates."""

    def __init__(self, parent):
        super().__init__(parent)
        self.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents)
        self.hide()
        self.forward = True

    def between(self, start, end):
        self.forward = end.x() >= start.x()
        self.setGeometry(QRect(start, end).normalized().intersected(self.parentWidget().rect()))
        self.show()
        self.raise_()
        self.update()

    def paintEvent(self, event):
        colors = self.parentWidget().colors
        color = QColor(colors["box_forward" if self.forward else "box_reverse"])
        painter = QPainter(self)
        pen = QPen(color, 1)
        if not self.forward:
            pen.setStyle(Qt.PenStyle.DashLine)
        painter.setPen(pen)
        color.setAlpha(48)
        painter.setBrush(color)
        painter.drawRect(self.rect().adjusted(0, 0, -1, -1))
        painter.end()


class SceneView(QRenderWidget):
    """Z-up scene and the viewport's Qt input surface."""

    def __init__(self, parent, colors):
        # Bitmap presentation composites real WebGPU frames with ordinary Qt
        # children on every platform, including the selection box during drags.
        super().__init__(parent=parent, present_method="bitmap", update_mode="ondemand")
        self.bindings = BUILTINS["Mouse"]
        self._button = Qt.MouseButton.NoButton
        self._dragging = False
        self._action = None
        self.setContextMenuPolicy(Qt.ContextMenuPolicy.PreventContextMenu)
        self.renderer = gfx.WgpuRenderer(self)
        # pygfx uses an orthographic projection at zero field of view.
        self.camera = gfx.PerspectiveCamera(fov=0, width=10, height=10)
        self.pivot = np.zeros(3)
        self.scene = gfx.Scene()
        self.background = gfx.Background(None, gfx.BackgroundMaterial())
        self.scene.add(self.background)
        self._add_ground()
        self.axes = {}
        for end, role in (((1, 0, 0), "axis_x"), ((0, 1, 0), "axis_y"), ((0, 0, 1), "axis_z")):
            positions = np.array([(0, 0, 0), end], dtype=np.float32)
            material = gfx.LineMaterial(thickness=2.5)
            self.axes[role] = material
            self.scene.add(gfx.Line(gfx.Geometry(positions=positions), material))
        self.origin = gfx.Mesh(gfx.sphere_geometry(0.055), gfx.MeshBasicMaterial())
        self.scene.add(self.origin)

        self.grid_action = QAction("Ground grid", self, checkable=True, checked=True)
        self.grid_action.toggled.connect(self._show_grid)
        self.projection_action = QAction(QIcon(":/gulag/Images/Projection.svg"), "Orthographic projection",
                                         self, checkable=True, checked=True)
        self.projection_action.toggled.connect(self._set_projection)
        self.context_menu = QMenu(self)
        self.context_menu.addAction("Inspect entity").setEnabled(False)
        self.context_menu.addAction("Measure distance").setEnabled(False)
        self.context_menu.addSeparator()
        self.context_menu.addAction(self.grid_action)
        self.context_menu.addAction("Restore isometric view", self.reset_view)

        overlay = QGridLayout(self)
        overlay.setContentsMargins(12, 12, 12, 12)
        self.view_label = QLabel()
        self.view_label.setObjectName("viewLabel")
        self.view_label.setAttribute(Qt.WidgetAttribute.WA_TransparentForMouseEvents)
        overlay.addWidget(self.view_label, 0, 0, Qt.AlignmentFlag.AlignTop | Qt.AlignmentFlag.AlignLeft)
        self.quick_tools = QuickTools(self)
        overlay.addWidget(self.quick_tools, 1, 0, Qt.AlignmentFlag.AlignVCenter | Qt.AlignmentFlag.AlignLeft)
        self.cube = ViewCube(self, colors)
        overlay.addWidget(self.cube, 1, 1, 2, 1, Qt.AlignmentFlag.AlignRight | Qt.AlignmentFlag.AlignBottom)
        overlay.setRowStretch(1, 1)
        overlay.setColumnStretch(1, 1)
        self.cube.directionSelected.connect(self.set_view)
        self.selection_box = SelectionBox(self)
        self.apply_theme(colors)
        self.reset_view()
        self.request_draw(lambda: self.renderer.render(self.scene, self.camera))

    def _add_ground(self):
        self.ground = gfx.Group()
        self.ground_material = gfx.MeshBasicMaterial(side="both")
        surface = gfx.Mesh(gfx.plane_geometry(10, 10), self.ground_material)
        surface.local.z = -0.008
        self.grid_material = gfx.GridMaterial(
            infinite=False, major_step=1, minor_step=0.25, major_thickness=1,
            minor_thickness=0.65,
        )
        grid = gfx.Grid(None, self.grid_material, orientation="xy")
        grid.local.scale = (10, 10, 1)
        grid.local.position = (-5, -5, -0.003)
        self.ground.add(surface, grid)
        self.scene.add(self.ground)

    def apply_theme(self, colors):
        self.colors = colors
        self.background.material.set_colors(colors["sky_bottom"], colors["sky_top"])
        self.ground_material.color = colors["ground"]
        self.grid_material.major_color = colors["grid_major"]
        self.grid_material.minor_color = colors["grid_minor"]
        for role, material in self.axes.items():
            material.color = colors[role]
        self.origin.material.color = colors["origin"]
        self.cube.colors = colors
        self.cube.update()
        self.selection_box.update()
        self.request_draw()

    def _show_grid(self, visible):
        self.ground.visible = visible
        self.request_draw()

    def reset_view(self):
        self.pivot[:] = 0
        self.camera.zoom = 1
        self.camera.local.position = (8, -8, 8)
        self._set_projection(self.projection_action.isChecked())
        self.set_view((1, -1, 1))

    def _set_projection(self, orthographic):
        if orthographic:
            self.camera.fov = 0
        else:
            # Match the width and height of the old frame on the plane through
            # the orbit pivot. Camera pose, pan scale and zoom stay continuous.
            distance = np.linalg.norm(self.camera.local.position - self.pivot)
            half_extent = (self.camera.width + self.camera.height) / 4
            self.camera.fov = math.degrees(2 * math.atan2(half_extent, distance))
        next_mode = "perspective" if orthographic else "orthographic"
        self.projection_action.setToolTip(f"Switch to {next_mode} projection")
        self._update_view_label()
        self.request_draw()

    def _update_view_label(self):
        direction = view_name(self.cube.active) if self.cube.active else "Custom view"
        projection = "Perspective" if self.camera.fov else "Orthographic"
        self.view_label.setText(f"{direction}   ·   {projection}")

    def _orient(self, direction):
        distance = np.linalg.norm(self.camera.local.position - self.pivot)
        self.camera.local.position = self.pivot + direction * distance
        # A vertical sightline needs an in-plane screen-up vector.
        if abs(direction[2]) > 1 - 1e-10:
            self.camera.world.reference_up = (0, np.sign(direction[2]), 0)
        else:
            self.camera.world.reference_up = (0, 0, 1)
        self.camera.look_at(self.pivot)
        self.request_draw()

    def set_view(self, direction):
        self._orient(np.array(direction) / np.linalg.norm(direction))
        self.cube.refresh(direction)
        self._update_view_label()

    def _navigate(self, delta):
        dx, dy = delta.x(), delta.y()
        if self._action == "pan":
            units = self.camera.height / (max(1, min(self.width(), self.height())) * self.camera.zoom)
            offset = (self.camera.world.up * dy - self.camera.world.right * dx) * units
            self.pivot += offset
            self.camera.local.position = self.camera.local.position + offset
        elif self._action == "orbit":
            x, y, z = self.camera.local.position - self.pivot
            horizontal = math.hypot(x, y)
            yaw = (math.atan2(y, x) if horizontal > 1e-7 else -math.pi / 2) - dx * 0.007
            limit = math.pi / 2 - 0.001
            pitch = np.clip(math.atan2(z, horizontal) + dy * 0.007, -limit, limit)
            planar = math.cos(pitch)
            self._orient(np.array([math.cos(yaw) * planar, math.sin(yaw) * planar, math.sin(pitch)]))
            self.cube.refresh(None)
            self._update_view_label()
        elif self._action == "zoom":
            self._zoom(-dy / 100)
        self.request_draw()

    def _zoom(self, amount):
        self.camera.zoom = float(np.clip(self.camera.zoom * 2 ** np.clip(amount, -8, 8), 0.01, 1000))
        self.request_draw()

    def mousePressEvent(self, event):
        if self._button != Qt.MouseButton.NoButton:
            event.accept()
            return
        self.setFocus()
        self._button = event.button()
        self._start = self._last = event.position().toPoint()
        self._dragging = False
        triggers = {Qt.MouseButton.MiddleButton: "Middle drag", Qt.MouseButton.RightButton: "Right drag"}
        binding = gesture(triggers.get(self._button, ""), event.modifiers())
        self._action = next((name for name in ("pan", "orbit", "zoom") if getattr(self.bindings, name) == binding), None)
        if self._button == Qt.MouseButton.LeftButton:
            self._action = "select"
        event.accept()

    def _move(self, point):
        if not self._dragging:
            if (point - self._start).manhattanLength() < QApplication.startDragDistance():
                return
            self._dragging = True
        if self._action == "select":
            self.selection_box.between(self._start, point)
        elif point != self._last and self._action:
            self._navigate(point - self._last)
        self._last = point

    def mouseMoveEvent(self, event):
        if self._button != Qt.MouseButton.NoButton:
            self._move(event.position().toPoint())
        event.accept()

    def mouseReleaseEvent(self, event):
        if event.button() == self._button:
            self._move(event.position().toPoint())
            menu = self._button == Qt.MouseButton.RightButton and not self._dragging
            self._cancel_gesture()
            if menu:
                self.context_menu.popup(event.globalPosition().toPoint())
        event.accept()

    def wheelEvent(self, event):
        if self._button == Qt.MouseButton.NoButton and gesture("Wheel", event.modifiers()) == self.bindings.zoom:
            pixels = event.pixelDelta().y()
            self._zoom(pixels / 160 if pixels else event.angleDelta().y() / 480)
        event.accept()

    def _cancel_gesture(self):
        self._button = Qt.MouseButton.NoButton
        self._action = None
        self._dragging = False
        self.selection_box.hide()

    def event(self, event):
        if event.type() in (QEvent.Type.WindowDeactivate, QEvent.Type.UngrabMouse, QEvent.Type.Hide):
            if hasattr(self, "selection_box"):
                self._cancel_gesture()
        return super().event(event)

    def keyPressEvent(self, event):
        if event.key() == Qt.Key.Key_Escape:
            self._cancel_gesture()
        else:
            super().keyPressEvent(event)
