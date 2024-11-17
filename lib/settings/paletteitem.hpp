#pragma once
#include <QColor>
#include <QFont>
#include <QObject>
#include <QtQmlIntegration>
#include "./constants.hpp"

namespace pepp::settings {
class PaletteItem : public QObject {
  Q_OBJECT
  Q_PROPERTY(PaletteItem *parent READ parent WRITE setParent NOTIFY preferenceChanged)
  Q_PROPERTY(QColor foreground READ foreground WRITE setForeground NOTIFY preferenceChanged)
  Q_PROPERTY(QColor background READ background WRITE setBackground NOTIFY preferenceChanged)
  Q_PROPERTY(QFont font READ font WRITE setFont NOTIFY preferenceChanged);
  Q_PROPERTY(bool hasOwnForeground READ hasOwnForeground NOTIFY preferenceChanged)
  Q_PROPERTY(bool hasOwnBackground READ hasOwnBackground NOTIFY preferenceChanged)
  Q_PROPERTY(bool hasOwnFont READ hasOwnFont NOTIFY preferenceChanged)
  QML_UNCREATABLE("")
  QML_ELEMENT

public:
  struct PreferenceOptions {
    PaletteItem *parent = nullptr;
    std::optional<QColor> fg{std::nullopt};
    std::optional<QColor> bg{std::nullopt};
    std::optional<QFont> font{std::nullopt};
  };
  explicit PaletteItem(PreferenceOptions opts, PaletteRole ownRole, QObject *parent = nullptr);
  PaletteItem *parent();
  const PaletteItem *parent() const;
  Q_INVOKABLE void clearParent();
  // May fail if setting parent would induce a cycle in reltaionship graph.
  void setParent(PaletteItem *newParent);
  QColor foreground() const;
  Q_INVOKABLE void clearForeground();
  void setForeground(const QColor foreground);
  QColor background() const;
  Q_INVOKABLE void clearBackground();
  void setBackground(const QColor background);
  QFont font() const;
  Q_INVOKABLE void clearFont();
  void setFont(const QFont font);

  bool hasOwnForeground() const;
  bool hasOwnBackground() const;
  bool hasOwnFont() const;
  Q_INVOKABLE void overrideBold(bool bold);
  Q_INVOKABLE void overrideItalic(bool italic);
  Q_INVOKABLE void overrideUnderline(bool underline);
  Q_INVOKABLE void overrideStrikeout(bool strikeout);

  bool updateFromJson(const QJsonObject &json, PaletteRole ownRole = PaletteRole::Invalid,
                      PaletteItem *parent = nullptr);
  QJsonObject toJson();
signals:
  void preferenceChanged();

public slots:
  void onParentChanged();
  void emitChanged();

private:
  PaletteItem *_parent{nullptr};
  std::optional<QColor> _foreground{std::nullopt};
  std::optional<QColor> _background{std::nullopt};
  std::optional<QFont> _font{std::nullopt};
  struct FontOverride {
    std::optional<bool> strikeout{std::nullopt}, bold{std::nullopt}, underline{std::nullopt}, italic{std::nullopt};
    std::optional<int> weight{std::nullopt};
  } _fontOverrides;
  // Apply new font to this item if it doesn't violate monospace requirements.
  void updateFont(const QFont newFont);
  // Something about our parent changed... make sure we don't inherit a non-mono font.
  void preventNonMonoParent();
  PaletteRole _ownRole;
};
namespace detail {
bool isAncestorOf(const PaletteItem *maybeAncestor, const PaletteItem *maybeDescendant);
} // namespace detail

} // namespace pepp::settings
