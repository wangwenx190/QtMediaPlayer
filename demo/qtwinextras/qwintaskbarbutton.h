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

#include <QtGui/qicon.h>
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QWindow;
class QWinTaskbarProgress;
class QWinTaskbarButtonPrivate;

class QWinTaskbarButton : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(QWinTaskbarButton)
    Q_DECLARE_PRIVATE(QWinTaskbarButton)
    Q_PROPERTY(QIcon overlayIcon READ overlayIcon WRITE setOverlayIcon RESET clearOverlayIcon NOTIFY overlayIconChanged FINAL)
    Q_PROPERTY(QString overlayAccessibleDescription READ overlayAccessibleDescription WRITE setOverlayAccessibleDescription NOTIFY overlayAccessibleDescriptionChanged FINAL)
    Q_PROPERTY(QWinTaskbarProgress *progress READ progress CONSTANT FINAL)
    Q_PROPERTY(QWindow *window READ window WRITE setWindow NOTIFY windowChanged FINAL)

public:
    explicit QWinTaskbarButton(QObject *parent = nullptr);
    ~QWinTaskbarButton() override;

    void setOverlayIcon(const QIcon &icon);
    void clearOverlayIcon();
    [[nodiscard]] QIcon overlayIcon() const;

    void setOverlayAccessibleDescription(const QString &description);
    [[nodiscard]] QString overlayAccessibleDescription() const;

    [[nodiscard]] QWinTaskbarProgress *progress() const;

    [[nodiscard]] QWindow *window() const;
    void setWindow(QWindow *window);

protected:
    [[nodiscard]] bool eventFilter(QObject *object, QEvent *event) override;

Q_SIGNALS:
    void overlayIconChanged();
    void overlayAccessibleDescriptionChanged();
    void windowChanged();

private:
    QScopedPointer<QWinTaskbarButtonPrivate> d_ptr;
};

QT_END_NAMESPACE
