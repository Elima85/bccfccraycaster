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

#ifndef VRN_ANIMATIONEXPORTWIDGET_H
#define VRN_ANIMATIONEXPORTWIDGET_H

#include "voreen/core/properties/cameraproperty.h"
#include "voreen/core/animation/animation.h"
#include "voreen/core/utils/voreenpainter.h"
#ifdef VRN_MODULE_FFMPEG
    #include "modules/ffmpeg/videoencoder/videoencoder.h"
#endif
#include "tgt/qt/qtcanvas.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QMenu>
#include <QPushButton>
#include <QToolButton>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTabWidget>
#include <QTimeEdit>
#include <QVBoxLayout>
#include <QDialog>
#include <map>

namespace voreen {

class NetworkEvaluator;

/**
 * This Class creates a widget for Animation to Video or Screencapture
 */
class AnimationExportWidget : public QDialog {
Q_OBJECT
public:
    AnimationExportWidget(QWidget* parent, Animation* animation, NetworkEvaluator* network, float duration, int startframe, int endframe, float);
    ~AnimationExportWidget();
    void networkChanged();

private slots:
    void controlledCanvasChanged(int index);
    void videoSetup();
    void recordAnimationFrameSeq();
    void recordAnimationVideo();
    void saveSpecificFrame();
    void setWidgetState();
    void updateFramesSpinBox();
    void setFirstFrameAsSpecificFrame();
    void setLastFrameAsSpecificFrame();

private:
    void closeEvent(QCloseEvent* );
    void recordAnimation(bool recordVideo);
    void startRendering();
    void endRendering();
    void renderingStep();
    void rotateView(CameraProperty* camProp, float angle, const tgt::vec3& axis, const tgt::vec3& camPos, const tgt::vec3& camLook);
    void createWidgets();
    void createConnections();
    void refreshComboBoxes();

    Animation* animation_;
    NetworkEvaluator* network_;
    CameraProperty* camForCubeMaps_;
    CameraProperty* camForStereoImages_;
    float duration_;
    int startframe_;
    int endframe_;

    tgt::QtCanvas* canvas_;
    VoreenPainter* painter_;
    tgt::ivec2 canvasSize_;
    bool renderingVideo_;
    bool renderSpecificFrame_;
    std::string recordPathName_;
    float fpsFactor_;
    int currentFrame_;
    int fps_;
    enum RenderState { Snapshot, Recording, Inactive };
    RenderState renderState_;
    typedef std::map<tgt::QtCanvas*, std::string> CanvasMap;
    typedef std::map<CameraProperty*, std::string> CameraPropertyMap;

    QGroupBox* createAnimationRenderBox(QWidget* parent);

#ifdef VRN_MODULE_FFMPEG
    VideoEncoder ffmpegEncoder_;
    QComboBox* preset_;
    QSpinBox* bitrate_;
    QDialog* createVideoSetupDialog(QWidget* parent, int curPreset, int curBitrate);
#endif

    CanvasMap allCanvases_;
    CameraPropertyMap allCameraPropertys_;
    QGroupBox* renderBox_;
    QSpinBox* spinRecordingFPS_;
    QSpinBox* spinWidth_;
    QSpinBox* spinHeight_;
    QSpinBox* specificFrame_;
    QDoubleSpinBox* eyeSeparation_;
    QLabel* eyeSeparationLabel_;
    QTimeEdit* spinStartTime_;
    QTimeEdit* spinEndTime_;
    QComboBox* comboCanvases_;
    QComboBox* comboCameras_;
    QComboBox* comboCamerasCubeMap_;
    QComboBox* comboCamerasStereo_;
    QVBoxLayout* mainLayout_;
    QPushButton* saveAsFrameSequenceButton_;
    QPushButton* saveAsVideoButton_; // will be disabled in case ffmpeg is not available
    QPushButton* saveSpecificFrame_;
    QToolButton* setSpecificFrameAsFirst_;
    QToolButton* setSpecificFrameAsLast_;
    QPushButton* videoSetupButton_;
    QCheckBox* cubeMapImages_;
    QCheckBox* cubeMapFront_;
    QCheckBox* cubeMapBack_;
    QCheckBox* cubeMapLeft_;
    QCheckBox* cubeMapRight_;
    QCheckBox* cubeMapTop_;
    QCheckBox* cubeMapBottom_;
    QCheckBox* stereoImages_;
    
};

}

#endif // VRN_ANIMATIONEXPORTWIDGET_H
