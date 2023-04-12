#pragma once
#include "errors.hpp"
#include "is.hpp"
#include "pas/ast/generic/attr_children.hpp"
#include "pas/ast/node.hpp"
#include "pas/ast/op.hpp"
#include <QtCore>

namespace pas::ops::generic {
struct GroupSections : public pas::ops::MutatingOp<void> {
  GroupSections(QString defaultSectionName,
                std::function<bool(const ast::Node &)> addressable);
  pas::ast::generic::Children newChildren;
  void operator()(ast::Node &node) override;

private:
  QSharedPointer<pas::ast::Node> currentSection;
  const std::function<bool(const ast::Node &)> addressable;
  bool hasSeenAddressable = false;
};

void groupSections(ast::Node &root,
                   std::function<bool(const ast::Node &)> addressable);
} // namespace pas::ops::generic
