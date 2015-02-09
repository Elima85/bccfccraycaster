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

#ifndef VRN_APPLICATION_H
#define VRN_APPLICATION_H

#include "voreen/core/voreencoredefine.h"
#include "voreen/core/properties/property.h"
#include "voreen/core/properties/propertyowner.h"
#include "voreen/core/utils/cmdparser/commandlineparser.h"

#include <string>
#include "tgt/logmanager.h"
#include "tgt/event/eventlistener.h"
#include "tgt/event/eventhandler.h"
#include "tgt/timer.h"

namespace tgt {
    class EventHandler;
}

namespace voreen {

class Processor;
class ProcessorWidget;
class Property;
class PropertyWidget;
class VoreenModule;
class NetworkEvaluator;
class ProgressBar;
class RemoteController;

/**
 * Represents basic properties of a Voreen application. There should only be one instance of
 * this class, which can be access via the static method app().
 */
class VRN_CORE_API VoreenApplication : private tgt::EventListener {
    friend class Processor;

public:
    /// Features used in this application
    enum ApplicationFeatures {
        APP_NONE                =  0,       ///< nothing
        APP_SHADER              =  1,       ///< detect shader path
        APP_DATA                =  2,       ///< detect data path
        APP_CONSOLE_LOGGING     =  4,       ///< activate logging to the console
        APP_HTML_LOGGING        =  8,       ///< activate logging to a HTML file
        APP_AUTOLOAD_MODULES    =  16,      ///< loads all modules listed in the module registration header
                                            ///  'moduleregistration.h' or 'gen_moduleregistration.h', resp.
        APP_PROCESSOR_WIDGETS   =  32,      ///< activate creation of processor widgets
        APP_PROPERTY_WIDGETS    =  64,      ///< activate creation of property widgets
        APP_ALL                 =  0xFFFF,  ///< all features
        APP_DEFAULT = APP_ALL &~ APP_HTML_LOGGING   ///< default: all features except HTML logging
    };

    /**
     * @param name Short name of the application in lowercase ("voreenve")
     * @param displayName Nice-looking name of the application ("VoreenVE")
     * @param argc Number of arguments as retrieved from main()
     * @param argv Argument vector as retrieved from main()
     * @param appType Features to activate
     * @param autoLoadModules if true, the application loads all modules listed in
     *  the module registration header ('moduleregistration.h' or 'gen_moduleregistration.h' resp.).
     *  Otherwise, modules need to be instantiated "manually" and passed to addModule().
     */
    VoreenApplication(const std::string& name, const std::string& displayName,
                      int argc, char** argv, ApplicationFeatures appType = APP_DEFAULT);

    /**
     * Calls deinit().
     */
    virtual ~VoreenApplication();

    /**
     * Allows access to the global instance of this class.
     */
    static VoreenApplication* app();

    std::string getName() const { return name_; }
    std::string getDisplayName() const { return displayName_; }

    CommandlineParser* getCommandLineParser() { return &cmdParser_; }

    /**
     * Overwrite this method to add commands to the CommandlineParser.
     */
    virtual void prepareCommandParser();

    /**
     * Assigns the network evaluator.
     *
     * Is internally used for scheduled network processing.
     * @see scheduleNetworkProcessing
     */
    void setNetworkEvaluator(NetworkEvaluator* evaluator);

    /**
     * Returns the network evaluator.
     */
    NetworkEvaluator* getNetworkEvaluator() const;

#ifdef VRN_REMOTE_CONTROL
    RemoteController* getRemoteController() const;
#endif

    /**
     * Performs basic initializations as controlled by appType_, which do not require OpenGL access:
     *  - initialize tgt
     *  - execute command parser
     *  - start logging
     *  - detect paths
     *  - instantiates and registers the module classes
     *
     * @note This function should be called right after object construction,
     *  especially before calling initGL().
     */
    virtual void initialize();

    /**
     * Deinitializes the application and deletes the module objects,
     * to be called right before object destruction.
     */
    virtual void deinitialize();

    /**
     * Do OpenGL-specific initialization and initialize registered modules.
     *
     * @note init() must be called first.
     *
     * @throws VoreenException if OpenGL initialization failed
     */
    virtual void initializeGL() throw (VoreenException);

    /**
     * Do OpenGL-specific deinitialization and deinitialize all registered modules.
     *
     * @throws VoreenException if OpenGL deinitialization failed.
     */
    virtual void deinitializeGL() throw (VoreenException);


    //
    // Modules
    //

    /**
     * Registers a module.
     */
    void addModule(VoreenModule* module);

    /**
     * Returns all registered modules.
     */
    const std::vector<VoreenModule*>& getModules() const;

    /**
     * Returns the VoreenModule specified by the name or 0 if no such module exists.
     */
    const VoreenModule* getModule(const std::string& moduleName) const;

    //
    // Factory methods
    //

    /**
     * Creates and returns a ProcessorWidget for the passed processor by delegation to 
     * the ProcessorWidgetFactories of the registered modules. If no factory is able 
     * to create a widget for the passed processor, the null pointer is returned.
     *
     * @note The creation of ProcessorWidgets can be disabled via the ApplicationFeatures.
     *  In this case, the method always returns null.
     */
    virtual ProcessorWidget* createProcessorWidget(Processor* processor) const; 

    /**
     * Creates and returns a PropertyWidget for the passed property by delegation to 
     * the PropertyWidgetFactories of the registered modules. If no factory is able 
     * to create a widget for the passed property, the null pointer is returned.
     *
     * @note The creation of PropertyWidgets can be disabled via the ApplicationFeatures.
     *  In this case, the method always returns null.
     */
    virtual PropertyWidget* createPropertyWidget(Property* property) const; 

