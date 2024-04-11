#include "aproject.hpp"

#include <QQmlEngine>

Pep10_ISA::Pep10_ISA(QObject *parent) : QObject(parent) {}

project::Environment Pep10_ISA::env() const {
  return {.arch = utils::Architecture::Pep10, .level = utils::Abstraction::ISA3, .features = project::Features::None};
}

QString Pep10_ISA::objectCodeText() const { return _objectCodeText; }

void Pep10_ISA::setObjectCodeText(const QString &objectCodeText) {
  if (_objectCodeText == objectCodeText)
    return;
  _objectCodeText = objectCodeText;
  emit objectCodeTextChanged();
}

int ProjectModel::rowCount(const QModelIndex &parent) const { return _projects.size(); }

QVariant ProjectModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || index.row() >= _projects.size() || index.column() != 0)
    return {};
  switch (role) {
  case Qt::DisplayRole:
    return QVariant::fromValue(_projects[index.row()]);
  case (int)Roles::MODES:
    return QVariant::fromValue(_projects[index.row()]->modes());
  }
  return {};
}

Pep10_ISA *ProjectModel::pep10ISA() {
  auto ret = new Pep10_ISA();
  QQmlEngine::setObjectOwnership(ret, QQmlEngine::CppOwnership);
  beginInsertRows(QModelIndex(), _projects.size(), _projects.size());
  _projects.push_back(ret);
  endInsertRows();
  return ret;
}

bool ProjectModel::removeRows(int row, int count, const QModelIndex &parent) {
  if (row < 0 || row + count >= _projects.size())
    return false;
  beginRemoveRows(QModelIndex(), row, row + count);
  _projects.erase(_projects.begin() + row, _projects.begin() + row + count);
  endRemoveRows();
  return true;
}

bool ProjectModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                            const QModelIndex &destinationParent, int destinationChild) {
  return false;
}

QHash<int, QByteArray> ProjectModel::roleNames() const {
  auto ret = QAbstractListModel::roleNames();
  ret[(int)Roles::MODES] = "modes";
  return ret;
}
