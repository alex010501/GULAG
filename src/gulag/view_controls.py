from PySide6.QtCore import QEvent, QPointF, QRectF, QSize, Qt, Signal
from PySide6.QtGui import QColor, QPainter, QPainterPath, QPainterPathStroker, QPolygonF, QRegion, QTransform
from PySide6.QtWidgets import (
    QAbstractButton, QFrame, QHBoxLayout, QMenu, QToolButton, QVBoxLayout,
)

from .icons import icon


def view_name(direction):
    x, y, z = direction
    return " / ".join(label for label in (
        {1: "Top", -1: "Bottom"}.get(z),
        {1: "Rear", -1: "Front"}.get(y),
        {1: "Right", -1: "Left"}.get(x),
    ) if label)


class CubeButton(QAbstractButton):
    """A keyboard-accessible target with a polygonal mouse hit area."""

    def __init__(self, parent, shape, hit_area, color_role):
        super().__init__(parent)
        self.shape = shape
        self.color_role = color_role
        bounds = hit_area.boundingRect().toAlignedRect()
        self.setGeometry(bounds)
        polygon = hit_area.toFillPolygon().toPolygon().translated(-bounds.topLeft())
        self.setMask(QRegion(polygon))
        self.setCheckable(True)
        self.setFocusPolicy(Qt.FocusPolicy.TabFocus)
        self.setCursor(Qt.CursorShape.PointingHandCursor)

    def paintEvent(self, event):
        # The parent paints the continuous map, including seams under these hit areas.
        pass

    def event(self, event):
        result = super().event(event)
        if event.type() in (QEvent.Type.Enter, QEvent.Type.Leave, QEvent.Type.FocusIn, QEvent.Type.FocusOut):
            self.parentWidget().update()
        return result


def _seam(start, end, width):
    line = QPainterPath(start)
    line.lineTo(end)
    stroke = QPainterPathStroker()
    stroke.setWidth(width)
    stroke.setCapStyle(Qt.PenCapStyle.FlatCap)
    return stroke.createStroke(line)


