/**********************************************************************
 *                                                                    *
 * Voreen - The Volume Rendering Engine                               *
 *                                                                    *
 * Created between 2005 and 2012 by The Voreen Team                   *
 * as listed in CREDITS.TXT <http://www.voreen.org>                   *
 *                                                                    *
 * This file is part of the Voreen software package. Voreen is free   *
 * software: you can redistribute it and/or modify it under the terms *
 * of the GNU General Public License version 2 as published by the    *
 * Free Software Foundation.                                          *
 *                                                                    *
 * Voreen is distributed in the hope that it will be useful,          *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU General Public License for more details.                       *
 *                                                                    *
 * You should have received a copy of the GNU General Public License  *
 * in the file "LICENSE.txt" along with this program.                 *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 * The authors reserve all rights not expressly granted herein. For   *
 * non-commercial academic use see the license exception specified in *
 * the file "LICENSE-academic.txt". To get information about          *
 * commercial licensing please contact the authors.                   *
 *                                                                    *
 **********************************************************************/

#include "voreen/qt/widgets/property/stringpropertywidget.h"

#include <QLineEdit>

namespace voreen {

StringPropertyWidget::StringPropertyWidget(StringProperty* prop, QWidget* parent)
    : QPropertyWidget(prop, parent)
    , property_(prop)
{
    lineEdit_ = new QLineEdit(QString(prop->get().c_str()));
    addWidget(lineEdit_);

    connect(lineEdit_, SIGNAL(textChanged(QString)), this, SLOT(setProperty(QString)));
    connect(lineEdit_, SIGNAL(textChanged(QString)), this, SIGNAL(widgetChanged()));

    addVisibilityControls();
}

void StringPropertyWidget::updateFromProperty() {
    lineEdit_->blockSignals(true);
    QString st = QString::fromStdString(property_->get());
    if(st != lineEdit_->text())
        lineEdit_->setText(st);
    lineEdit_->blockSignals(false);
}

void StringPropertyWidget::setProperty(const QString& text) {
    if (!disconnected_) {
        property_->set(text.toStdString());
        emit modified();
    }
    else
        updateFromProperty();
}

} // namespace
