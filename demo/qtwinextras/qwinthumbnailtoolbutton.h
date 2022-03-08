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

#include <QtCore/qobject.h>
#include <QtGui/qicon.h>

QT_BEGIN_NAMESPACE

class QWinThumbnailToolBar;
class QWinThumbnailToolButtonPrivate;

class QWinThumbnailToolButton : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(QWinThumbnailToolButton)
    Q_DECLARE_PRIVATE(QWinThumbnailToolButton)
    Q_PROPERTY(QString toolTip READ toolTip WRITE setToolTip NOTIFY toolTipChanged FINAL)
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon NOTIFY iconChanged FINAL)
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged FINAL)
    Q_PROPERTY(bool interactive READ isInteractive WRITE setInteractive NOTIFY interactiveChanged FINAL)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged FINAL)
    Q_PROPERTY(bool dismissOnClick READ dismissOnClick WRITE setDismissOnClick NOTIFY dismissOnClickChanged FINAL)
    Q_PROPERTY(bool flat READ isFlat WRITE setFlat NOTIFY flatChanged FINAL)

    friend class QWinThumbnailToolBar;

public:
    explicit QWinThumbnailToolButton(QObject *parent = nullptr);
    ~QWinThumbnailToolButton() override;

    void setToolTip(const QString &toolTip);
    [[nodiscard]] QString toolTip() const;

    void setIcon(const QIcon &icon);
    [[nodiscard]] QIcon icon() const;

    void setEnabled(const bool enabled);
    [[nodiscard]] bool isEnabled() const;

    void setInteractive(const bool interactive);
    [[nodiscard]] bool isInteractive() const;

    void setVisible(const bool visible);
    [[nodiscard]] bool isVisible() const;

    void setDismissOnClick(const bool dismiss);
    [[nodiscard]] bool dismissOnClick() const;

    void setFlat(const bool flat);
    [[nodiscard]] bool isFlat() const;

public Q_SLOTS:
    void click();

Q_SIGNALS:
    void clicked();
    void changed();

    void toolTipChanged();
    void iconChanged();
    void enabledChanged();
    void interactiveChanged();
    void visibleChanged();
    void dismissOnClickChanged();
    void flatChanged();

private:
    QScopedPointer<QWinThumbnailToolButtonPrivate> d_ptr;
    QWinThumbnailToolBar *toolbar = nullptr;
};

QT_END_NAMESPACE
