def main() -> int:
    """Start the GULAG desktop workbench."""
    import sys

    from PySide6.QtWidgets import QApplication

    from .window import MainWindow

    app = QApplication(sys.argv)
    window = MainWindow()
    window.showMaximized()
    return app.exec()
