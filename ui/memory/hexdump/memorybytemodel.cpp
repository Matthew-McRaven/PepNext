#include "memorybytemodel.hpp"
#include "rawmemory.hpp"

#include <cmath>

#include <QBrush>
#include <QColor>
#include <QPoint>
#include <QQmlEngine>
#include <QRect>
//  For testing only
#include <QRandomGenerator>

MemoryByteModel::MemoryByteModel(QObject *parent, const quint32 totalBytes, const quint8 bytesPerRow)
    : QAbstractTableModel(parent), empty_(new EmptyRawMemory(totalBytes, this)), memory_(empty_),
      column_(new MemoryColumns) {
  //  Changing width also changes height
  setNumBytesPerLine(bytesPerRow);
  clear();
  //  Test last cell
  // writeByte(size_ -1, 88);
}

ARawMemory *MemoryByteModel::memory() const {
  QQmlEngine::setObjectOwnership(memory_, QQmlEngine::CppOwnership);
  return memory_;
}

void MemoryByteModel::setMemory(ARawMemory *memory) {
  if (memory_ == memory)
    return;
  memory_ = memory;
  emit memoryChanged();
}

quint8 MemoryByteModel::readByte(const quint32 address) const { return memory_->read(address); }

void MemoryByteModel::writeByte(const quint32 address, const quint8 value) {
  memory_->write(address, value);

  const auto index = memoryIndex(address);

  //  Write byte to model
  setData(index, value, Qt::DisplayRole);
}

void MemoryByteModel::setNumBytesPerLine(const quint8 bytesPerLine) {
  Q_ASSERT(bytesPerLine > 0);

  //  Set bytes per row
  //  Initialized on construction to 8 bytes per row.
  //  If values are invalid, default is used
  if (bytesPerLine == 0) {
    width_ = 8;
  } else {
    //  Limit size to 32 since screen refresh will be slow
    width_ = bytesPerLine > 32 ? 32 : bytesPerLine;
  }

  //  Compute memory height.
  const auto size = memory_->byteCount();
  height_ = size / width_;

  //  Pad last row if not exactly divisible
  if ((size % width_) != 0)
    ++height_;

  //  Updated column identifiers for width change
  column_->setNumBytesPerLine(width_);

  //  Signal that row count has changed
  emit dimensionsChanged();
}

QHash<int, QByteArray> MemoryByteModel::roleNames() const {
  using M = MemoryRoles::Roles;
  // Use a type alias so that auto-formatting is less ugly.
  using T = QHash<int, QByteArray>;
  static T ret = {{Qt::DisplayRole, "display"},
                  {Qt::ToolTipRole, "toolTip"},
                  {Qt::TextAlignmentRole, "textAlign"},
                  {M::Selected, "selected"},
                  {M::Editing, "editing"},
                  {M::Type, "type"}};
  qDebug() << ret;
  return ret;
}

QVariant MemoryByteModel::headerData(int section, Qt::Orientation orientation, int role) const { return QVariant(); }

int MemoryByteModel::rowCount(const QModelIndex &parent) const {
  //  Number of rows of binary numbers
  return height_;
}

int MemoryByteModel::columnCount(const QModelIndex &parent) const {
  //  Number of binary numbers in row plus row number and ascii representation
  Q_ASSERT(column_->Total() == (width_ + 4));
  return column_->Total();
}

