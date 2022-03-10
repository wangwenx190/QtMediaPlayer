/*
 * MIT License
 *
 * Copyright (C) 2022 by wangwenx190 (Yuhang Zhao)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <QtCore/qobject.h>
#include <QtGui/qcolor.h>
#include <QtQml/qqmlregistration.h>

class Theme : public QObject
{
    Q_OBJECT
#ifdef QML_ELEMENT
    QML_ELEMENT
#endif
#ifdef QML_SINGLETON
    QML_SINGLETON
#endif
    Q_DISABLE_COPY_MOVE(Theme)
    Q_PROPERTY(bool darkModeEnabled READ darkModeEnabled NOTIFY darkModeEnabledChanged FINAL)
    Q_PROPERTY(QColor titleBarBackgroundColor READ titleBarBackgroundColor NOTIFY titleBarBackgroundColorChanged FINAL)
    Q_PROPERTY(QColor windowBackgroundColor READ windowBackgroundColor NOTIFY windowBackgroundColorChanged FINAL)
    Q_PROPERTY(QColor themeColor READ themeColor NOTIFY themeColorChanged FINAL)
    Q_PROPERTY(QColor systemColor READ systemColor NOTIFY systemColorChanged FINAL)
    Q_PROPERTY(QColor sliderBackgroundColor READ sliderBackgroundColor NOTIFY sliderBackgroundColorChanged FINAL)
    Q_PROPERTY(QColor sliderHandleBorderColor READ sliderHandleBorderColor NOTIFY sliderHandleBorderColorChanged FINAL)
    Q_PROPERTY(QColor systemAccentColor READ systemAccentColor NOTIFY systemAccentColorChanged FINAL)
    Q_PROPERTY(QColor windowFrameBorderColor READ windowFrameBorderColor NOTIFY windowFrameBorderColorChanged FINAL)

public:
    explicit Theme(QObject *parent = nullptr);
    ~Theme() override;

    [[nodiscard]] bool darkModeEnabled() const;

    [[nodiscard]] QColor titleBarBackgroundColor() const;
    [[nodiscard]] QColor windowBackgroundColor() const;
    [[nodiscard]] QColor themeColor() const;
    [[nodiscard]] QColor systemColor() const;
    [[nodiscard]] QColor sliderBackgroundColor() const;
    [[nodiscard]] QColor sliderHandleBorderColor() const;
    [[nodiscard]] QColor systemAccentColor() const;
    [[nodiscard]] QColor windowFrameBorderColor() const;

public Q_SLOTS:
    void refresh();

Q_SIGNALS:
    void darkModeEnabledChanged();
    void titleBarBackgroundColorChanged();
    void windowBackgroundColorChanged();
    void themeColorChanged();
    void systemColorChanged();
    void sliderBackgroundColorChanged();
    void sliderHandleBorderColorChanged();
    void systemAccentColorChanged();
    void windowFrameBorderColorChanged();

private:
    bool m_darkModeEnabled = false;
    QColor m_titleBarBackgroundColor = {};
    QColor m_windowBackgroundColor = {};
    QColor m_themeColor = {};
    QColor m_systemColor = {};
    QColor m_sliderBackgroundColor = {};
    QColor m_sliderHandleBorderColor = {};
    QColor m_systemAccentColor = {};
    QColor m_windowFrameBorderColor = {};
};
