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

#include "voreen/qt/widgets/property/camerawidget.h"
#include "voreen/qt/widgets/property/floatvec3propertywidget.h"
#include "voreen/qt/widgets/property/floatpropertywidget.h"
#include "voreen/qt/widgets/property/corepropertywidgetfactory.h"

#include "voreen/core/voreenapplication.h"
#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/interaction/voreentrackball.h"

#include "tgt/quaternion.h"
#include "tgt/quadric.h"
#include "tgt/glmath.h"
#include "tgt/quadric.h"
#include "tgt/glmath.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QComboBox>
#include <QApplication>
#include <QTabWidget>

#include "tinyxml/tinyxml.h"

#include <vector>

using std::abs;
using tgt::quat;
using tgt::vec3;

namespace voreen {

CameraWidget::CameraWidget(CameraProperty* cameraProp, float minDist, float maxDist, QWidget* parent)
    : QWidget(parent)
    , cameraProp_(cameraProp)
    , CAM_DIST_SCALE_FACTOR(100.0f)
    , AXIAL_VIEW(tgt::vec3(0.0f, 0.0f, -1.0f))
    , CORONAL_VIEW(tgt::vec3(0.0f, 1.0f, 0.0f))
    , SAGITTAL_VIEW(tgt::vec3(1.0f, 0.0f,0.0f))
    , AXIAL_INV_VIEW(tgt::vec3(0.0f, 0.0f, 1.0f))
    , CORONAL_INV_VIEW(tgt::vec3(0.0f, -1.0f, 0.0f))
    , SAGITTAL_INV_VIEW(tgt::vec3(-1.0f, 0.0f, 0.0f))
{
    setObjectName("CameraWidget");

    tgtAssert(cameraProp_, "No camera property");

    track_ = new VoreenTrackball(cameraProp_);

    minDist_ = tgt::iround(minDist * CAM_DIST_SCALE_FACTOR);
    maxDist_ = tgt::iround(maxDist * CAM_DIST_SCALE_FACTOR);

    setWindowIcon(QIcon(":/icons/trackball-reset.png"));
    dist_ = 5;
    timer_ = new QBasicTimer();
    rotateX_ = rotateY_ = rotateZ_ = false;
}

CameraWidget::~CameraWidget() {
    delete track_;
    delete timer_;
}

void CameraWidget::createWidgets() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QTabWidget* tab = new QTabWidget(this);
    mainLayout->addWidget(tab);
    QWidget* cameraTab = new QWidget();
    QWidget* positionTab = new QWidget();
    QWidget* projectionTab = new QWidget();

    tab->addTab(cameraTab, tr("Trackball"));
    tab->addTab(positionTab, tr("Tripod"));
    tab->addTab(projectionTab, tr("Projection"));

    QVBoxLayout* cameraLayout = new QVBoxLayout(cameraTab);
    QVBoxLayout* posLayout = new QVBoxLayout(positionTab);
    QVBoxLayout* projLayout = new QVBoxLayout(projectionTab);

    cameraTab->setLayout(cameraLayout);
    positionTab->setLayout(posLayout);
    projectionTab->setLayout(projLayout);

    // Group box for orientation
    orientationBox_ = new QGroupBox(tr("Orientation and Distance"));
    QGridLayout* gridLayout = new QGridLayout();
    gridLayout->setColumnStretch(0, 3);
    gridLayout->setColumnStretch(1, 2);

    gridLayout->addWidget(new QLabel(tr("Orientation: ")), 1, 0);

    comboOrientation_ = new QComboBox();
    comboOrientation_->addItem(tr("custom"));
    comboOrientation_->addItem(tr("axial"));
    comboOrientation_->addItem(tr("coronal"));
    comboOrientation_->addItem(tr("sagittal"));
    comboOrientation_->addItem(tr("axial (bottom)"));
    comboOrientation_->addItem(tr("coronal (back)"));
    comboOrientation_->addItem(tr("sagittal (left)"));
    gridLayout->addWidget(comboOrientation_, 1, 1);

    QGridLayout* positionLayout = new QGridLayout();
    QGroupBox* positionBox = new QGroupBox(tr("Position"));
    positionBox->setLayout(positionLayout);

    QGridLayout* focusLayout = new QGridLayout();
    QGroupBox* focusBox = new QGroupBox(tr("Focus"));
    focusBox->setLayout(focusLayout);

