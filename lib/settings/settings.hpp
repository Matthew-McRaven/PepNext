#include <QObject>
#include <QtQmlIntegration>
#include "builtins/constants.hpp"

class QJSEngine;
class QQmlEngine;

namespace pepp::settings {
class Category : public QObject {
  Q_OBJECT
  QML_UNCREATABLE("")
  Q_PROPERTY(QString name READ name CONSTANT)
  Q_PROPERTY(QString source READ source CONSTANT)

public:
  explicit Category(QObject *parent = nullptr);
  virtual QString name() const = 0;
  virtual QString source() const { return "UnimplementedCategoryDelegate.qml"; };
  // Flush all QSettings to a file
  virtual void sync() {};
  // Reset all settings to their default values
  virtual void resetToDefault() {};
  // Pull all settings from QSettings
  virtual void reload() {};
};

class GeneralCategory : public Category {
  Q_OBJECT
  QML_UNCREATABLE("")
  QML_NAMED_ELEMENT(GeneralCategory)
  // "Defaults" group box
  // When given a file with an ambiguous extension, interpret it using this architecture.
  Q_PROPERTY(builtins::Architecture defaultArch READ defaultArch WRITE setDefaultArch NOTIFY defaultArchChanged)
  Q_PROPERTY(builtins::Abstraction defaultAbstraction READ defaultAbstraction WRITE setDefaultAbstraction NOTIFY
                 defaultAbstractionChanged)
  // "Menus" group box
  Q_PROPERTY(int maxRecentFiles READ maxRecentFiles WRITE setMaxRecentFiles NOTIFY maxRecentFilesChanged)
  Q_PROPERTY(bool showMenuHotkeys READ showMenuHotkeys WRITE setShowMenuHotkeys NOTIFY showMenuHotkeysChanged)

  // "Updates" group box
  Q_PROPERTY(bool showChangeDialog READ showChangeDialog WRITE setShowChangeDialog NOTIFY showChangeDialogChanged)
public:
  explicit GeneralCategory(QObject *parent = nullptr);
  QString name() const override { return "General"; };
  QString source() const override { return "GeneralCategoryDelegate.qml"; };
  void sync() override;

  builtins::Architecture defaultArch() const;
  void setDefaultArch(builtins::Architecture arch);
  builtins::Abstraction defaultAbstraction() const;
  void setDefaultAbstraction(builtins::Abstraction abstraction);
  int maxRecentFiles() const;
  void setMaxRecentFiles(int max);
  bool showMenuHotkeys() const;
  void setShowMenuHotkeys(bool show);
  bool showChangeDialog() const;
  void setShowChangeDialog(bool show);

signals:
  void defaultArchChanged();
  void defaultAbstractionChanged();
  void maxRecentFilesChanged();
  void showMenuHotkeysChanged();
  void showChangeDialogChanged();

private:
  mutable QSettings _settings;
  const builtins::Architecture defaultDefaultArch = builtins::Architecture::PEP10;
  const builtins::Abstraction defaultDefaultAbstraction = builtins::Abstraction::ASMB5;
  bool validateMaxRecentFiles(int max) const;
  const int defaultMaxRecentFiles = 5;
  const bool defaultShowMenuHotkeys = true;
  const bool defaultShowChangeDialog = true;
};

class ThemeCategory : public Category {
  Q_OBJECT
  QML_UNCREATABLE("")
  QML_NAMED_ELEMENT(ThemeCategory)

public:
  explicit ThemeCategory(QObject *parent = nullptr);
  QString name() const override { return "Fonts & Colors"; };
  QString source() const override { return "ThemeCategoryDelegate.qml"; };
};

class EditorCategory : public Category {
  Q_OBJECT
  QML_UNCREATABLE("")
  QML_NAMED_ELEMENT(ThemeCategory)
  Q_PROPERTY(
      bool visualizeWhitespace READ visualizeWhitespace WRITE setVisualizeWhitespace NOTIFY visualizeWhitespaceChanged)

public:
  explicit EditorCategory(QObject *parent = nullptr);
  QString name() const override { return "Editor"; };
  QString source() const override { return "EditorCategoryDelegate.qml"; };
  void sync() override;

  bool visualizeWhitespace() const;
  void setVisualizeWhitespace(bool visualize);

signals:
  void visualizeWhitespaceChanged();

private:
  bool _defaultVisualizeWhitespace = false;
  mutable QSettings _settings;
};

class SimulatorCategory : public Category {
  Q_OBJECT
  QML_UNCREATABLE("")
  QML_NAMED_ELEMENT(ThemeCategory)
  Q_PROPERTY(
      int maxStepbackBufferKB READ maxStepbackBufferKB WRITE setMaxStepbackBufferKB NOTIFY maxStepbackBufferKBChanged)

public:
  explicit SimulatorCategory(QObject *parent = nullptr);
  QString name() const override { return "Simulator"; };
  QString source() const override { return "SimulatorCategoryDelegate.qml"; };
  void sync() override;

  Q_INVOKABLE int minMaxStepbackBufferKB() const;
  Q_INVOKABLE int maxMaxStepbackBufferKB() const;
  int maxStepbackBufferKB() const;
  void setMaxStepbackBufferKB(int max);

signals:
  void maxStepbackBufferKBChanged();

private:
  bool validateMaxStepbackBufferKB(int max) const;
  int _defaultMaxStepbackBufferKB = 50;
  mutable QSettings _settings;
};

class KeyMapCategory : public Category {
  Q_OBJECT
  QML_UNCREATABLE("")
  QML_NAMED_ELEMENT(ThemeCategory)

public:
  explicit KeyMapCategory(QObject *parent = nullptr);
  QString name() const override { return "Key Map"; };
};

class AppSettings : public QObject {
  Q_OBJECT
  Q_PROPERTY(QList<Category *> categories READ categories CONSTANT)
  Q_PROPERTY(GeneralCategory general READ general CONSTANT)
  Q_PROPERTY(ThemeCategory theme READ theme CONSTANT)
  Q_PROPERTY(EditorCategory editor READ editor CONSTANT)
  Q_PROPERTY(SimulatorCategory simulator READ simulator CONSTANT)
  Q_PROPERTY(KeyMapCategory keymap READ keymap CONSTANT)
  QML_SINGLETON
  QML_NAMED_ELEMENT(AppSettings)
  Q_CLASSINFO("DefaultProperty", "categories")

public:
  explicit AppSettings(QObject *parent = nullptr);
  QList<Category *> categories() const { return _categories; };
  GeneralCategory *general() const { return _general; };
  ThemeCategory *theme() const { return _theme; }
  EditorCategory *editor() const { return _editor; }
  SimulatorCategory *simulator() const { return _simulator; }
  KeyMapCategory *keymap() const { return _keymap; }
public slots:
  void sync();

private:
  GeneralCategory *_general = nullptr;
  ThemeCategory *_theme = nullptr;
  EditorCategory *_editor = nullptr;
  SimulatorCategory *_simulator = nullptr;
  KeyMapCategory *_keymap = nullptr;
  QList<Category *> _categories;
};
} // namespace pepp::settings
