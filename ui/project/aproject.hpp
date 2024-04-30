#pragma once
#include <QQmlEngine>
#include <QStringListModel>
#include <deque>
#include <qabstractitemmodel.h>
#include "cpu/registermodel.hpp"
#include "memory/hexdump/rawmemory.hpp"
#include "utils/constants.hpp"

namespace project {
// Additional options requested for a project.
// A particular (arch, level) tuple may only support a subset of features.
// TODO: Wrap in a Q_OBJECT to expose to QML.
enum class Features : int {
  None = 0,
  OneByte,
  TwoByte,
  NoOS,
};

// TODO: Expose values on AProject directly
struct Environment {
  utils::Architecture arch;
  utils::Abstraction level;
  Features features;
};
} // namespace project

// Dummy base class which provides functionality common to all projects.
class AProject : public QObject {
  Q_OBJECT
  Q_PROPERTY(project::Environment env READ env)
public:
  explicit AProject(project::Environment env) : _env(env) {}
  project::Environment env() const { return _env; }
  virtual ~AProject() = default;
  // virtual void *memoryModel() = 0;
  // virtual void *cpuModel() = 0;

private:
  const project::Environment _env;
};

struct HexFormatter : public RegisterFormatter {
  explicit HexFormatter(std::function<uint64_t()> fn) : _fn(fn) {}
  ~HexFormatter() override = default;
  QString format() const override { return QString::number(_fn(), 16); }
  bool readOnly() const override { return false; }
  qsizetype length() const override { return 4 + 2; }

private:
  std::function<uint64_t()> _fn;
};

struct UnsignedDecFormatter : public RegisterFormatter {
  explicit UnsignedDecFormatter(std::function<uint64_t()> fn) : _fn(fn) {}
  ~UnsignedDecFormatter() override = default;
  QString format() const override { return QString::number(_fn()); }
  bool readOnly() const override { return false; }
  qsizetype length() const override { return 5; }

private:
  std::function<uint64_t()> _fn;
};

struct SignedDecFormatter : public RegisterFormatter {
  explicit SignedDecFormatter(std::function<int64_t()> fn) : _fn(fn) {}
  ~SignedDecFormatter() override = default;
  QString format() const override { return QString::number(_fn()); }
  bool readOnly() const override { return false; }
  qsizetype length() const override { return 6; }

private:
  std::function<int64_t()> _fn;
};

struct BinaryFormatter : public RegisterFormatter {
  explicit BinaryFormatter(std::function<uint64_t()> fn) : _fn(fn) {}
  ~BinaryFormatter() override = default;
  QString format() const override { return QString::number(static_cast<int16_t>(_fn())); }
  bool readOnly() const override { return false; }
  qsizetype length() const override { return 8; }

private:
  std::function<uint64_t()> _fn;
};

class Pep10_ISA final : public QObject {
  Q_OBJECT
  Q_PROPERTY(project::Environment env READ env CONSTANT)
  Q_PROPERTY(utils::Architecture architecture READ architecture CONSTANT)
  Q_PROPERTY(utils::Abstraction abstraction READ abstraction CONSTANT)
  Q_PROPERTY(QVariant delegate MEMBER _delegate NOTIFY delegateChanged)
  Q_PROPERTY(QString objectCodeText READ objectCodeText WRITE setObjectCodeText NOTIFY objectCodeTextChanged);
  Q_PROPERTY(ARawMemory *memory READ memory CONSTANT)
  Q_PROPERTY(RegisterModel *registers MEMBER _registers CONSTANT)
public:
  explicit Pep10_ISA(QVariant delegate, QObject *parent = nullptr);
  project::Environment env() const;
  utils::Architecture architecture() const;
  utils::Abstraction abstraction() const;
  ARawMemory *memory() const;
  QString objectCodeText() const;
  void setObjectCodeText(const QString &objectCodeText);
  Q_INVOKABLE static QStringListModel *modes() {
    static QStringListModel ret({"Welcome", "Edit", "Debug", "Help"});
    QQmlEngine::setObjectOwnership(&ret, QQmlEngine::CppOwnership);
    return &ret;
  }
  // Actually utils::Abstraction, but QM passes it as an int.
  Q_INVOKABLE void set(int abstraction, QString value);

signals:
  void objectCodeTextChanged();
  void delegateChanged();

private:
  QString _objectCodeText = {};
  QVariant _delegate = {};
  ArrayRawMemory *_memory = nullptr;
  RegisterModel *_registers = nullptr;
};

// Factory to ensure class invariants of project are maintained.
// Must be a singleton to call methods on it.
// Can't seem to call Q_INVOKABLE on an uncreatable type.
class ProjectModel : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(int count READ _rowCount NOTIFY rowCountChanged)
public:
  enum class Roles {
    ProjectRole = Qt::UserRole + 1,
  };
  Q_ENUM(Roles);
  // Q_INVOKABLE ISAProject *isa(utils::Architecture::Value arch, project::Features features);
  explicit ProjectModel(QObject *parent = nullptr) : QAbstractListModel(parent){};
  // Helper to expose rowCount as a property to QML.
  int _rowCount() const { return rowCount({}); }
  int rowCount(const QModelIndex &parent) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  Q_INVOKABLE Pep10_ISA *pep10ISA(QVariant delegate);
  bool removeRows(int row, int count, const QModelIndex &parent) override;
  bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent,
                int destinationChild) override;
  QHash<int, QByteArray> roleNames() const override;
signals:
  void rowCountChanged(int);

private:
  std::deque<Pep10_ISA *> _projects = {};
};