QVariant MemoryByteModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid())
    return QVariant();

  // The index returns the requested row and column information
  // The first column is the line number, which we ignore
  const int row = index.row();
  const int col = index.column();
  const int i = memoryOffset(index);
  const bool editField = (flags(index) != Qt::NoItemFlags);

  using M = MemoryRoles::Roles;
  switch (role) {

  //  Determines which delegate to assign in grid
  case M::Type:
    //  First column is formatted row number
    if (col == column_->LineNo())
      return QString("lineNo");

    //  Last column is ascii representation of data
    if (col == column_->Ascii())
      return QVariant("ascii");

    if (col == column_->Border1() || col == column_->Border2())
      return QVariant("border");

    return QVariant("cell");
  case Qt::DisplayRole:
    //  First column is formatted row number
    if (col == column_->LineNo())
      return QStringLiteral("%1").arg(row * width_, 4, 16, QLatin1Char('0')).toUpper();

    //  Last column is ascii representation of data
    else if (col == column_->Ascii())
      return ascii(row);

    else if (col == column_->Border1() || col == column_->Border2())
      return {};

    else if (i < 0)
      return QVariant("");

    //  Show data in hex format
    return QStringLiteral("%1").arg(memory_->read(i), 2, 16, QLatin1Char('0')).toUpper();

  case M::Editing:
    //  for last line when memory model is smaller than displayed items
    if (i < 0)
      return QVariant();

    //  Only one cell can be edited at a time
    // return editing_;
    return i == editing_;
  case Qt::TextAlignmentRole:
    if (col == column_->Ascii())
      return QVariant(Qt::AlignLeft);

    //  Default for all other cells
    return QVariant(Qt::AlignHCenter);
  case Qt::ToolTipRole:
    //  Handle invalid index
    if (i < 0)
      return {};

    // return QVariant("tool tip");
    //   Only show for memory
    if (col >= column_->CellStart() && col <= column_->CellEnd()) {

      //  toUpper will work on entire string literal. Separate hex
      //  values and cast to upper case as separate strings
      const auto mem = QStringLiteral("%1").arg(i, 4, 16, QLatin1Char('0')).toUpper();
      auto v = memory_->read(i);
      const auto newH = QStringLiteral("%1").arg(v, 2, 16, QLatin1Char('0')).toUpper();
      const auto oldH = QStringLiteral("%1").arg(v, 2, 16, QLatin1Char('0')).toUpper();

      return QStringLiteral("<b>Memory Location: 0x%1</b><br>"
                            "Hex: 0x%2<br>"
                            "Unsigned Decimal: %3<br>"
                            "Binary: 0b%4<br>"
                            "Previous Hex: 0x%5<br>"
                            "Previous Unsigned Decimal: %6<br>"
                            "Previous Binary: 0b%7")
          .arg(mem)                       // Hex Address
          .arg(newH)                      // Hex
          .arg(v)                         // Decimal
          .arg(v, 8, 2, QLatin1Char('0')) // Binary
          .arg(oldH)                      // Hex
          .arg(0)                         // Decimal
          .arg(0);                        // Binary
    } else
      return {};
  }

  return QVariant();
}

bool MemoryByteModel::setData(const QModelIndex &index, const QVariant &value, int role) {
  //  See if value is different from passed in value
  using M = MemoryRoles::Roles;
  switch (role) {
  case M::Editing: {
    // if( current != value) {
    const int i = memoryOffset(index);

    //  Bad index, just return
    if (i < 0)
      return false;

    QModelIndex ascii = QAbstractItemModel::createIndex(index.row(), column_->Ascii());

    //  Save index for editing
    lastEdit_ = editing_;
    editing_ = value.toInt();

    //  Repaint changed row
    emit dataChanged(index, ascii);

    //  Return true if cleared
    return true;
    //}
    // break;
  }
  case Qt::DisplayRole: {

    int hex = (int)std::strtol(value.toString().toStdString().c_str(), NULL, 16);
    const auto current = data(index, role);
    if (current != hex) {
      const int i = memoryOffset(index);

      //  Bad index, just return
      if (i < 0)
        return false;

      QModelIndex ascii = QAbstractItemModel::createIndex(index.row(), column_->Ascii());

      memory_->write(i, hex);
      //  Repaint changed value
      emit dataChanged(index, ascii);
    }
  }
  }

  return false;
}

