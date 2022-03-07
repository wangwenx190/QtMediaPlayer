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

#include <QtCore/qobject.h>
#include <QtQml/qqmlregistration.h>
#include <QtQuick/qquickitem.h>
#include "qwintaskbarprogress.h"

QT_BEGIN_NAMESPACE

class QWinTaskbarButton;

class QQuickTaskbarOverlay : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(TaskbarOverlay)
    QML_UNCREATABLE("Cannot create TaskbarOverlay - use TaskbarButton.overlay instead.")
    Q_DISABLE_COPY_MOVE(QQuickTaskbarOverlay)
    Q_PROPERTY(QUrl iconSource READ iconSource WRITE setIconSource NOTIFY iconSourceChanged FINAL)
    Q_PROPERTY(QString accessibleDescription READ accessibleDescription WRITE setAccessibleDescription NOTIFY accessibleDescriptionChanged FINAL)

public:
    explicit QQuickTaskbarOverlay(QWinTaskbarButton *button, QObject *parent = nullptr);
    ~QQuickTaskbarOverlay() override;

    [[nodiscard]] QUrl iconSource() const;
    void setIconSource(const QUrl &iconSource);

    [[nodiscard]] QString accessibleDescription() const;
    void setAccessibleDescription(const QString &description);

Q_SIGNALS:
    void iconSourceChanged();
    void accessibleDescriptionChanged();

private Q_SLOTS:
    void iconLoaded(const QVariant &value);

private:
    QUrl m_iconSource = {};
    QWinTaskbarButton *m_button = nullptr;
};

class QQuickTaskbarButton : public QQuickItem
{
    Q_OBJECT
    QML_NAMED_ELEMENT(TaskbarButton)
    Q_DISABLE_COPY_MOVE(QQuickTaskbarButton)
    Q_PROPERTY(QQuickTaskbarOverlay *overlay READ overlay CONSTANT FINAL)
    Q_PROPERTY(QWinTaskbarProgress *progress READ progress CONSTANT FINAL)

public:
    explicit QQuickTaskbarButton(QQuickItem *parent = nullptr);
    ~QQuickTaskbarButton() override;

    [[nodiscard]] QQuickTaskbarOverlay *overlay() const;
    [[nodiscard]] QWinTaskbarProgress *progress() const;

protected:
    void itemChange(ItemChange change, const ItemChangeData &data) override;

private:
    QWinTaskbarButton *m_button = nullptr;
    QQuickTaskbarOverlay *m_overlay = nullptr;
};

QT_END_NAMESPACE
