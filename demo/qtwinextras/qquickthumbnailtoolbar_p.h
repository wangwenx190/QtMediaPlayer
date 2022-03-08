/****************************************************************************
 **
 ** Copyright (C) 2022 Ivan Vizir <define-true-false@yandex.com>
 ** Copyright (C) 2022 The Qt Company Ltd.
 ** Contact: https://www.qt.io/licensing/
 **
 ** This file is part of the QtWinExtras module of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:LGPL$
 ** Commercial License Usage
 ** Licensees holding valid commercial Qt licenses may use this file in
 ** accordance with the commercial license agreement provided with the
 ** Software or, alternatively, in accordance with the terms contained in
 ** a written agreement between you and The Qt Company. For licensing terms
 ** and conditions see https://www.qt.io/terms-conditions. For further
 ** information use the contact form at https://www.qt.io/contact-us.
 **
 ** GNU Lesser General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU Lesser
 ** General Public License version 3 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.LGPL3 included in the
 ** packaging of this file. Please review the following information to
 ** ensure the GNU Lesser General Public License version 3 requirements
 ** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
 **
 ** GNU General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU
 ** General Public License version 2.0 or (at your option) the GNU General
 ** Public license version 3 or any later version approved by the KDE Free
 ** Qt Foundation. The licenses are as published by the Free Software
 ** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
 ** included in the packaging of this file. Please review the following
 ** information to ensure the GNU General Public License requirements will
 ** be met: https://www.gnu.org/licenses/gpl-2.0.html and
 ** https://www.gnu.org/licenses/gpl-3.0.html.
 **
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

#pragma once

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtQuick/qquickitem.h>
#include <QtCore/qurl.h>
#include "qquickthumbnailtoolbutton_p.h"

QT_BEGIN_NAMESPACE

class QWinThumbnailToolBar;

class QQuickThumbnailToolBar : public QQuickItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ThumbnailToolBar)
    Q_DISABLE_COPY_MOVE(QQuickThumbnailToolBar)
    Q_PROPERTY(int count READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(QQmlListProperty<QObject> data READ data FINAL)
    Q_PROPERTY(QQmlListProperty<QQuickThumbnailToolButton> buttons READ buttons NOTIFY buttonsChanged FINAL)
    Q_PROPERTY(bool iconicNotificationsEnabled READ iconicNotificationsEnabled WRITE setIconicNotificationsEnabled NOTIFY iconicNotificationsEnabledChanged FINAL)
    Q_PROPERTY(QUrl iconicThumbnailSource READ iconicThumbnailSource WRITE setIconicThumbnailSource NOTIFY iconicThumbnailSourceChanged FINAL)
    Q_PROPERTY(QUrl iconicLivePreviewSource READ iconicLivePreviewSource WRITE setIconicLivePreviewSource NOTIFY iconicLivePreviewSourceChanged FINAL)
    Q_CLASSINFO("DefaultProperty", "data")

public:
    explicit QQuickThumbnailToolBar(QQuickItem *parent = nullptr);
    ~QQuickThumbnailToolBar() override;

    [[nodiscard]] int count() const;

    [[nodiscard]] QQmlListProperty<QObject> data();
    [[nodiscard]] QQmlListProperty<QQuickThumbnailToolButton> buttons();

    Q_INVOKABLE void addButton(QQuickThumbnailToolButton *button);
    Q_INVOKABLE void removeButton(QQuickThumbnailToolButton *button);

    [[nodiscard]] bool iconicNotificationsEnabled() const;
    void setIconicNotificationsEnabled(const bool);
    [[nodiscard]] QUrl iconicThumbnailSource() const { return m_iconicThumbnailSource; }
    void setIconicThumbnailSource(const QUrl &);
    [[nodiscard]] QUrl iconicLivePreviewSource() const { return m_iconicLivePreviewSource; }
    void setIconicLivePreviewSource(const QUrl &);

public Q_SLOTS:
    void clear();

Q_SIGNALS:
    void countChanged();
    void buttonsChanged();
    void iconicNotificationsEnabledChanged();
    void iconicThumbnailSourceChanged();
    void iconicThumbnailRequested();
    void iconicLivePreviewSourceChanged();
    void iconicLivePreviewRequested();

private Q_SLOTS:
    void iconicThumbnailLoaded(const QVariant &);
    void iconicLivePreviewLoaded(const QVariant &);

protected:
    void itemChange(const ItemChange change, const ItemChangeData &data) override;

private:
    static void addData(QQmlListProperty<QObject> *property, QObject *button);
    [[nodiscard]] static qsizetype buttonCount(QQmlListProperty<QQuickThumbnailToolButton> *property);
    [[nodiscard]] static QQuickThumbnailToolButton *buttonAt(
        QQmlListProperty<QQuickThumbnailToolButton> *property, qsizetype index);

    QScopedPointer<QWinThumbnailToolBar> m_toolbar;
    QList<QQuickThumbnailToolButton *> m_buttons = {};
    QUrl m_iconicThumbnailSource = {};
    QUrl m_iconicLivePreviewSource = {};
};

QT_END_NAMESPACE