Qt::ItemFlags MemoryByteModel::flags(const QModelIndex &index) const {
  if (!index.isValid() || index.column() < column_->CellStart() || index.column() > column_->CellEnd())
    return Qt::NoItemFlags;

  //  All other items can be edited.
  return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void MemoryByteModel::clear() {
  memory_->clear();
  emit dataChanged(index(0, 0), index(height_ - 1, column_->Ascii()));
}

//  Convert from cellIndex to memory location
//  Model is calculated as (col, row)
std::size_t MemoryByteModel::memoryOffset(const QModelIndex &index) const {
  const std::size_t offset = index.row() * width_ + (index.column() - column_->CellStart());
  const auto size = memory_->byteCount();
  if (offset >= size)
    return -1;

  //  Test if index is inside data model
  if (index.row() < 0 && index.row() >= height_)
    return -1;

  //  First column is line number. Skip
  if (index.column() <= 0 && index.column() >= width_)
    return -1;

  //  First column is line number. Return cell index to first edit column
  return offset;
}

//  Convert to location in model from memory location
QModelIndex MemoryByteModel::memoryIndex(std::size_t index) {
  //  Check for memory overflow
  const auto size = memory_->byteCount();
  if (index >= size)
    return QModelIndex();

  const int row = std::floor(index / width_);
  const int col = index % width_;

  //  Test if index is inside data model
  if (row < 0 && row >= height_)
    return QModelIndex();

  //  First column is line number. Skip
  if (col <= 0 && col >= width_)
    return QModelIndex();

  //  First column is line number. Ignore
  return QAbstractItemModel::createIndex(row, col + column_->CellStart());
}

QString MemoryByteModel::ascii(const int row) const {
  const auto size = memory_->byteCount();
  const int start = row * width_;
  const int end = start + width_;
  const auto len = end - start;
  QString edit(static_cast<qsizetype>(len), ' ');
  for (int i = 0; i < len; ++i) {
    if (i + start >= size) {
      continue;
    }

    QChar c(memory_->read(start + i));

    if (c.isPrint()) {
      edit[i] = c;
    } else {
      edit[i] = '.';
    }
  }
  return edit;
}

QVariant MemoryByteModel::selected(const QModelIndex &index, const MemoryRoles::Roles role) const {
  if (!index.isValid())
    return QVariant();

  //  Convert to memory location
  int i = memoryOffset(index);
  if (i < 0)
    return false;

  //  Check for edit mode
  if (role == MemoryRoles::Editing)
    return editing_;
  // else if( role == RoleNames::Selected)
  //   Return indicator for selected
  //    return selected_.contains(i);
  return QVariant();
}

QVariant MemoryByteModel::setSelected(const QModelIndex &index, const MemoryRoles::Roles role) {
  //  Current field is not editable or selectable
  if (flags(index) == Qt::NoItemFlags)
    return false;

  //  Check for edit mode
  if (role == MemoryRoles::Editing) {
    //  New location
    const QModelIndex oldIndex = memoryIndex(editing_);

    //  Clear old value, if any
    if (oldIndex != index) {
      //  clear old index
      clearSelected(oldIndex, role);
    }

    //  Convert QModelIndex into memory location
    const auto i = memoryOffset(index);

    //  Set new value - changes formatting
    setData(index, QVariant::fromValue(i), role);

    return editing_ > -1;
  }

  //  Other roles are read only
  return QVariant();
}

void MemoryByteModel::clearSelected(const QModelIndex &index, const MemoryRoles::Roles role) {
  //  Return if index is invalid
  if (!index.isValid())
    return;

  //  Check for edit mode
  if (role == MemoryRoles::Editing) {
    if (editing_ == -1) {
      return;
    }

    //  Only 1 cell can be edited. Find cell from currently selected item
    lastEdit_ = editing_;
    const QModelIndex oldIndex = memoryIndex(editing_);

    //  Check that old index matches currently edited field
    //  before clearing.
    if (oldIndex.isValid() && index == oldIndex) {
      //  Fix colors on old cell
      setData(oldIndex, -1, role);
    }
  }
}

QModelIndex MemoryByteModel::currentCell() {
  //  Only 1 cell can be edited. Find cell from currently selected item
  return memoryIndex(editing_);
}

QModelIndex MemoryByteModel::lastCell() { return memoryIndex(lastEdit_); }
