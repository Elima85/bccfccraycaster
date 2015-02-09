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

#ifndef VRN_PROCESSORLISTWIDGET_H
#define VRN_PROCESSORLISTWIDGET_H

#include <QContextMenuEvent>
#include <QSettings>
#include <QTreeWidget>
#include <QWidget>

#include "voreen/core/processors/processor.h"
#include "voreen/core/processors/processorfactory.h"
#include "voreen/qt/widgets/lineeditresetwidget.h"
#include "voreen/qt/voreenqtdefine.h"

#include <vector>
#include <map>

class QMouseEvent;
class QTreeWidgetItem;
class QTextBrowser;
class QSplitter;
class QContextMenuEvent;

namespace voreen {

class DocumentationWidget;
class VoreenModule;
class Processor;

class VRN_QT_API ProcessorListItem : public QTreeWidgetItem {
public:
    ProcessorListItem(const std::string& id);
    const std::string& getId() { return id_; }
    const std::string& getInfo() { return info_; }
    //void setProcessorInfo(const std:: string& name);

private:
    const std::string id_;
    std::string info_;
};

class ProcessorListTreeWidget;
class Processor;

class VRN_QT_API ProcessorListWidget : public QWidget {
Q_OBJECT
public:
    ProcessorListWidget(QWidget* parent = 0);
    ~ProcessorListWidget();

    void clearInfo();

signals:
    void selectedProcessorForAdding(const QString& processor);

public slots:
    void processorsSelected(const QList<Processor*>& processors);
    void setInfo(const Processor* processor);
    void setInfo(VoreenModule* module);
    //void setInfo(std::string);
    void loadSettings();
    void resetSettings();

protected:
    ProcessorListTreeWidget* tree_;
    QIcon resetIcon_;
    QTextBrowser* info_;
    QSplitter* splitter_;

    QAction* sortByCategory_;            // sort by category
    QAction* sortByModule_;              // sort by module
    QAction* sortByModuleThenCategory_;  // sort by module then category
    
    QAction* showCodeStateExperimental_; // show code state experimental
    QAction* showCodeStateBroken_;       // show code state broken
    QAction* showCodeStateStable_;       // show code state stable
    QAction* showCodeState_;
    QAction* searchDescription_;
    
    std::map<std::string, bool> moduleVisibility_;
    std::map<std::string, QAction*> moduleVisibilityActions_;
    QMenu* moduleVisibilityMenu_;
    
    LineEditResetWidget* edit_;
    DocumentationWidget* documentationWidget_;

    VoreenModule* recentlyUsedModule_;
    const Processor* recentlyUsedProcessor_;

protected slots:
    void reloadInfoText();
    void setInfo();
    void sortMenu();
    void setModuleNameVisibility(std::string, bool);
    void saveSettings();
    void textBrowserRequestedContextMenu(const QPoint& pos);
    void showDescriptionEditor();

signals:
    void sort(int);
    void hideStatus(bool);
    void searchDescription(bool);
    void showModule(QString, bool);

private:
    bool resetSettings_;
};

class VRN_QT_API ProcessorListTreeWidget : public QTreeWidget {
Q_OBJECT
friend class ProcessorListWidget;
public:
    ProcessorListTreeWidget(ProcessorListWidget* processorListWidget, QWidget* parent = 0);

    enum GroupType {
        GroupTypeCategory,
        GroupTypeModule,
        GroupTypeModuleCategory
    };

signals:
    void selectedProcessorForAdding(const QString& processor);

public slots:
    void filter(const QString& text);

    /// Sorts the Processorlist by category, modulename, or modulename then category
    void sort(int);
    void hideStatus(bool);
    void searchDescription(bool);
    void setModuleNameVisibility(QString, bool);

protected:
    void mousePressEvent(QMouseEvent*);

    void setCodeStateIcon(const std::string& classname, QTreeWidgetItem*) const;

private slots:
    void itemHasBeenDoubleClicked(QTreeWidgetItem* item, int column);

private:
    QList<QTreeWidgetItem*> items_;
    ProcessorListWidget* processorListWidget_;

    std::vector<const Processor*> getVisibleProcessors() const;
    
    void sortByCategory();
    void sortByModuleName();
    void sortByModuleThenCategory();

    /**
     * Returns a widget tree representing the category hierarchy of the passed processors. The \p categoryLevel
     * determines which category level to be considered as root level (for recursive calls).
     */
    QList<QTreeWidgetItem*> createCategoryHierarchy(const std::vector<const Processor*>& processors, int categoryLevel) const;

    GroupType sortType_;
    std::string filterText_;
    bool showCodeState_;
    bool searchDescription_;

    std::map<std::string, bool> moduleVisibility_;
    std::map<std::string, std::string> processorDescriptions_;
    std::set<Processor::CodeState> codeState_;

};

} //namespace voreen

#endif // VRN_PROCESSORLISTWIDGET_H
