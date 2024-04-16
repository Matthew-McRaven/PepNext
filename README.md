# Pepp

[![CI Status](https://github.com/matthew-mcraven/pepp/actions/workflows/ci.yml/badge.svg)](https://github.com/matthew-mcraven/pepp/actions/workflows/ci.yml)
![Codecov](https://img.shields.io/codecov/c/github/matthew-mcraven/Pepp)
![CodeFactor Grade](https://img.shields.io/codefactor/grade/github/matthew-mcraven/Pepp)
![GitHub file size in bytes](https://img.shields.io/github/repo-size/matthew-mcraven/Pepp)
[![Discord Invite](https://dcbadge.vercel.app/api/server/wF7HYdvF55?style=flat)](https://discord.gg/wF7HYdvF55)
[![Lines of Code](https://sonarcloud.io/api/project_badges/measure?project=Matthew-McRaven_Pepp&metric=ncloc)](https://sonarcloud.io/summary/new_code?id=Matthew-McRaven_Pepp)

# Table of Contents

1. [Building for Source](#build-from-sources)
2. [Previous Works](#relation-to-pep9suite)
3. [Pep/9 and /10 Virtual Machines](#what-are-pep9-and-pep10)
4. [Project Features](#what-is-pepp)
5. [Contributing](#contributing)
6. [Making Modifications](#making-modifications)

# Build from Sources

When cloning this project, use `--recurse-submodules` to install relevant source code dependencies.

| Required Build Tools                                              | Reason                                              |
|-------------------------------------------------------------------|-----------------------------------------------------|
| [Qt 6.6](https://doc.qt.io/qt-6/get-and-install-qt.html) or later | Required for cross-platform GUIs                    |
| [CMake 3.24](https://cmake.org/download/)  or later               | Required build system; usually bundled with Qt      |
| [git lfs](https://git-lfs.com/)                                   | Required for binary assets, such as images an icons |

While not required, the following tools enable additional features in built applications:

| Optional Build Tools                                                         | Reason                                   |
|------------------------------------------------------------------------------|------------------------------------------|
| [Qt Creator 12](https://www.qt.io/download) or later                         | Preferred IDE for this project           |
| [QT IFW 4.7](https://doc.qt.io/qtinstallerframework/) or later               | Creates Windows installers               |
| [Sphinx 7.1.2](https://www.sphinx-doc.org/en/master/usage/installation.html) | Generates application help documentation |

It is recommended to build the application from within Qt Creator.

## Additional Windows Build Steps

On Windows, the default build process does not set QML plugin search paths correctly.
In most cases, this prevents the application from being run in Qt Creator or from the command line.

This is fixed with
a [custom build step](https://doc.qt.io/qtcreator/creator-build-settings.html#adding-custom-build-steps).
This build step must be added to each build configuration (e.g., build, release).

| Line              | Value                                                            | Description                                              |
|-------------------|------------------------------------------------------------------|----------------------------------------------------------|
| Command           | `<QT_ROOT>/bin/windeployqt6.exe`                                 | On my PC, `<QT_ROOT>` is `C:/Qt/6.6.0/msvc2019_64`       |
| Arguments         | `%{buildDir}\output\pepp.exe --qmldir --qmldir %{sourceDir}\bin` | `--qmldir` is required for discovery of our dependencies |
| Working Directory | (blank)                                                          | This value does not matter and is unused                 |

# Relation to [Pep9Suite](https://github.com/StanWarford/pep9suite)

This project is a continuation of the Pep/9 computer's associated software
suite, [Pep9Suite](https://github.com/StanWarford/pep9suite).

When complete, this project will contain assembler and microcode simulators for Pep/8, Pep/9, and Pep/10, and we will
deprecate older repos.

The goal for this new project is to reduce cross-generation maintenance, improve reliability, and reduce turn-around
time for new features.

# What are Pep/9 and Pep/10

## Pep/9

The Pep/9 computer is a 16-bit complex instruction set computer (CISC).
It is designed to teach computer architecture, assembly language programming, and computer organization principles as
described in the text [_Computer
Systems_, J. Stanley Warford, 5th edition](http://computersystemsbook.com/5th-edition/).
Pep/9 instructions are based on an expanding opcode and are either unary (one byte) or nonunary (three bytes).
The eight addressing modes and eight dot commands are designed for straightforward translation from C to assembly
language.

## Pep/10

The Pep/10 computer is a 16-bit complex instruction set computer (CISC).
It is designed to teach computer architecture, assembly language programming, and computer organization principles as
described in a future book.
Pep/10 instructions are based on an expanding opcode and are either unary (one byte) or nonunary (three bytes).
The eight addressing modes and eight dot commands are designed for straightforward translation from C to assembly
language.
The inclusion of macros facilities ease this translation from C to assembly language.

# What is Pepp?

The Pep project consists of many iterations of a 16-bit complex instruction set computer .
Within this repository, the Pep/10 version (and soon the Pep/8 and Pep/9 versions) are represented across multiple
levels of abstraction.

The project now consists of a single download for all virtual machines at all levels of abstraction.
The terminal version was previously a stand-alone executable and is now part of the main download.

## Assembler

Our Pepp IDE features an integrated text editor, error messages in red type that are inserted within the source code at
the place where the error is detected, student-friendly machine language object code in hexadecimal format, the ability
to code directly in machine language, bypassing the assembler.

The Pep/9 computer features the ability to redefine the mnemonics for the unimplemented opcodes that trigger synchronous
traps.

The Pep/10 computer features the ability to define custom macros.

### ISA

The simulator features simulated ROM that is not altered by store instructions, a small operating system burned into
simulated ROM that includes a loader and a trap handler system, an integrated debugger that allows for break points,
single and multi step execution, CPU tracing, and memory tracing, the option to trace an application, the loader, or the
operating system, the ability to recover from endless loops, and the ability to modify the operating system by designing
new trap handlers for the unimplemented opcodes.

### Microcode

The CPU mode is a simulator allowing users to interact with the data sections of the Pep/9 and Pep/10 CPUs.

It contains two versions of the Pep/9 CPU data section &ndash; one with a one-byte wide data bus and another with a
two-byte wide data bus. Using a GUI, students are able to set the control signals to direct the flow of data and change
the state of the CPU. Alternatively, the Microcode IDE allows students to write microprogram code fragments to perform
useful computations. An integrated unit test facility allows users to write pre- and post-conditions to verify correct
behavior of arbitrary microprograms.

While debugging a microprogram fragment, the CPU simulator performs graphical tracing of data paths through the CPU.
Using breakpoints, students may skip over previously debugged microstatments and resume debugging at a later point in
the program.

### ISA / Microcode Interface

Contained within IDE is a fully microcoded implementation of the Pep/9 and Pep/10 virtual machines.
These extensions to their respective textbooks is dubbed *PepMicro*
PepMicro adds a control section, missing in Pep9CPU, and extends the microcode language to allow conditional microcode
branches.
It integrates all the programming features of Pep/9 and the graphical CPU interaction of Pep9CPU to simulate the
complete execution of assembly language programs.

* Extend the assembler and CPU simulator so that complete assembly language programs can be executed at the microcode
  level spanning four levels of system abstraction &ndash; the assembly level, the operating system level, the ISA
  level, and the microcode level.
* Runs both memory aligned and nonaligned programs. Assembly language programs that do not use optimal .ALIGN directives
  still execute correctly but slower.
* Provides performance statistics in the form of statement execution counts at the microcode level and the ISA level.
  Students can measure the performance differences between aligned and nonaligned programs.
* Retains the unit tests of the original Pep/9 CPU IDE so that students can write microcode fragments with the extended
  microinstruction format.
* Supports new debugging features like step-into, step-out, and step-over so students can trace assembly programs more
  efficiently.

## Terminal Support

The IDE includes a command-line version of the Pep/9 and Pep/10 virtual machine.s
It uses the assembler from the PepIDE application to create a .pepo file, and the simulator to execute the .pepo file.

Teachers can script PepTerm to batch test assembly language homework submissions.

# Contributing

Please see [our Contribution Guidelines](CONTRIBUTING.md) before contributing to this project.

# Making Modifications

| I want to...                                                         | ...so I should                                                                                                                                                 |
|----------------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Expose a new C++ type to QML from [ui/](ui/) or [bin/gui/](bin/gui)  | Add your [QML Registration code](https://doc.qt.io/qt-6/qtqml-cppintegration-definetypes.html) to the nearest _registration.cpp_                               |
| Add new registration to a child of [ui/](ui/) or [bin/gui/](bin/gui) | Copy an [existing registration](ui/text/registration.cpp). Update the [graphics entry point](bin/commands/gui.cpp) to invoke it                                |
| Add a new QML screen to the application from [ui/](/ui)              | Add your QML file, accessed elsewhere via `qrc:/ui/<directory relative to ui>`                                                                                 |
| Add a new QML screen to the application from [bin/gui](bin/gui)      | Add your QML file, accessed elsewhere via `qrc:/qt/qml/Pepp/gui/<directory relative to gui>`                                                                   |
| Expose a C++ object of a registered type as a QML global             | Set a property on the [root context of the QQMLEngine](https://doc.qt.io/qt-6/qqmlengine.html#rootContext) in the [graphics entry point](bin/commands/gui.cpp) |
| Modify the main screen of the application                            | Modify [bin/gui/main.qml](bin/gui/main.qml)                                                                                                                    |

## GUI Edits

If the feature is mostly complete (i.e., stable enough to be reused), it belongs under [ui/](ui/).
Otherwise, it can be prototyped under [bin/gui/](bin/gui/).
Please find an appropriate subdirectory in either location for your feature.



