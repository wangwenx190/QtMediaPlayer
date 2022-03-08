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
#include <QtCore/qurl.h>
#include <QtQml/qqmlregistration.h>

QT_BEGIN_NAMESPACE

class QWinThumbnailToolButton;

class QQuickThumbnailToolButton : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(ThumbnailToolButton)
    Q_DISABLE_COPY_MOVE(QQuickThumbnailToolButton)
    Q_PROPERTY(QUrl iconSource READ iconSource WRITE setIconSource NOTIFY iconSourceChanged FINAL)
    Q_PROPERTY(QString tooltip READ tooltip WRITE setTooltip NOTIFY tooltipChanged FINAL)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged FINAL)
    Q_PROPERTY(bool interactive READ isInteractive WRITE setInteractive NOTIFY interactiveChanged FINAL)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged FINAL)
    Q_PROPERTY(bool dismissOnClick READ dismissOnClick WRITE setDismissOnClick NOTIFY dismissOnClickChanged FINAL)
    Q_PROPERTY(bool flat READ isFlat WRITE setFlat NOTIFY flatChanged FINAL)

    friend class QQuickThumbnailToolBar;

public:
    explicit QQuickThumbnailToolButton(QObject *parent = nullptr);
    ~QQuickThumbnailToolButton() override;

    void setIconSource(const QUrl &iconSource);
    [[nodiscard]] QUrl iconSource();

    void setTooltip(const QString &tooltip);
    [[nodiscard]] QString tooltip() const;

    void setEnabled(const bool isEnabled);
    [[nodiscard]] bool isEnabled() const;

    void setInteractive(const bool isInteractive);
    [[nodiscard]] bool isInteractive() const;

    void setVisible(const bool isVisible);
    [[nodiscard]] bool isVisible() const;

    void setDismissOnClick(const bool dismiss);
    [[nodiscard]] bool dismissOnClick() const;

    void setFlat(const bool flat);
    [[nodiscard]] bool isFlat() const;

Q_SIGNALS:
    void clicked();
    void iconSourceChanged();
    void tooltipChanged();
    void enabledChanged();
    void interactiveChanged();
    void visibleChanged();
    void dismissOnClickChanged();
    void flatChanged();

private Q_SLOTS:
    void iconLoaded(const QVariant &);

private:
    QUrl m_iconSource = {};
    QWinThumbnailToolButton *m_button = nullptr;
};

QT_END_NAMESPACE
