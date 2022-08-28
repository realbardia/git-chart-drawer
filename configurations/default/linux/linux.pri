linux: !android {
    isEmpty(PREFIX): PREFIX = /usr

    SHORTCUT = git-chart-drawer

    shortcut.input = $$PWD/share/git-chart-drawer.desktop.in
    shortcut.output = $$PWD/share/git-chart-drawer.desktop
    shortcut.path = $$PREFIX/share/applications/
    shortcut.files = $$shortcut.output
    icons.path = $$PREFIX/share/icons
    icons.files = $$PWD/share/hicolor
    pixmaps.path = $$PREFIX/share/pixmaps
    pixmaps.files = $$PWD/share/git-chart-drawer.png
    target.path = $$PREFIX/bin

    QMAKE_SUBSTITUTES += shortcut

    INSTALLS += target shortcut icons pixmaps

    DISTFILES += \
        $$PWD/share/git-chart-drawer.desktop.in \
        $$PWD/share/hicolor/128x128/apps/git-chart-drawer.png \
        $$PWD/share/hicolor/16x16/apps/git-chart-drawer.png \
        $$PWD/share/hicolor/256x256/apps/git-chart-drawer.png \
        $$PWD/share/hicolor/32x32/apps/git-chart-drawer.png \
        $$PWD/share/hicolor/48x48/apps/git-chart-drawer.png \
        $$PWD/share/hicolor/64x64/apps/git-chart-drawer.png \
        $$PWD/share/hicolor/96x96/apps/git-chart-drawer.png \
        $$PWD/share/git-chart-drawer.png \
}