    QGridLayout* upLayout = new QGridLayout();
    QGroupBox* upBox = new QGroupBox(tr("Up Vector"));
    upBox->setLayout(upLayout);

    //cameraPosition_ = new FloatVec3Property("Position", "Position", tgt::vec3(0.0f), tgt::vec3(-FLT_MAX), tgt::vec3(FLT_MAX));
    cameraPosition_ = new FloatVec3Property("Position", "Position", tgt::vec3(0.0f), tgt::vec3(-500.0f), tgt::vec3(500.0f));
    cameraPosition_->set(cameraProp_->get().getPosition());

    //focusVector_ = new FloatVec3Property("Focus", "Focus", tgt::vec3(0.0f), tgt::vec3(-FLT_MAX), tgt::vec3(FLT_MAX));
    focusVector_ = new FloatVec3Property("Focus", "Focus", tgt::vec3(0.0f), tgt::vec3(-500.0f), tgt::vec3(500.0f));
    focusVector_->set(cameraProp_->get().getFocus());

    upVector_ = new FloatVec3Property("Upvector", "Upvector", tgt::vec3(0.0f), tgt::vec3(-1.0f), tgt::vec3(1.0f));
    upVector_->set(cameraProp_->get().getUpVector());

    CorePropertyWidgetFactory wf;

    FloatVec3PropertyWidget* pos = dynamic_cast<FloatVec3PropertyWidget*>(wf.createWidget(cameraPosition_));
    FloatVec3PropertyWidget* focus = dynamic_cast<FloatVec3PropertyWidget*>(wf.createWidget(focusVector_));
    FloatVec3PropertyWidget* up = dynamic_cast<FloatVec3PropertyWidget*>(wf.createWidget(upVector_));
    connect(pos, SIGNAL(valueChanged(FloatVec3Property::ElemType)), this, SLOT(positionChange(FloatVec3Property::ElemType)));
    connect(focus, SIGNAL(valueChanged(FloatVec3Property::ElemType)), this, SLOT(focusChange(FloatVec3Property::ElemType)));
    connect(up, SIGNAL(valueChanged(FloatVec3Property::ElemType)), this, SLOT(upChange(FloatVec3Property::ElemType)));

    positionLayout->addWidget(pos);
    cameraPosition_->addWidget(pos);
    focusLayout->addWidget(focus);
    focusVector_->addWidget(focus);
    upLayout->addWidget(up);
    upVector_->addWidget(up);
    gridLayout->addWidget(new QLabel(tr("Distance: ")), 2, 0);
    gridLayout->addWidget(slDistance_ = new QSlider(Qt::Horizontal, 0), 2, 1);
    slDistance_->setRange(minDist_, maxDist_);
    slDistance_->setSliderPosition(static_cast<int>(CAM_DIST_SCALE_FACTOR));
    slDistance_->setToolTip(tr("Adjust distance of point of view"));

    orientationBox_->setLayout(gridLayout);
    cameraLayout->addWidget(orientationBox_);
    posLayout->addWidget(positionBox);
    posLayout->addWidget(focusBox);
    posLayout->addWidget(upBox);

    // Group box for motion settings
    motionBox_ = new QGroupBox(tr("Continuous Motion"));
    QVBoxLayout* vboxLayout = new QVBoxLayout();

    rotateAroundX_ = new QCheckBox(tr("Continually rotate around x-Axis"));
    rotateAroundY_ = new QCheckBox(tr("Continually rotate around y-Axis"));
    rotateAroundZ_ = new QCheckBox(tr("Continually rotate around z-Axis"));
    continueSpin_  = new QCheckBox(tr("Give trackball a spin with\neach mouse-movement"));

    vboxLayout->addWidget(rotateAroundX_);
    vboxLayout->addWidget(rotateAroundY_);
    vboxLayout->addWidget(rotateAroundZ_);
//    vboxLayout->addWidget(continueSpin_);
    motionBox_->setLayout(vboxLayout);
    cameraLayout->addWidget(motionBox_);

    frustumBox_ = new QGroupBox(tr("Frustum parameters"));
    perspectiveBox_ = new QGroupBox(tr("Perspective projection parameters"));
    QHBoxLayout* nearfarLay = new QHBoxLayout();

