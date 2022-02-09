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
#include <QtQml/qqmlregistration.h>

class QQmlEngine;

class I18N : public QObject
{
    Q_OBJECT
#ifdef QML_ELEMENT
    QML_ELEMENT
#endif
#ifdef QML_SINGLETON
    QML_SINGLETON
#endif
    Q_DISABLE_COPY_MOVE(I18N)
    Q_PROPERTY(QStringList translations READ translations CONSTANT FINAL)
    Q_PROPERTY(QString translation READ translation WRITE setTranslation NOTIFY translationChanged FINAL)

public:
    explicit I18N(QObject *parent = nullptr);
    ~I18N() override;

    void setEngine(QQmlEngine *value);

    [[nodiscard]] QStringList translations() const;

    [[nodiscard]] QString translation() const;
    void setTranslation(const QString &value);

private:
    void reload();
    void removeCurrentTranslationIfAny();

Q_SIGNALS:
    void translationChanged();

private:
    QString m_currentTranslation = {};
};
