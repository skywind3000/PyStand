# PyStand

Python 独立部署环境。Python 3.5 以后，Windows 下面都有一个 Embedded Python 的
独立 Python 运行环境，这个 PyStand 就是配合 Embedded Python 使用的。

## 特性介绍

- 使用 PyStand + PyQt5 精简版发布 PyQt 程序，打包大小只有 14MB。
- 使用 PyStand 发布普通 Python 程序，打包大小仅 5MB。

## 功能说明

- Windows 下独立 Python 环境的启动器。
- 自动加载 PyStand.exe 同级目录下面 runtime 子目录内的 Embedded Python。
- 自动启动 PyStand.exe 同级目录下面的 PyStand.int 程序（Python 代码）。
- 如果改名，会加载对应名称的 `.int` 文件，比如改为 `MyDemo.exe` 就会加载 `MyDemo.int`。
- 窗口程序，无 Console，但是如果在 cmd.exe 内运行，可以看到 print 的内容。
- 会自动添加 PyStand.exe 同级目录下的 `site-packages` 目录，库可以放到里面。

## 使用方式

- 用 CMake 生成 PyStand.exe （或者到 Release 里下个现成的）。
- 下载 Python Embedded 版本，放到 PyStand.exe 所在目录的 runtime 子目录内。
- 注意 Python Embedded 如果是 32 位，PyStand 配置 CMake 时也需要指明 `-A Win32`。
- 在 PyStand.exe 所在目录创建 Python 源代码 PyStand.int。
- 双击 PyStand.exe 就会运行 PyStand.int 里的代码。

## 配置程序

### 安装依赖

用一个同 Embedded Python 相同版本的 Python 做一个 venv，然后 pip 独立安装好模块后
将 site-packages 内对应的包复制到 PyStand.exe 的 site-packages 下直接使用。

### 查看错误

如果在 cmd.exe 内部运行 PyStand.exe 可以看到标准输出和标准错误。

### MessageBox

PyStand 添加了一个 `os.MessageBox(msg, title)` 的接口，可以用来简单显示个对话框。

### 更换图标

可以替换 appicon.ico 文件并重新编译 PyStand.exe ，或者使用 Resource Hacker 直接
替换 Release 内下载的 PyStand.exe 文件的程序图标。

### 大量脚本

可以在 PyStand.exe 同级目录新建一个 script 文件夹，将脚本放进去，PyStand.int 里面
就是添加一下 sys.path 然后 import 即可。

发布打包时将 script 文件夹用 zip 压缩成 script.egg 文件，PyStand.int 里检测到该
文件存在就加入到 sys.path，然后再 import。

## 使用例子

见 Release 下面的内容。