    comboProjection_ = new QComboBox();
    comboProjection_->addItem(tr("Orthogonal"));
    comboProjection_->addItem(tr("Perspective"));
    comboProjection_->addItem(tr("Frustum"));

    leftProp_   = new FloatProperty("frust.left", "Left", -1.f, -10.f, 10.f);
    rightProp_  = new FloatProperty("frust.right", "Right", 1.f, -10.f, 10.f);
    bottomProp_ = new FloatProperty("frust.bottom", "Bottom", -1.f, -10.f, 10.f);
    topProp_    = new FloatProperty("frust.top", "Top", 1.f, -10.f, 10.f);
    nearProp_   = new FloatProperty("frust.near", "Near", 0.1f, 0.0001f, 1.f);
    farProp_    = new FloatProperty("frust.far", "Far", 10.f, 0.0, maxDist_);
    fovyProp_   = new FloatProperty("frust.fovy", "Fov", 45.f, 5.f, 175.f);
    ratioProp_  = new FloatProperty("frust.ratio", "Ratio", 1.f, 0.05f, 10.f);

    leftProp_->setStepping(0.0001f);
    leftProp_->setNumDecimals(4);
    rightProp_->setStepping(0.0001f);
    rightProp_->setNumDecimals(4);
    bottomProp_->setStepping(0.0001f);
    bottomProp_->setNumDecimals(4);
    topProp_->setStepping(0.0001f);
    topProp_->setNumDecimals(4);
    nearProp_->setStepping(0.0001f);
    nearProp_->setNumDecimals(4);
    farProp_->setStepping(0.0001f);
    farProp_->setNumDecimals(4);
    fovyProp_->setStepping(0.0001f);
    fovyProp_->setNumDecimals(4);
    ratioProp_->setStepping(0.0001f);
    ratioProp_->setNumDecimals(4);

    FloatPropertyWidget* left = dynamic_cast<FloatPropertyWidget*>(wf.createWidget(leftProp_));
    FloatPropertyWidget* right = dynamic_cast<FloatPropertyWidget*>(wf.createWidget(rightProp_));
    FloatPropertyWidget* bottom = dynamic_cast<FloatPropertyWidget*>(wf.createWidget(bottomProp_));
    FloatPropertyWidget* top = dynamic_cast<FloatPropertyWidget*>(wf.createWidget(topProp_));
    FloatPropertyWidget* nearPlane = dynamic_cast<FloatPropertyWidget*>(wf.createWidget(nearProp_));
    FloatPropertyWidget* farPlane = dynamic_cast<FloatPropertyWidget*>(wf.createWidget(farProp_));
    FloatPropertyWidget* fovy = dynamic_cast<FloatPropertyWidget*>(wf.createWidget(fovyProp_));
    FloatPropertyWidget* ratio = dynamic_cast<FloatPropertyWidget*>(wf.createWidget(ratioProp_));
    FloatPropertyWidget* nearPlane2 = dynamic_cast<FloatPropertyWidget*>(wf.createWidget(nearProp_));
    FloatPropertyWidget* farPlane2 = dynamic_cast<FloatPropertyWidget*>(wf.createWidget(farProp_));

    leftProp_->addWidget(left);
    rightProp_->addWidget(right);
    bottomProp_->addWidget(bottom);
    topProp_->addWidget(top);
    nearProp_->addWidget(nearPlane);
    farProp_->addWidget(farPlane);
    nearProp_->addWidget(fovy);
    farProp_->addWidget(ratio);
    nearProp_->addWidget(nearPlane2);
    farProp_->addWidget(farPlane2);

    nearfarLay->addWidget(new QLabel(tr("Near: ")));
    nearfarLay->addWidget(nearPlane2);
    nearfarLay->addWidget(new QLabel(tr("Far: ")));
    nearfarLay->addWidget(farPlane2);

    QHBoxLayout* projectionLayout = new QHBoxLayout();