    /**
     * Factory method for timers.
     *
     * @param handler The event handler that will be used
     *  for broadcasting the timer events. Must not be null.
     *
     * @note You have to override this function in a toolkit specific subclass
     *  in order to actual create a timer. The standard implementation returns
     *  the null pointer.
     */
    virtual tgt::Timer* createTimer(tgt::EventHandler* handler) const;

    /**
     * Factory method for progress dialogs.
     *
     * @note You have to override this function in a toolkit specific subclass
     *  in order to actual create a progress dialog. The standard implementation returns
     *  the null pointer.
     */
    virtual ProgressBar* createProgressDialog() const;

    ///
    /// Paths
    ///

    /**
     * Returns the application's base path as detected by \sa init().
     */
    std::string getBasePath() const;

    /**
     * Constructs an absolute path consisting of the cache directory
     * (typically voreen/data/cache) and the given filename.
     */
    std::string getCachePath(const std::string& filename = "") const;

    /**
     * Constructs an absolute path consisting of the data directory (typically voreen/data) and
     * the given filename.
     */
    std::string getDataPath(const std::string& filename = "") const;

    /**
     * Constructs an absolute path consisting of the volume data directory and the given
     * filename.
     */
    std::string getVolumePath(const std::string& filename = "") const;

    /**
     * Constructs an absolute path consisting of the shader source directory and the given
     * filename.
     */
    std::string getShaderPath(const std::string& filename = "") const;

    /**
     * Constructs an absolute path consisting of Snapshot directory and the given
     * filename.
     */
    std::string getSnapshotPath(const std::string& filename = "") const;

    /**
     * Constructs an absolute path consisting of the font directory (typically
     * voreen/data/fonts) and the given filename.
     */
    std::string getFontPath(const std::string& filename = "") const;

    /**
    * Constructs an absolute path consisting of network file directory (typically
    * voreen/data/networks) and the given filename.
    */
    std::string getNetworkPath(const std::string& filename = "") const;

    /**
    * Constructs an absolute path consisting of workspace file directory (typically
    * voreen/data/workspaces) and the given filename.
    */
    std::string getWorkspacePath(const std::string& filename = "") const;

    /**
     * Constructs an absolute path consisting of script directory (typically
     * voreen/data/scripts) and the given filename.
     */
    std::string getScriptPath(const std::string& filename = "") const;

    /**
     * Constructs an absolute path consisting of transfer function directory (typically
     * voreen/data/transferfuncs) and the given filename.
     */
    std::string getTransFuncPath(const std::string& filename = "") const;

    /**
    * Constructs an absolute path consisting of the textures directory (typically
    * voreen/data/textures) and the given filename.
    */
    std::string getTexturePath(const std::string& filename = "") const;

    /**
     * Constructs an absolute path consisting of the module source directory
     * (typically voreen/modules) and the given filename.
     */
    std::string getModulePath(const std::string& filename = "") const;

    /**
     * Constructs an absolute path consisting of the temporary directory (typically
     * voreen/data/tmp) and the given filename.
     */
    std::string getTemporaryPath(const std::string& filename = "") const;

    /**
     * Constructs an absolute path consisting of the documentation directory (typically
     * voreen/doc) and the given filename.
     */
    std::string getDocumentationPath(const std::string& filename = "") const;

    /**
     * Constructs an absolute path consisting of the documents directory (typically
     * "C:\Documents and Settings\user\Documents" on Windows and $HOME on unix) and the given
     * filename.
     */
    std::string getDocumentsPath(const std::string& filename = "") const;

#ifdef __APPLE__
    /**
     * Constructs an absolute path consisting of the Mac application bundle's resource
     * directory (path Contents/Resources within the bundle) and the given filename.
     */
    std::string getAppBundleResourcesPath(const std::string& filename = "") const;
#endif

    /**
     * Returns the application's log level, which Info by default
     * but can overwritten via command line.
     */
    tgt::LogLevel getLogLevel() const;

    void setLogLevel(tgt::LogLevel logLevel);

protected:
    /**
     * This function triggers a non-blocking network processing,
     * i.e., the assigned NetworkEvaluator's process() function is called
     * after the current call stack has been completed (event queue concept).
     *
     * This function is called by each end processor when it receives an invalidation.
     *
     * @note Since internally a tgt::Timer is used for scheduling, this
     *  function does only work in a derived class that overrides createTimer().
     *  Otherwise, it is a no-op.
     */
    virtual void scheduleNetworkProcessing();

    static VoreenApplication* app_;

    ApplicationFeatures appFeatures_;
    std::string name_;
    std::string displayName_;
    CommandlineParser cmdParser_;

#ifdef VRN_REMOTE_CONTROL
    RemoteController* remoteController_;
#endif

    std::string basePath_;
    std::string cachePath_;
    std::string dataPath_;
    std::string texturePath_;
    std::string fontPath_;
    std::string volumePath_;
    std::string temporaryPath_;
    std::string documentationPath_;
    std::string documentsPath_;
#ifdef __APPLE__
    std::string appBundleResourcesPath_;
#endif

    tgt::LogLevel logLevel_;
    std::string logFile_;

    bool initialized_;
    bool initializedGL_;

    static const std::string loggerCat_;

private:
    /**
     * Helper function for scheduled network processing.
     * @see scheduleNetworkProcessing
     */
    virtual void timerEvent(tgt::TimeEvent* e);

    std::vector<VoreenModule*> modules_;

    NetworkEvaluator* networkEvaluator_;
    tgt::Timer* schedulingTimer_;       ///< Timer for scheduled network processing.
    tgt::EventHandler eventHandler_;    ///< Local event handler for the scheduling events.
    std::string shaderPath_;

    std::string overrideGLSLVersion_; ///< cmdparser writes the passed shader version string to this
};

} // namespace

#endif
