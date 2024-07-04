#pragma once

#include <QAbstractListModel>
#include <QVector>
#include "./cpu_globals.hpp"
#include "sim/api2/trace/iterator.hpp"

class CPU_EXPORT Flag {
public:
  explicit Flag(QString name, std::function<bool()> value);
  QString name() const;
  bool value() const;

private:
  QString _name;
  std::function<bool()> _fn;
};

//  Read only class for change in status bits
class CPU_EXPORT FlagModel : public QAbstractListModel {
  Q_OBJECT

public:
  enum class Roles { Value = Qt::UserRole + 1 };
  Q_ENUM(Roles);

  explicit FlagModel(QObject *parent = nullptr);
  ~FlagModel() = default;

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

  void appendFlag(QSharedPointer<Flag> flag);

public slots:
  void onUpdateGUI(sim::api2::trace::FrameIterator);

protected: //  Role Names must be under protected
  QHash<int, QByteArray> roleNames() const override;
  QVector<QSharedPointer<Flag>> _flags;
};
