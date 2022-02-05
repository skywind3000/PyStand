# PyStand

Python 独立部署环境。

## 特性说明

- Windows 下独立 Python 环境的启动器。
- 自动加载 PyStand.exe 同级目录下面 runtime 子目录内的 Embedded Python。
- 自动启动 PyStand.exe 同级目录下面的 PyStand.int 程序（Python 代码）。
- 如果改名，会加载对应名称的 `.int` 文件，比如改为 `MyDemo.exe` 就会加载 `MyDemo.int`。
- 窗口程序，无 Console，但是如果在 cmd.exe 内运行，可以看到 print 的内容。

## 使用说明

- 用 CMake 生成 PyStand.exe
- 下载 Python Embedded 版本，放到 PyStand.exe 所在目录的 runtime 子目录内。
- 注意 Python Embedded 如果是 32 位，PyStand 配置 CMake 时也需要指明 `-A Win32`。
- 在 PyStand.exe 所在目录创建 Python 源代码 PyStand.int。
- 双击 PyStand.exe 就会运行 PyStand.int 里的代码。
- 如果在 cmd.exe 内部运行 PyStand.exe 可以看到标准输出和标准错误。

## 使用例子

见 Release 下面的内容。


