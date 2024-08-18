/*
 * Copyright (c) 2024 J. Stanley Warford, Matthew McRaven
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "registration.hpp"
#include <aproject.hpp>
#include <pep10.hpp>
#include <qqml.h>
#include "ScintillaEditBase/ScintillaEditBase.h"
#include "about/contributors.hpp"
#include "about/dependencies.hpp"
#include "about/pepp.hpp"
#include "about/read.hpp"
#include "about/version.hpp"
#include "builtins/book_item_model.hpp"
#include "builtins/bookfiltermodel.hpp"
#include "builtins/helpmodel.hpp"
#include "components/charcheck.hpp"
#include "cpu/registermodel.hpp"
#include "cpu/statusbitmodel.hpp"
#include "memory/hexdump/memorybytemodel.hpp"
#include "preferences/preferencemodel.hpp"
#include "project/pep10.hpp"
#include "text/editor/object.hpp"
#include "text/editor/scintillaasmeditbase.hpp"
#include "utils/opcodemodel.hpp"
#include "utils/sequenceconverter.hpp"
#include "utils/strings.hpp"

void registerTypes(const char *) {
  // About
  qmlRegisterSingletonType<about::Version>("edu.pepp", 1, 0, "Version",
                                           [](QQmlEngine *, QJSEngine *) { return new about::Version(); });
  qmlRegisterUncreatableType<Maintainer>("edu.pepp", 1, 0, "Maintainer", "Must be created from C++");
  qmlRegisterSingletonType<QList<Maintainer *>>("edu.pepp", 1, 0, "Maintainers", [](QQmlEngine *, QJSEngine *) {
    // Need global scope ::, or it picks up about::Maintainer
    QList<::Maintainer *> maintainers{};
    for (const auto &maintainer : about::maintainers()) {
      auto *item = new ::Maintainer(maintainer.name, maintainer.email, nullptr);
      maintainers.push_back(item);
    }
    // Class assumes ownership of objects via modifying parent pointer.
    auto owning = new MaintainerList(maintainers);
    return owning;
  });
  qmlRegisterSingletonType<Contributors>("edu.pepp", 1, 0, "Contributors",
                                         [](QQmlEngine *, QJSEngine *) { return new Contributors(); });
  qmlRegisterUncreatableType<about::DependencyRoles>("edu.pepp", 1, 0, "DependencyRoles", "Error: only enums");
  qmlRegisterSingletonType<about::Dependencies>("edu.pepp", 1, 0, "Dependencies",
                                                [](QQmlEngine *, QJSEngine *) { return new about::Dependencies(); });
  qmlRegisterSingletonType<about::detail::ReadHelper>(
      "edu.pepp", 1, 0, "FileReader", [](QQmlEngine *, QJSEngine *eng) { return new about::detail::ReadHelper(eng); });
  // Builtins
  // TODO: Missing translations
  qmlRegisterType<builtins::BookModel>("edu.pepp", 1, 0, "BookModel");
  qmlRegisterType<builtins::BookFilterModel>("edu.pepp", 1, 0, "BookFilterModel");
  qmlRegisterUncreatableType<builtins::ArchitectureHelper>("edu.pepp", 1, 0, "Architecture", "Only enums");
  qmlRegisterUncreatableType<builtins::AbstractionHelper>("edu.pepp", 1, 0, "Abstraction", "Only enums");
  qmlRegisterType<HelpModel>("edu.pepp", 1, 0, "HelpModel");
  qmlRegisterUncreatableType<HelpEntry>("edu.pepp", 1, 0, "HelpEntry", "Created with HelpModel");
  // Components
  // TODO: Missing translations
  qmlRegisterType<CharCheck>("edu.pepp", 1, 0, "CharCheck");
  // CPU
  qmlRegisterType<RegisterModel>("edu.pepp", 1, 0, "RegisterModel");
  qmlRegisterType<FlagModel>("edu.pepp", 1, 0, "FlagModel");
  // Memory
  // Note, these models are instantiated in C++ and passed to QML. QML
  // cannot instantiate these models directly
  qmlRegisterType<MemoryByteModel>("edu.pepp", 1, 0, "MemoryModel");
  qmlRegisterUncreatableType<MemoryRoles>("edu.pepp", 1, 0, "MemoryRoles", "Error: only enums");
  qmlRegisterUncreatableType<MemoryHighlight>("edu.pepp", 1, 0, "MemoryHighlight", "Error: only enums");
  qmlRegisterUncreatableType<EmptyRawMemory>("edu.pepp", 1, 0, "EmptyRawMemory", "Must use create(int)");
  qmlRegisterSingletonType<EmptyRawMemoryFactory>("edu.pepp", 1, 0, "EmptyRawMemoryFactory",
                                                  EmptyRawMemoryFactory::singletonProvider);
  qmlRegisterUncreatableType<ArrayRawMemory>("edu.pepp", 1, 0, "ArrayRawMemory", "Must use create(int)");
  qmlRegisterSingletonType<ArrayRawMemoryFactory>("edu.pepp", 1, 0, "ArrayRawMemoryFactory",
                                                  ArrayRawMemoryFactory::singletonProvider);
  // Preferences
  qmlRegisterUncreatableType<PreferenceModel>("edu.pepp", 1, 0, "PrefProperty", "Error: only enums");
  // Project
  qmlRegisterUncreatableType<project::DebugEnableFlags>("edu.pepp", 1, 0, "DebugEnableFlags", utils::error_only_enums);
  qmlRegisterUncreatableType<project::StepEnableFlags>("edu.pepp", 1, 0, "StepEnableFlags", utils::error_only_enums);
  qmlRegisterUncreatableType<Pep10_ISA>("edu.pepp", 1, 0, "Pep10ISA", utils::error_only_project);
  qmlRegisterUncreatableType<Pep10_ASMB>("edu.pepp", 1, 0, "Pep10ASMB", utils::error_only_project);
  qmlRegisterType<ProjectModel>("edu.pepp", 1, 0, "ProjectModel");
  // Text
  qmlRegisterType<ObjectUtilities>("edu.pepp.text", 1, 0, "ObjectUtilities");
  qmlRegisterType<ScintillaEditBase>("org.scintilla.scintilla", 1, 0, "ScintillaEditBase");
  qmlRegisterType<ScintillaAsmEditBase>("org.scintilla.scintilla", 1, 0, "ScintillaAsmEdit");
  // Utils
  qmlRegisterType<OpcodeModel>("edu.pepp", 1, 0, "OpcodeModel");
  qmlRegisterSingletonType<utils::SequenceConverter>("edu.pepp", 1, 0, "SequenceConverter",
                                                     [](auto *, auto *) { return new utils::SequenceConverter(); });
}
