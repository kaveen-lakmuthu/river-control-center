#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>

class KeybindingsController final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList bindings READ bindings NOTIFY bindingsChanged)
    Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY filterChanged)
    Q_PROPERTY(QString categoryFilter READ categoryFilter WRITE setCategoryFilter NOTIFY filterChanged)
    Q_PROPERTY(bool dirty READ dirty NOTIFY dirtyChanged)
    Q_PROPERTY(QString conflictSummary READ conflictSummary NOTIFY bindingsChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusChanged)
    Q_PROPERTY(bool statusError READ statusError NOTIFY statusChanged)

public:
    explicit KeybindingsController(QObject *parent = nullptr);

    QVariantList bindings() const;
    QString searchText() const { return m_searchText; }
    QString categoryFilter() const { return m_categoryFilter; }
    bool dirty() const { return m_dirty; }
    QString conflictSummary() const;
    QString statusMessage() const { return m_statusMessage; }
    bool statusError() const { return m_statusError; }

    void setSearchText(const QString &value);
    void setCategoryFilter(const QString &value);

    Q_INVOKABLE void addBinding(const QString &label, const QString &category,
                                const QString &mode, const QString &modifiers,
                                const QString &key, const QString &command);
    Q_INVOKABLE void updateBinding(int sourceIndex, const QString &label,
                                   const QString &category, const QString &mode,
                                   const QString &modifiers, const QString &key,
                                   const QString &command);
    Q_INVOKABLE void removeBinding(int sourceIndex);
    Q_INVOKABLE void resetBinding(int sourceIndex);
    Q_INVOKABLE bool saveAndApply();
    Q_INVOKABLE void reload();
    Q_INVOKABLE bool exportPrintableMap();
    QString validationError() const;
    bool regenerate(QString *error = nullptr);

signals:
    void bindingsChanged();
    void filterChanged();
    void dirtyChanged();
    void statusChanged();

private:
    static QVariantList defaultBindings();
    QString canonicalChord(const QVariantMap &binding, const QString &mode) const;
    QStringList bindingModes(const QVariantMap &binding) const;
    bool load(QString *error = nullptr);
    bool save(QString *error);
    bool generate(QString *error);
    bool applyLive();
    bool writeFile(const QString &path, const QByteArray &contents, QString *error) const;
    QString printableMap() const;
    void setDirty(bool value);
    void setStatus(const QString &message, bool error = false);

    QString m_configRoot;
    QString m_settingsPath;
    QString m_generatedPath;
    QVariantList m_bindings;
    QVariantList m_savedBindings;
    QString m_searchText;
    QString m_categoryFilter = QStringLiteral("all");
    bool m_dirty = false;
    QString m_statusMessage;
    bool m_statusError = false;
};