    //QVBoxLayout* frustLayout = new QVBoxLayout();
    QGridLayout* frustLayout = new QGridLayout();
    frustLayout->addWidget(new QLabel(tr("Left: ")), 0, 0);
    frustLayout->addWidget(left, 0, 1);
    frustLayout->addWidget(new QLabel(tr("Right: ")), 0, 2);
    frustLayout->addWidget(right, 0, 3);
    frustLayout->addWidget(new QLabel(tr("Bottom: ")), 1, 0);
    frustLayout->addWidget(bottom, 1, 1);
    frustLayout->addWidget(new QLabel(tr("Top: ")), 1, 2);
    frustLayout->addWidget(top, 1, 3);
    frustLayout->addWidget(new QLabel(tr("Near: ")), 2, 0);
    frustLayout->addWidget(nearPlane, 2, 1);
    frustLayout->addWidget(new QLabel(tr("Far: ")), 2, 2);
    frustLayout->addWidget(farPlane, 2, 3);


    frustumBox_->setLayout(frustLayout);

    QVBoxLayout* perspectiveLayout = new QVBoxLayout();
    perspectiveLayout->addWidget(new QLabel(tr("Field of Vision: ")));
    perspectiveLayout->addWidget(fovy);
    perspectiveLayout->addWidget(new QLabel(tr("Side Ratio: ")));
    perspectiveLayout->addWidget(ratio);
    perspectiveLayout->addLayout(nearfarLay);
    perspectiveBox_->setLayout(perspectiveLayout);

    projectionLayout->addWidget(perspectiveBox_);
    projectionLayout->addWidget(frustumBox_);
    frustumBox_->hide();

    QPushButton* resetProjButton = new QPushButton("Reset Projection to default");
    projLayout->addWidget(comboProjection_);
    projLayout->addLayout(projectionLayout);
    projLayout->addWidget(resetProjButton);

    connect(left, SIGNAL(valueChanged(double)), this, SLOT(frustumChangeLeft(double)));
    connect(right, SIGNAL(valueChanged(double)), this, SLOT(frustumChangeRight(double)));
    connect(bottom, SIGNAL(valueChanged(double)), this, SLOT(frustumChangeBottom(double)));
    connect(top, SIGNAL(valueChanged(double)), this, SLOT(frustumChangeTop(double)));
    connect(nearPlane, SIGNAL(valueChanged(double)), this, SLOT(frustumChangeNear(double)));
    connect(farPlane, SIGNAL(valueChanged(double)), this, SLOT(frustumChangeFar(double)));
    connect(nearPlane2, SIGNAL(valueChanged(double)), this, SLOT(frustumChangeNear(double)));
    connect(farPlane2, SIGNAL(valueChanged(double)), this, SLOT(frustumChangeFar(double)));
    connect(fovy, SIGNAL(valueChanged(double)), this, SLOT(fovChange(double)));
    connect(ratio, SIGNAL(valueChanged(double)), this, SLOT(ratioChange(double)));
    connect(resetProjButton, SIGNAL(clicked()), this, SLOT(resetCameraProjection()));

    // Group box for trackball
    trackballBox_ = new QGroupBox(tr("Load/Save Camera Settings"));
    QHBoxLayout* hboxLayout2 = new QHBoxLayout();

    hboxLayout2->addWidget(buRestoreTrackball_ = new QToolButton(0));
    hboxLayout2->addSpacing(10);
    hboxLayout2->addWidget(buSaveTrackball_ = new QToolButton(0));
    hboxLayout2->addSpacing(10);
    buRestoreTrackball_->setIcon(QIcon(":/icons/open.png"));
    buRestoreTrackball_->setToolTip(tr("Load camera settings from a file"));
    buSaveTrackball_->setIcon(QIcon(":/icons/save.png"));
    buSaveTrackball_->setToolTip(tr("Save camera settings to a file"));
    hboxLayout2->addStretch();

    trackballBox_->setLayout(hboxLayout2);
    cameraLayout->addWidget(trackballBox_);

    cameraLayout->addStretch();

    setLayout(mainLayout);

}

void CameraWidget::positionChange(FloatVec3Property::ElemType pos) {
    cameraProp_->setPosition(pos);
}

void CameraWidget::focusChange(FloatVec3Property::ElemType pos) {
    cameraProp_->setFocus(pos);
}

void CameraWidget::upChange(FloatVec3Property::ElemType pos) {
    cameraProp_->setUpVector(pos);
}