class ViewCube(QFrame):
    """Five facets with shared edge strips and vertex targets on each hemisphere."""

    directionSelected = Signal(tuple)

    def __init__(self, parent, colors):
        super().__init__(parent)
        self.colors = colors
        self.setObjectName("viewCube")
        self.hemisphere = 1
        self.active = None
        self.cells = []
        self.caps = {}
        self.setFixedSize(128, 151)
        layout = QVBoxLayout(self)
        layout.setContentsMargins(5, 0, 5, 4)
        layout.addStretch()
        center = QPainterPath()
        center.addRect(QRectF(39, 39, 50, 50))
        self._region((0, 0, 1), center, center, "cube_center", "T")
        top = QPainterPath()
        top.addPolygon(QPolygonF([QPointF(x, y) for x, y in
                                  ((12, 5), (116, 5), (119.5, 8.5), (89, 39),
                                   (39, 39), (8.5, 8.5))]))
        top.closeSubpath()
        sides = ((0, 1, "B", "cube_rear"), (1, 0, "R", "cube_right"),
                 (0, -1, "F", "cube_front"), (-1, 0, "L", "cube_left"))
        # Faces are behind the wider, invisible edge hit strips. Corner dots
        # are created last so a shared endpoint always selects the three-face view.
        seams, corners = [], []
        for quarter, (x, y, label, color) in enumerate(sides):
            turn = QTransform().translate(64, 64).rotate(90 * quarter).translate(-64, -64)
            face = turn.map(top)
            self._region((x, y, 0), face, face, color, label)
            a, b = turn.map(QPointF(39, 39)), turn.map(QPointF(89, 39))
            seams.append(((x, y, 1), a, b))
            diagonal = (x + y, y - x, 0)
            seams.append((diagonal, b, turn.map(QPointF(119.5, 8.5))))
            corners.append(((diagonal[0], diagonal[1], 1), b))
        for direction, start, end in seams:
            self._region(direction, _seam(start, end, 4), _seam(start, end, 12), "cube_seam")
        for direction, point in corners:
            dot, hit = QPainterPath(), QPainterPath()
            dot.addEllipse(point, 5.4, 5.4)
            hit.addEllipse(point, 8.5, 8.5)
            self._region(direction, dot, hit, None)

        switch = QHBoxLayout()
        switch.setSpacing(2)
        layout.addLayout(switch)
        for z, label in ((1, "Top"), (-1, "Bottom")):
            button = QToolButton()
            button.setText(label)
            button.setToolTip(f"Show {label.lower()} view and its adjacent edges and corners")
            button.setCheckable(True)
            button.setFixedHeight(21)
            button.clicked.connect(lambda checked=False, cap=z: self.directionSelected.emit((0, 0, cap)))
            self.caps[z] = button
            switch.addWidget(button)
        self.refresh((1, -1, 1))

    def _region(self, direction, shape, hit_area, color, label=""):
        x, y, cap = direction
        button = CubeButton(self, shape, hit_area, color)
        button.setText(label)
        button.clicked.connect(lambda checked=False: self.directionSelected.emit((x, y, self.hemisphere if cap else 0)))
        self.cells.append((button, x, y, cap))

    def paintEvent(self, event):
        super().paintEvent(event)
        painter = QPainter(self)
        painter.setRenderHint(QPainter.RenderHint.Antialiasing)
        buttons = [button for button, *_ in self.cells]
        targeted = next((b for b in buttons if b.underMouse()), None)
        if targeted is None:
            targeted = next((b for b in buttons if b.hasFocus()), None)
        font = self.font()
        font.setBold(True)
        font.setPixelSize(15)
        painter.setFont(font)
        for button in buttons:
            role = button.color_role
            if button is targeted:
                role = "cube_hover"
            elif button.isChecked() and targeted is None:
                role = "cube_active"
            if role:
                painter.fillPath(button.shape, QColor(self.colors[role]))
            if button.text():
                ink = "cube_text" if role == button.color_role else "cube_on_accent"
                painter.setPen(QColor(self.colors[ink]))
                painter.drawText(button.shape.boundingRect(), Qt.AlignmentFlag.AlignCenter, button.text())
        painter.end()

    def refresh(self, direction):
        if direction is None and self.active is None:
            return
        self.active = direction
        if direction and direction[2]:
            self.hemisphere = direction[2]
        for z, button in self.caps.items():
            button.setChecked(z == self.hemisphere)
        for button, x, y, cap in self.cells:
            vector = (x, y, self.hemisphere if cap else 0)
            name = view_name(vector)
            if not x and not y:
                button.setText("T" if self.hemisphere > 0 else "Btm")
            button.setToolTip(name)
            button.setAccessibleName(name)
            button.setChecked(vector == direction)
        self.update()


class QuickTools(QFrame):
    def __init__(self, view):
        super().__init__(view)
        self.setObjectName("quickTools")
        layout = QVBoxLayout(self)
        layout.setContentsMargins(4, 4, 4, 4)
        layout.setSpacing(3)
        for symbol, title in (("select", "Box selection"), ("measure", "Measurement tools"),
                              ("projection", "Orthographic projection"),
                              ("cube", "Display mode"), ("home", "Restore isometric view")):
            button = QToolButton()
            button.setFixedSize(32, 32)
            button.setIconSize(QSize(18, 18))
            button.setAccessibleName(title)
            button.setToolTip(title)
            layout.addWidget(button)
            if symbol == "projection":
                button.setDefaultAction(view.projection_action)
                continue
            button.setIcon(icon(symbol))
            if symbol == "select":
                button.setCheckable(True)
                button.setChecked(True)
                button.clicked.connect(lambda: self.selection.setChecked(True))
                self.selection = button
            elif symbol == "home":
                button.clicked.connect(view.reset_view)
            else:
                menu = QMenu(button)
                if symbol == "cube":
                    shaded = menu.addAction("Shaded")
                    shaded.setCheckable(True)
                    shaded.setChecked(True)
                    shaded.setEnabled(False)
                    menu.addAction("Wireframe").setEnabled(False)
                    menu.addSeparator()
                    menu.addAction(view.grid_action)
                else:
                    menu.addAction("Measure distance").setEnabled(False)
                button.setMenu(menu)
                button.setPopupMode(QToolButton.ToolButtonPopupMode.InstantPopup)
