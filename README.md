# DeepFilterNet VST2

基于 `JUCE 8.0.12 + FST` 的 DeepFilterNet VST2 降噪插件工程。

## 仓库内容

- `DeepFilterNetVst2/`：插件源代码、GUI、运行时资源嵌入逻辑。
- `DeepFilterNetVst2/EmbeddedAssets/`：会被打包进插件的 `df.dll` 与模型文件。
- `extern/JUCE/`：JUCE 子模块。
- `extern/FST/`：FST 子模块，用作 JUCE 的 VST2 兼容头来源。

## 特性

- 单文件分发：最终只需要 `DeepFilterNet VST2.dll`。
- 内嵌运行时：`df.dll` 与 `DeepFilterNet3_onnx.tar.gz` 会嵌入插件资源。
- JUCE 原生 GUI：支持 `Denoise Strength` 与 `Post Filter` 参数。
- 自动重采样：宿主不是 `48 kHz` 时，插件内部会自动重采样到 `48 kHz` 后处理，再转回宿主采样率。

## 构建

前置要求：

- Windows
- CMake 3.22+
- Visual Studio Build Tools / MSVC x64
- Git（用于拉取子模块）

初始化子模块：

```bash
git submodule update --init --recursive
```

使用 Visual Studio 生成器：

```bash
cmake -S DeepFilterNetVst2 -B build/juce-vst2 -G "Visual Studio 17 2022" -A x64
cmake --build build/juce-vst2 --config Release
```

或在开发者命令行里使用 NMake：

```bash
cmake -S DeepFilterNetVst2 -B build/juce-vst2-msvc -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build build/juce-vst2-msvc
```

输出文件：

- `build/juce-vst2/DeepFilterNetVst2_artefacts/Release/VST/DeepFilterNet VST2.dll`
- `build/juce-vst2-msvc/DeepFilterNetVst2_artefacts/Release/VST/DeepFilterNet VST2.dll`

## 运行方式

- 插件首次加载时会把嵌入的 `df.dll` 和模型释放到系统临时目录缓存。
- 处理链路固定运行在 `48 kHz`。
- 当前音频处理逻辑为单声道降噪，输出会复制到所有输出通道。

## 许可证说明

这个仓库目前没有单独声明顶层项目许可证。

原因是当前实现同时依赖：

- `extern/FST/`：GPLv3+
- `extern/JUCE/`：AGPLv3 或商业许可证

你在公开发布或二次分发前，需要先明确处理整体许可证兼容性问题。