void CameraWidget::createConnections() {
    connect(rotateAroundX_,              SIGNAL(toggled(bool)),     this, SLOT(enableX(bool)));
    connect(rotateAroundY_,              SIGNAL(toggled(bool)),     this, SLOT(enableY(bool)));
    connect(rotateAroundZ_,              SIGNAL(toggled(bool)),     this, SLOT(enableZ(bool)));
    connect(continueSpin_,               SIGNAL(toggled(bool)),     this, SLOT(enableContSpin(bool)));

    connect(comboOrientation_,   SIGNAL(currentIndexChanged(int)),    this, SLOT(orientationChanged(int)));
    connect(slDistance_,         SIGNAL(valueChanged(int)), this, SLOT(distanceSliderChanged(int)));
    connect(slDistance_,         SIGNAL(sliderPressed()),   this, SLOT(distanceSliderPressed()));
    connect(slDistance_,         SIGNAL(sliderReleased()),  this, SLOT(distanceSliderReleased()));

    connect(comboProjection_,       SIGNAL(currentIndexChanged(int)), this, SLOT(switchProjection(int)));

    connect(buRestoreTrackball_, SIGNAL(clicked()),     this, SLOT(restoreCamera()));
    connect(buSaveTrackball_,    SIGNAL(clicked()),     this, SLOT(saveCameraToDisk()));
}

void CameraWidget::toAbove() {
    quat q = quat(0.0f, 0.0f, 0.0f, 1.0f);
    applyOrientation(q);
}

void CameraWidget::toBelow() {
    quat q = quat(1.0f, 0.f, 0.f, 0.f);
    applyOrientation(q);
}

void CameraWidget::toBehind() {
    // This is not very pretty, admittedly.  The quaternion should be (0, 0.5*sqrt(2), 0.5*sqrt(2), 0)
    // to be precise, but that seems to be some kind of edge case that breaks the modelview-matrix which
    // can also not be reached by turning the trackball by mouse.  So, we use a quaternion that is approximately
    // the mathematically correct one; the difference is so small that one cannot see any.
    quat q = normalize(quat(0.00306279f, 0.710406f, 0.708503f, 0.00167364f));
    applyOrientation(q);
}

void CameraWidget::toFront() {
    const float c = 0.5f * sqrtf(2.0f);
    quat q = quat(c, 0.0f, 0.0f, c);
    applyOrientation(q);
}

void CameraWidget::toLeft() {
    quat q = quat(-0.5f, -0.5f, -0.5f, -0.5f);
    applyOrientation(q);
}

void CameraWidget::toRight() {
    quat q = quat(-0.5f, 0.5f, 0.5f, -0.5f);
    applyOrientation(q);
}

void CameraWidget::applyOrientation(const quat& q) {
    std::vector<float> keyframe;
    keyframe.push_back(q.x);
    keyframe.push_back(q.y);
    keyframe.push_back(q.z);
    keyframe.push_back(q.w);
    keyframe.push_back(slDistance_->value() / CAM_DIST_SCALE_FACTOR);

    applyOrientationAndDistanceAnimated(keyframe);
}

void CameraWidget::updateDistance() {
    dist_ = slDistance_->value() / CAM_DIST_SCALE_FACTOR;
    slDistance_->blockSignals(true);
    track_->zoomAbsolute(dist_);
    slDistance_->blockSignals(false);

    cameraProp_->invalidate();
    cameraProp_->notifyChange();
}

void CameraWidget::orientationChanged(int index) {
    cameraPosition_->set(cameraProp_->get().getPosition());
    focusVector_->set(cameraProp_->get().getFocus());
    upVector_->set(cameraProp_->get().getUpVector());
    cameraPosition_->updateWidgets();
    switch (index) {
    case 1:
        // axial (top to bottom)
        toAbove();
        break;
    case 2:
        // coronal (front to back)
        toFront();
        break;
    case 3:
        // sagittal (volume's right to left)
        toRight();
        break;
    case 4:
        // axial bottom (bottom to top)
        toBelow();
        break;
    case 5:
        // coronal back (back to front)
        toBehind();
        break;
    case 6:
        // sagittal left (volume's left to right)
        toLeft();
        break;
    }
    cameraProp_->notifyChange();
}

void CameraWidget::distanceSliderChanged(int value) {
    if (value != slDistance_->value()) {
        slDistance_->blockSignals(true);
        slDistance_->setValue(value);
        slDistance_->blockSignals(false);
    }
    updateDistance();
}

