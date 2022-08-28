# git-chart-drawer
Draw charts from your Git repository using Qt and QtCharts

## How to build?

It's easy like as other Qt projects. Just clone it, build it and install it:

```bash
git clone https://github.com/realbardia/git-chart-drawer.git
mkdir build && cd build
qmake -r ..
make -j4
sudo make install
```

## How to run it?

Git chart drawer contains two command-line and graphical mode.

To run graphical mode just run the binary or shortcut from the menu:

```
git-chart-drawer
```

To run from command line you could read manual using `--help` switch. It's an example usage of the command line mode:

```bash
git-chart-drawer -i /path/to/git/repo -o ~/Desktop/image.png
```

