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
#include <QtGui/qpixmap.h>

QT_BEGIN_NAMESPACE

class QWindow;
class QWinThumbnailToolButton;
class QWinThumbnailToolBarPrivate;

class QWinThumbnailToolBar : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(QWinThumbnailToolBar)
    Q_DECLARE_PRIVATE(QWinThumbnailToolBar)
    Q_PROPERTY(int count READ count STORED false NOTIFY countChanged FINAL)
    Q_PROPERTY(QWindow *window READ window WRITE setWindow NOTIFY windowChanged FINAL)
    Q_PROPERTY(bool iconicPixmapNotificationsEnabled READ iconicPixmapNotificationsEnabled WRITE setIconicPixmapNotificationsEnabled NOTIFY iconicPixmapNotificationsEnabledChanged FINAL)
    Q_PROPERTY(QPixmap iconicThumbnailPixmap READ iconicThumbnailPixmap WRITE setIconicThumbnailPixmap NOTIFY iconicThumbnailPixmapChanged FINAL)
    Q_PROPERTY(QPixmap iconicLivePreviewPixmap READ iconicLivePreviewPixmap WRITE setIconicLivePreviewPixmap NOTIFY iconicLivePreviewPixmapChanged FINAL)

    friend class QWinThumbnailToolButton;

public:
    explicit QWinThumbnailToolBar(QObject *parent = nullptr);
    ~QWinThumbnailToolBar() override;

    void setWindow(QWindow *window);
    [[nodiscard]] QWindow *window() const;

    void addButton(QWinThumbnailToolButton *button);
    void removeButton(QWinThumbnailToolButton *button);
    void setButtons(const QList<QWinThumbnailToolButton *> &buttons);
    [[nodiscard]] QList<QWinThumbnailToolButton *> buttons() const;
    [[nodiscard]] int count() const;

    [[nodiscard]] bool iconicPixmapNotificationsEnabled() const;
    void setIconicPixmapNotificationsEnabled(const bool enabled);

    [[nodiscard]] QPixmap iconicThumbnailPixmap() const;
    [[nodiscard]] QPixmap iconicLivePreviewPixmap() const;

public Q_SLOTS:
    void clear();
    void setIconicThumbnailPixmap(const QPixmap &);
    void setIconicLivePreviewPixmap(const QPixmap &);

Q_SIGNALS:
    void iconicThumbnailPixmapRequested();
    void iconicLivePreviewPixmapRequested();

    void countChanged();
    void windowChanged();
    void iconicPixmapNotificationsEnabledChanged();
    void iconicThumbnailPixmapChanged();
    void iconicLivePreviewPixmapChanged();

private:
    QScopedPointer<QWinThumbnailToolBarPrivate> d_ptr;
};

QT_END_NAMESPACE