void CameraWidget::distanceSliderPressed() {
    cameraProp_->toggleInteractionMode(true, slDistance_);
    updateDistance();
}

void CameraWidget::distanceSliderReleased() {
    cameraProp_->toggleInteractionMode(false, slDistance_);
}

void CameraWidget::timerEvent(QTimerEvent* /*event*/) {

    if (!track_)
        return;

    if (rotateX_ || rotateY_ || rotateZ_) {
         continueSpin_->setCheckState(Qt::Unchecked);
//         painter_->setTrackballContinousSpin(false);
     }

    if (rotateX_)
        track_->rotate(tgt::vec3(1.f, 0.f, 0.f), 0.05f);

    if (rotateY_)
        track_->rotate(tgt::vec3(0.f, 1.f, 0.f), 0.05f);

    if (rotateZ_)
        track_->rotate(tgt::vec3(0.f, 0.f, 1.f), 0.05f);

    if (rotateX_ || rotateY_ || rotateZ_) {
        if (comboOrientation_->currentIndex() != 0)
            comboOrientation_->setCurrentIndex(0);
    }

    cameraProp_->invalidate();
    cameraProp_->notifyChange();
}

void CameraWidget::checkCameraState() {

    tgt::Frustum f = cameraProp_->get().getFrustum();

    if (comboProjection_->currentIndex() == 1) {
        f.setFovy(std::min(fovyProp_->getMaxValue(), std::max(fovyProp_->getMinValue(), f.getFovy())));
        f.setRatio(std::min(ratioProp_->getMaxValue(), std::max(ratioProp_->getMinValue(), f.getRatio())));
        fovyProp_->set(f.getFovy());
        ratioProp_->set(f.getRatio());
    }

    f.setLeft(std::min(leftProp_->getMaxValue(), std::max(leftProp_->getMinValue(), f.getLeft())));
    f.setRight(std::min(rightProp_->getMaxValue(), std::max(rightProp_->getMinValue(), f.getRight())));
    f.setBottom(std::min(bottomProp_->getMaxValue(), std::max(bottomProp_->getMinValue(), f.getBottom())));
    f.setTop(std::min(topProp_->getMaxValue(), std::max(topProp_->getMinValue(), f.getTop())));
    f.setNearDist(std::min(nearProp_->getMaxValue(), std::max(nearProp_->getMinValue(), f.getNearDist())));
    f.setFarDist(std::min(farProp_->getMaxValue(), std::max(farProp_->getMinValue(), f.getFarDist())));

    leftProp_->set(f.getLeft());
    rightProp_->set(f.getRight());
    bottomProp_->set(f.getBottom());
    topProp_->set(f.getTop());
    nearProp_->set(f.getNearDist());
    farProp_->set(f.getFarDist());

    cameraPosition_->set(cameraProp_->get().getPosition());
    focusVector_->set(cameraProp_->get().getFocus());
    upVector_->set(cameraProp_->get().getUpVector());

    if (track_) {

        if (track_->getCamera() != cameraProp_)
            track_->setCamera(cameraProp_);

        if (!track_->getCamera())
            return;

        // update distance slider
        slDistance_->blockSignals(true);
        slDistance_->setValue(tgt::iround(CAM_DIST_SCALE_FACTOR*track_->getCenterDistance()));
        slDistance_->blockSignals(false);

        // update orientation box
        const float MAX_DEVIATION = 1.0e-4f;
        tgt::vec3 look = track_->getCamera()->get().getLook();
        comboOrientation_->blockSignals(true);
        if (abs(tgt::dot(AXIAL_VIEW, look) - 1.0f) < MAX_DEVIATION)
            comboOrientation_->setCurrentIndex(1);
        else if (abs(tgt::dot(CORONAL_VIEW, look) - 1.0f) < MAX_DEVIATION)
            comboOrientation_->setCurrentIndex(2);
        else if (abs(tgt::dot(SAGITTAL_VIEW, look) - 1.0f) < MAX_DEVIATION)
            comboOrientation_->setCurrentIndex(3);
        else if (abs(tgt::dot(AXIAL_INV_VIEW, look) - 1.0f) < MAX_DEVIATION)
            comboOrientation_->setCurrentIndex(4);
        else if (abs(tgt::dot(CORONAL_INV_VIEW, look) - 1.0f) < MAX_DEVIATION)
            comboOrientation_->setCurrentIndex(5);
        else if (abs(tgt::dot(SAGITTAL_INV_VIEW, look) - 1.0f) < MAX_DEVIATION)
            comboOrientation_->setCurrentIndex(6);
        else
            comboOrientation_->setCurrentIndex(0);
        comboOrientation_->blockSignals(false);

        comboProjection_->blockSignals(true);
        comboProjection_->setCurrentIndex((int)cameraProp_->get().getProjectionMode());
        comboProjection_->blockSignals(false);

        if (comboProjection_->currentIndex() == 1) {
            frustumBox_->hide();
            perspectiveBox_->show();
        }
        else {
            perspectiveBox_->hide();
            frustumBox_->show();
        }
    }


}

