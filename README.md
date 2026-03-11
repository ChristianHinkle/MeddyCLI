# MeddyCLI

A command-line interface application for Meddy.

See [MeddySuperproject](https://github.com/ChristianHinkle/MeddySuperproject) for the rest of the Meddy repositories and further documentation.

## Dependencies

MeddyCLI
- [MeddySDK_Meddyproject](https://github.com/ChristianHinkle/MeddySDK_Meddyproject)
- [boost_filesystem](https://github.com/boostorg/filesystem)
- [CppUtils_Core](https://github.com/ChristianHinkle/CppUtils_Core)
- [CppUtils_Misc](https://github.com/ChristianHinkle/CppUtils_Misc)
- [CppUtils_StdReimpl](https://github.com/ChristianHinkle/CppUtils_StdReimpl)

## Project Structure 📂

Does it install: Yes, it's installed as the `MeddyCLI` package and export.

### Source/

Provides the target: `MeddyCLI::Source` (interface library).

Does it install: Yes, it's installed as the `Source` component.

What it is: Holds all the private source files to compile.

### Executable/

Provides the target: `MeddyCLI::Executable` (executable).

Does it install: Yes, it's installed as the `Executable` component.

### Tests/

Does it install: No, but we should support this so that dependent projects can use our test code to help with writing their own tests.

What it is: Provides automated tests. See "Test Instructions" for how to use.

## Build System ⌨

Built with CMake - cross-platform, standardized, and IDE-friendly.

We provide CMake presets, which handle feeding arguments to CMake for you.

### IDE Support

Most IDEs provide built-in CMake integration.

#### VS Code

Has the "CMake Tools" and "C/C++" extensions, both developed by Microsoft.

#### Visual Studio

Has very nice integration, but they seem behind when it comes to supporting the latest CMake features. I've had experiences where I have to switch to VS Code because of this.

## Build Instructions 🔨

This project is fully isolated from its dependencies, and therefore must be tied together by a superproject. See [MeddySuperproject](https://github.com/ChristianHinkle/MeddySuperproject) for a complete build setup, as well as instructions for how to build, package, and test.
