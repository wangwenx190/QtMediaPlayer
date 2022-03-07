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
#include <QtQml/qqmlregistration.h>

QT_BEGIN_NAMESPACE

class QWinTaskbarProgressPrivate;

class QWinTaskbarProgress : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(TaskbarProgress)
    QML_UNCREATABLE("Cannot create TaskbarProgress - use TaskbarButton.progress instead.")
    Q_DISABLE_COPY_MOVE(QWinTaskbarProgress)
    Q_DECLARE_PRIVATE(QWinTaskbarProgress)
    Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged FINAL)
    Q_PROPERTY(int minimum READ minimum WRITE setMinimum NOTIFY minimumChanged FINAL)
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum NOTIFY maximumChanged FINAL)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibilityChanged FINAL)
    Q_PROPERTY(bool paused READ isPaused WRITE setPaused NOTIFY pausedChanged FINAL)
    Q_PROPERTY(bool stopped READ isStopped NOTIFY stoppedChanged FINAL)

public:
    explicit QWinTaskbarProgress(QObject *parent = nullptr);
    ~QWinTaskbarProgress() override;

    void setValue(const int value);
    [[nodiscard]] int value() const;

    void setMinimum(const int minimum);
    [[nodiscard]] int minimum() const;

    void setMaximum(const int maximum);
    [[nodiscard]] int maximum() const;

    void setVisible(const bool visible);
    [[nodiscard]] bool isVisible() const;

    void setPaused(const bool paused);
    [[nodiscard]] bool isPaused() const;

    [[nodiscard]] bool isStopped() const;

public Q_SLOTS:
    void setRange(const int minimum, const int maximum);
    void reset();
    void show();
    void hide();
    void pause();
    void resume();
    void stop();

Q_SIGNALS:
    void valueChanged();
    void minimumChanged();
    void maximumChanged();
    void visibilityChanged();
    void pausedChanged();
    void stoppedChanged();

private:
    QScopedPointer<QWinTaskbarProgressPrivate> d_ptr;
};

QT_END_NAMESPACE