void CameraWidget::applyOrientationAndDistanceAnimated(std::vector<float> keyframe) {
    tgtAssert(track_, "No trackball");
    CameraProperty* camera = track_->getCamera();

    tgt::quat newQuat;
    newQuat.x = keyframe[0];
    newQuat.y = keyframe[1];
    newQuat.z = keyframe[2];
    newQuat.w = keyframe[3];
    float newDist = keyframe[4];

    float t = 0.1f;

    tgt::quat oldQuat = camera->get().getQuat();
    float oldDist = camera->get().getFocalLength();
    resetCameraPosition();
    tgt::quat initQuat = camera->get().getQuat();
    initQuat.invert();

    cameraProp_->toggleInteractionMode(true, comboOrientation_);
    // todo: use timer
    for (int i = 0; i <= 9; ++i) {
        tgt::quat tmp = tgt::slerpQuat(oldQuat, newQuat, std::min(t, 1.f));
        float tmpDist = t*(newDist - oldDist) + oldDist;
        resetCameraPosition();
        track_->zoomAbsolute(tmpDist);
        track_->rotate(initQuat);
        track_->rotate(tmp);

        cameraProp_->invalidate();
        cameraProp_->notifyChange();
        qApp->processEvents();

        t += 0.1f;
    }
    cameraProp_->toggleInteractionMode(false, comboOrientation_);

    resetCameraPosition();
    track_->zoomAbsolute(newDist);
    track_->rotate(initQuat);
    track_->rotate(newQuat);

    cameraProp_->notifyChange();
}

void CameraWidget::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    checkCameraState();
}

void CameraWidget::enableContSpin(bool /*b*/) {
//    painter_->setTrackballContinousSpin(b);
}

void CameraWidget::enableX(bool b) {
    rotateX_ = b;
    setTimerState();
}

void CameraWidget::enableY(bool b) {
    rotateY_ = b;
    setTimerState();
}

void CameraWidget::enableZ(bool b) {
    rotateZ_ = b;
    setTimerState();
}

void CameraWidget::switchProjection(int m) {
    tgt::Camera cam = cameraProp_->get();
    cam.setProjectionMode((tgt::Camera::ProjectionMode)m);
    if(m == 1) {
        frustumBox_->hide();
        perspectiveBox_->show();
        fovyProp_->set(std::min(fovyProp_->getMaxValue(), std::max(fovyProp_->getMinValue(), cam.getFovy())));
        ratioProp_->set(std::min(ratioProp_->getMaxValue(), std::max(ratioProp_->getMinValue(), cam.getRatio())));
        fovyProp_->updateWidgets();
        ratioProp_->updateWidgets();
    } else {
        perspectiveBox_->hide();
        frustumBox_->show();
    }
    cameraProp_->set(cam);
    cameraProp_->notifyChange();
}

void CameraWidget::frustumChangeLeft(double v) {
    tgt::Camera cam = cameraProp_->get();
    cam.setFrustLeft(v);
    cameraProp_->set(cam);
    cameraProp_->notifyChange();
}

void CameraWidget::frustumChangeRight(double v) {
    tgt::Camera cam = cameraProp_->get();
    cam.setFrustRight(v);
    cameraProp_->set(cam);
    cameraProp_->notifyChange();
}

void CameraWidget::frustumChangeBottom(double v) {
    tgt::Camera cam = cameraProp_->get();
    cam.setFrustBottom(v);
    cameraProp_->set(cam);
    cameraProp_->notifyChange();
}

