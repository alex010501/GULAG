"""Original toolbar artwork and complementary colored workbench symbols."""

from functools import lru_cache

from PySide6.QtCore import QRectF, Qt
from PySide6.QtGui import QColor, QFont, QFontDatabase, QIcon, QPainter, QPixmap


ARTWORK = {
    "new": "New", "open": "Open", "save": "Save", "saveAs": "SaveAs", "undo": "Undo",
    "redo": "Redo", "cut": "Cut", "copy": "Copy", "paste": "Paste", "play": "Play", "stop": "Stop",
}
GLYPHS = {
    "settings": (0xf013, "#f0ba67"), "help": (0xf059, "#72caff"),
    "folder": (0xf07b, "#d79a30"), "scope": (0xf201, "#129878"),
    "select": (0xf245, "#267dc0"), "measure": (0xf545, "#b17d20"),
    "cube": (0xf1b2, "#4398c3"), "home": (0xf015, "#358776"),
    "console": (0xf120, "#298e91"), "library": (0xf02d, "#5c7bbb"),
    "scene": (0xf0e8, "#508ba8"), "properties": (0xf1de, "#9473b0"),
    "plus": (0xf067, "#338565"), "panels": (0xf009, "#678bc2"),
}


@lru_cache(maxsize=1)
def _family():
    font_id = QFontDatabase.addApplicationFont(":/gulag/Fonts/FA6_Free-Solid.otf")
    return QFontDatabase.applicationFontFamilies(font_id)[0]


@lru_cache(maxsize=128)
def icon(name, color=None, size=18):
    result = QIcon()
    if name in ARTWORK:
        original = QPixmap(f":/gulag/Images/{ARTWORK[name]}.png")
        result.addPixmap(original)
        # Keep the asset's colors while giving unavailable commands less emphasis.
        faded = QPixmap(original.size())
        faded.fill(Qt.GlobalColor.transparent)
        painter = QPainter(faded)
        painter.setOpacity(0.66)
        painter.drawPixmap(0, 0, original)
        painter.end()
        result.addPixmap(faded, QIcon.Mode.Disabled)
        return result
    glyph, ink = GLYPHS[name]
    for mode, opacity in ((QIcon.Mode.Normal, 1.0), (QIcon.Mode.Disabled, 0.66)):
        pixmap = QPixmap(size * 3, size * 3)
        pixmap.fill(Qt.GlobalColor.transparent)
        pixmap.setDevicePixelRatio(3)
        painter = QPainter(pixmap)
        font = QFont(_family())
        font.setPixelSize(size - 2)
        painter.setFont(font)
        painter.setOpacity(opacity)
        painter.setPen(QColor(color or ink))
        painter.drawText(QRectF(0, 0, size, size), Qt.AlignmentFlag.AlignCenter, chr(glyph))
        painter.end()
        result.addPixmap(pixmap, mode)
    return result
