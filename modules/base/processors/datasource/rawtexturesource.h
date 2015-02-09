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

#ifndef VRN_RAWTEXTURESOURCE_H
#define VRN_RAWTEXTURESOURCE_H

#include "voreen/core/processors/renderprocessor.h"
#include "voreen/core/properties/filedialogproperty.h"
#include "voreen/core/properties/buttonproperty.h"
#include "voreen/core/properties/vectorproperty.h"
#include "voreen/core/properties/optionproperty.h"
#include "voreen/core/voreenapplication.h"

using tgt::Texture;

namespace voreen {

/**
 * Loads a texture from disk and supplies it.
 */
class RawTextureSource : public Processor {

public:
    RawTextureSource();
    ~RawTextureSource();
    virtual Processor* create() const;

    virtual std::string getClassName() const { return "RawTextureSource"; }
    virtual std::string getCategory() const  { return "Data Source"; }
    virtual CodeState getCodeState() const   { return CODE_STATE_STABLE; }

    /**
     * Loads the texture specified by filename.
     *
     * @note The processor must have been initialized
     *       before calling this function.
     */
    void loadTexture();

protected:
    virtual void beforeProcess();
    virtual void process();
    virtual void initialize() throw (tgt::Exception);
    virtual void deinitialize() throw (tgt::Exception);

    /**
     * Clears the loaded texture. Processor needs to be initialized when calling this function.
     */
    void clearTexture();

    GenericPort<tgt::Texture> texturePort_;
    FileDialogProperty textureFile_;  ///< Path of the loaded texture file.
    ButtonProperty loadTexButton_;    ///< Loads the texture.
    IntVec2Property textureSize_;     ///< Read-only property providing the dimensions of the loaded image.
    GLEnumOptionProperty textureFormat_;
    GLEnumOptionProperty textureDataType_;

    tgt::Texture* texture_;         ///< The 2D texture encapsulating the image file.

    /// Determines whether the processor owns the texture, which
    /// is only the case, if it has loaded it.
    bool textureOwner_;

    bool forceReload_;
    static const std::string loggerCat_;
};

} // namespace

#endif