void CameraWidget::frustumChangeTop(double v) {
    tgt::Camera cam = cameraProp_->get();
    cam.setFrustTop(v);
    cameraProp_->set(cam);
    cameraProp_->notifyChange();
}

void CameraWidget::frustumChangeNear(double v) {
    tgt::Camera cam = cameraProp_->get();
    cam.setNearDist(v);
    cameraProp_->set(cam);
    cameraProp_->notifyChange();
}

void CameraWidget::frustumChangeFar(double v) {
    tgt::Camera cam = cameraProp_->get();
    cam.setFarDist(v);
    cameraProp_->set(cam);
    cameraProp_->notifyChange();
}

void CameraWidget::fovChange(double v) {
    tgt::Camera cam = cameraProp_->get();
    cam.setFovy(v);
    cameraProp_->set(cam);
    cameraProp_->notifyChange();
}

void CameraWidget::ratioChange(double v) {
    tgt::Camera cam = cameraProp_->get();
    cam.setRatio(v);
    cameraProp_->set(cam);
    cameraProp_->notifyChange();
}

CameraWidget::SerializationResource::SerializationResource(XmlSerializerBase* serializer,
                                                           const std::string& filename,
                                                           const std::ios_base::openmode& openMode)
    : stream_(filename.c_str(), openMode)
{
    serializer->setUseAttributes(true);
}

CameraWidget::SerializationResource::~SerializationResource() {
    stream_.close();
}

std::fstream& CameraWidget::SerializationResource::getStream() {
    return stream_;
}

void CameraWidget::saveCameraToDisk() {
    QFileDialog dlg(this, tr("Save Camera Position As"), "", tr("Voreen Camera Files (*.vcm)"));
    dlg.setDefaultSuffix("vcm");

    std::string chosenFileName;
    if (dlg.exec())
        chosenFileName = dlg.selectedFiles().first().toStdString();

    if (!chosenFileName.empty())
        saveCameraToDisk(chosenFileName);
}

void CameraWidget::saveCameraToDisk(std::string fn) {
    XmlSerializer s(fn);
    SerializationResource resource(&s, fn, std::ios_base::out);

    s.serialize("Property", *cameraProp_);
    s.write(resource.getStream());
}

void CameraWidget::restoreCamera(std::string fn) {
    XmlDeserializer d(fn);
    SerializationResource resource(&d, fn, std::ios_base::in);

    d.read(resource.getStream());
    d.deserialize("Property", *cameraProp_);

    cameraProp_->invalidate();
    checkCameraState();
    cameraProp_->notifyChange();
}

void CameraWidget::restoreCamera() {
    QString fn = QFileDialog::getOpenFileName(this, tr("Open Camera Position"), ".",
                                              tr("Voreen Camera Files (*.vcm)"));
    std::string s = fn.toStdString();
    if (!s.empty())
        restoreCamera(s);
}

void CameraWidget::setTimerState() {
    if (timer_->isActive()) {
        if (!(rotateX_ || rotateY_ || rotateZ_)) {
            cameraProp_->toggleInteractionMode(false, this);
            timer_->stop();
        }
    }
    else {
        if (rotateX_ || rotateY_ || rotateZ_) {
            cameraProp_->toggleInteractionMode(true, this);
            timer_->start(25, this);
        }
    }
}

void CameraWidget::updateFromCamera() {
    checkCameraState();
}

void CameraWidget::resetCameraPosition() {
    // all trackball operations assume an initial view along the negative z-axis with the
    // y-axis as up vector
    tgt::Camera cam(vec3(0.f, 0.f, 1.f), vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
    cam.setFrustum(cameraProp_->get().getFrustum());
    cam.setProjectionMode(cameraProp_->get().getProjectionMode());
    cameraProp_->set(cam);
    cameraPosition_->set(cameraProp_->get().getPosition());
    cameraProp_->notifyChange();
}

void CameraWidget::resetCameraProjection() {
    tgt::Camera cam = cameraProp_->get();
    cam.setProjectionMode(tgt::Camera::PERSPECTIVE);
    cam.setNearDist(0.1f);
    cam.setFovy(45.f);
    cam.setRatio(1.f);
    cameraProp_->set(cam);
    cameraProp_->notifyChange();
}

} // namespace voreen
