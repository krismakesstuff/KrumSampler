/*
  ==============================================================================

    KrumFileBrowser.cpp
    Created: 26 Mar 2021 12:46:23pm
    Author:  krisc

  ==============================================================================
*/

#include "KrumFileBrowser.h"
#include "../KrumModuleContainer.h"
#include "../KrumModule/KrumModuleEditor.h"
#include "../PluginEditor.h"
#include "../InfoPanel.h"
#include "../KrumLookAndFeel.h"


//==============================================================================
RecentFilesList::RecentFilesList(KrumFileBrowser& fb, SimpleAudioPreviewer* p)
    : previewer(p), fileBrowser(fb)
{
    listBox.setColour(juce::ListBox::ColourIds::backgroundColourId, Colors::getBrowserBGColor());
    listBox.setRowHeight(Dimensions::rowHeight);
    listBox.setMultipleSelectionEnabled(true);
    //listBox.updateContent();
    addAndMakeVisible(listBox);

    recentValueTree.addListener(this);

}

RecentFilesList::~RecentFilesList() 
{
    DBG("RecentFilesList Destructor Called");
}

void RecentFilesList::valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& removedChild, int indexOfRemoval)
{
    listBox.updateContent();
    fileBrowser.getPanelHeader(KrumFileBrowser::BrowserSections::recent)->animateRemoveFile();
}

void RecentFilesList::resized()
{
    auto area = getLocalBounds();
    listBox.setBounds(area);
}

int RecentFilesList::getNumRows()
{
    return recentValueTree.getNumChildren();
}

void RecentFilesList::listBoxItemClicked(int row, const juce::MouseEvent& e)
{
    if(e.mods.isPopupMenu())
        return;
    
    juce::File file{ getFilePath(row) };
    if (previewer)
    {
        //if auto-play is on, we load the file into the previewer
        if (previewer->isAutoPlayActive())
        {
            if (file != previewer->getCurrentFile())
            {
                previewer->loadFile(file);
            }

            previewer->setWantsToPlayFile(true);
        }
    }
    else
    {
        DBG("Previewer NULL");
    }
}

void RecentFilesList::listBoxItemDoubleClicked(int row, const juce::MouseEvent& e)
{
    if(e.mods.isPopupMenu())
        return;
    
    juce::File file{ getFilePath(row) };

    if (previewer)
    {
        //if auto-play is off, we load the file into the previewer.
        if (!previewer->isAutoPlayActive())
        {
            if (file != previewer->getCurrentFile())
            {
                previewer->loadFile(file);
            }
            previewer->setWantsToPlayFile(true);
        }
    }
    else
    {
        DBG("Previewer NULL");
    }
}

void RecentFilesList::selectedRowsChanged(int lastRowSelected)
{
    // we handle clicked items in the itemClicked method, so this is just for the selection made using the keyboard arrows
    if (isMouseButtonDown())
    {
        return;
    }

    juce::File file{ getFilePath(lastRowSelected) };
    if (previewer && !file.isDirectory() && previewer->isAutoPlayActive())
    {
        if (file != previewer->getCurrentFile())
        {
            previewer->loadFile(file);
        }
        previewer->setWantsToPlayFile(true);   
    }

}


void RecentFilesList::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) 
{
    //int indent = 20 ;
    int spacer = 5;

    //auto klaf = static_cast<KrumLookAndFeel*>(&getLookAndFeel());

    juce::Rectangle<int> area = { spacer, 0, width - spacer, height };
    
    
    juce::Colour fontC = Colors::getBrowserFontColor();

    if (rowIsSelected)
    {
        g.setColour(Colors::getHighlightColor());
        fontC = Colors::getHighlightFontColor();
        g.fillRect(area);
    }
    else
    {
        g.setColour(Colors::getBrowserBGColor().darker());
        g.drawRect(area);
    }
    
    //g.setFont(klaf->getFileBrowserFont());
    g.setFont((float)height * Dimensions::rowTextScalar);
    g.setColour(fontC);
    //g.drawFittedText(getFileName(rowNumber), area.withX(Dimensions::fileIconSize + indent + spacer), juce::Justification::centredLeft, 1);
    g.drawText(getFileName(rowNumber), area.withX(Dimensions::fileIconSize + spacer), juce::Justification::centredLeft, true);

    audioFileIcon->drawWithin(g, area.withWidth(Dimensions::fileIconSize).reduced(3).toFloat(), juce::RectanglePlacement::stretchToFit, Dimensions::fileIconAlpha);

}

bool RecentFilesList::addFile(juce::File fileToAdd, juce::String name)
{

    //checks if file already exists in recent, returns if found
    for (int i = 0; i < recentValueTree.getNumChildren(); ++i)
    {
        auto rec = recentValueTree.getChild(i);
        if (rec.getProperty(TreeIDs::filePath.getParamID()).toString().compare(fileToAdd.getFullPathName()) == 0)
        {
            return false;
        }
    }

    //otherwise, add file to tree and update UI
    juce::ValueTree newFileTree{ TreeIDs::File.getParamID() };
    newFileTree.setProperty(TreeIDs::filePath.getParamID(), fileToAdd.getFullPathName(), nullptr);
    newFileTree.setProperty(TreeIDs::fileName.getParamID(), name, nullptr);

    recentValueTree.addChild(newFileTree, -1, nullptr);

    listBox.updateContent();

    return true;
}

void RecentFilesList::updateFileListFromTree(juce::ValueTree& recentsTree)
{
    recentValueTree = recentsTree;
    listBox.setModel(this);
}

int RecentFilesList::getNumSelectedRows()
{
    return listBox.getNumSelectedRows();
}

juce::Array<juce::ValueTree> RecentFilesList::getSelectedValueTrees()
{
    juce::Array<juce::ValueTree> selectedTrees;

    auto selectedRows = listBox.getSelectedRows();

    for (int i = 0; i < selectedRows.size(); ++i)
    {
        int rowNum = selectedRows[i];
        selectedTrees.add(recentValueTree.getChild(rowNum));
    }

    return selectedTrees;
}

juce::var RecentFilesList::getDragSourceDescription(const juce::SparseSet<int>& rowsToDescribe)
{
    return juce::var(DragStrings::recentsDragString);
}

juce::String RecentFilesList::getFileName(int rowNumber)
{
    for (int i = 0; i < recentValueTree.getNumChildren(); ++i)
    {
        auto recentFileTree = recentValueTree.getChild(rowNumber);
        if (recentFileTree.getType() == juce::Identifier(TreeIDs::File.getParamID()))
        {
            return recentFileTree.getProperty(TreeIDs::fileName.getParamID()).toString();
        }
    }

    return juce::String{};
}

juce::String RecentFilesList::getFilePath(int rowNumber)
{
    for (int i = 0; i < recentValueTree.getNumChildren(); ++i)
    {
        auto recentFileTree = recentValueTree.getChild(rowNumber);
        if (recentFileTree.getType() == juce::Identifier(TreeIDs::File.getParamID()))
        {
            return recentFileTree.getProperty(TreeIDs::filePath.getParamID()).toString();
        }
    }

    return juce::String();
}

void RecentFilesList::clearList()
{
    recentValueTree.removeAllChildren(nullptr);
}

//==============================================================================
KrumTreeItem::KrumTreeItem(juce::ValueTree& fileVt, FavoritesTreeView* parentView, SimpleAudioPreviewer* previewer)
: fileValueTree(fileVt), previewer(previewer), parentTreeView(parentView), InfoPanelComponent("File", "Files can be renamed or removed from this browser. NOTE: these aren't you're actual files, so any changes made aren't making changes to the actual file.")
{

    treeHasChanged();

    setDrawsInLeftMargin(true);
    setInterceptsMouseClicks(false, true);
}

bool KrumTreeItem::mightContainSubItems()
{
    return false;
}

std::unique_ptr<juce::Component> KrumTreeItem::createItemComponent()
{

    if (fileValueTree == juce::ValueTree())
    {
        return nullptr;
    }

    auto newKrumItem = new EditableComp(*this, bgColor);

    auto label = static_cast<juce::Label*>(newKrumItem);
    auto comp = static_cast<juce::Component*>(label);
    return std::move(std::unique_ptr<juce::Component>(comp));
}

int KrumTreeItem::getItemHeight() const
{
    //return 15;
    return Dimensions::rowHeight;
}

void KrumTreeItem::itemClicked(const juce::MouseEvent& e)
{
    
    
    if (previewer)
    {
        juce::File file{ getFilePath() };
        if (previewer->isAutoPlayActive())
        {
            if (file != previewer->getCurrentFile())
            {
                previewer->loadFile(file);
            }

            previewer->setWantsToPlayFile(true);
        }
    }
    else
    {
        DBG("Previewer NULL");
    }

}

void KrumTreeItem::itemDoubleClicked(const juce::MouseEvent& e)
{
    juce::File file{ getFilePath() };

    if (previewer)
    {
        if (!previewer->isAutoPlayActive())
        {
            if (file != previewer->getCurrentFile())
            {
                previewer->loadFile(file);
            }
            previewer->setWantsToPlayFile(true);
        }
    }
    else
    {
        DBG("Previewer NULL");
    }
}

void KrumTreeItem::itemSelectionChanged(bool isNowSelected)
{
    // we handle clicked items in the itemClicked method, so this is just for the selection made using the keyboard arrows
    if(parentTreeView->isMouseButtonDown())
    {
		return;
	}


    if (isNowSelected && previewer)
    {
        juce::File file{ getFilePath() };
        if (!file.isDirectory() && previewer->isAutoPlayActive())
        {
			if (file != previewer->getCurrentFile())
			{
				previewer->loadFile(file);
			}

			previewer->setWantsToPlayFile(true);
        }
    }
}

void KrumTreeItem::closeLabelEditor(juce::Label* label)
{
    if (label->getText().isNotEmpty())
    {
       setItemName(label->getText());
    }

    setItemEditing(false);
    treeHasChanged();
    repaintItem();
}

juce::String KrumTreeItem::getUniqueName() const
{
    return juce::String(juce::String(getIndexInParent()) + "-" + getFilePath());
}

juce::String KrumTreeItem::getFilePath() const
{
    return fileValueTree.getProperty(TreeIDs::filePath.getParamID()).toString();
}

juce::String KrumTreeItem::getItemName() const
{
    return fileValueTree.getProperty(TreeIDs::fileName.getParamID()).toString();
}

void KrumTreeItem::setItemName(juce::String newName)
{
    fileValueTree.setProperty(TreeIDs::fileName.getParamID(), newName, nullptr);
}

juce::File KrumTreeItem::getFile()
{
    return juce::File(getFilePath());
}

bool KrumTreeItem::isItemEditing()
{
    return editing;
}

void KrumTreeItem::setItemEditing(bool isEditing)
{
    editing = isEditing;
}

void KrumTreeItem::removeThisItem()
{
    getParentItem()->removeSubItem(getIndexInParent());
}

void KrumTreeItem::tellParentToRemoveMe()
{

    auto parent = fileValueTree.getParent();
    parent.removeChild(fileValueTree, nullptr);
    
    removeThisItem();
}

void KrumTreeItem::setBGColor(juce::Colour newColor)
{
    bgColor = newColor;
}

void KrumTreeItem::dragInParentTree(const juce::MouseEvent& e)
{
    parentTreeView->mouseDrag(e);
}

juce::ValueTree KrumTreeItem::getValueTree()
{
    return fileValueTree;
}

//-----------------------------------------------------------------------

KrumTreeItem::EditableComp::EditableComp (KrumTreeItem& o, juce::Colour backColor)
    : owner(o), bgColor(backColor)
{
    setText(owner.getItemName(), juce::dontSendNotification);
    setTooltip(owner.getFilePath());
    setInterceptsMouseClicks(true, true);

}

void KrumTreeItem::EditableComp::paint(juce::Graphics& g)
{
    auto area = getLocalBounds();

    juce::Colour fontC = Colors::getBrowserFontColor();

    if (owner.isSelected())
    {
        g.setColour(Colors::getHighlightColor());
        fontC = Colors::getHighlightFontColor();
    }
    else
    {
        g.setColour(Colors::getBrowserBGColor());
    }
    
    g.fillRect(area);

    if (!isBeingEdited())
    {
        g.setFont(area.getHeight() * Dimensions::rowTextScalar);
        g.setColour(fontC);
        g.drawFittedText(getText(), area.withLeft(Dimensions::fileIconSize + 5), juce::Justification::centredLeft, 1);
    }
    
    owner.audioFileIcon->drawWithin(g, area.withWidth(Dimensions::fileIconSize).reduced(3).toFloat(), juce::RectanglePlacement::stretchToFit, Dimensions::fileIconAlpha);
}

void KrumTreeItem::EditableComp::textWasEdited()
{
    //passed the text through to the owner to handle different situations
    owner.setItemName(getText());
    setText(owner.getItemName(), juce::dontSendNotification);
    owner.setItemEditing(false);
}

void KrumTreeItem::EditableComp::editorAboutToBeHidden(juce::TextEditor* editor)
{
    owner.setItemEditing(false);
}

void KrumTreeItem::EditableComp::mouseEnter(const juce::MouseEvent& e)
{
    owner.mouseEnter(e);
}

void KrumTreeItem::EditableComp::mouseExit(const juce::MouseEvent& e)
{
    owner.mouseExit(e);
}

void KrumTreeItem::EditableComp::mouseDown(const juce::MouseEvent& e)
{
    bool shift = e.mods.isShiftDown();
    bool cntrl = e.mods.isCtrlDown();
    bool itemSelected = owner.isSelected();
    int thisIndex = owner.getIndexInParent();
    int numSelectedItems = owner.parentTreeView->getNumSelectedItems();

    //works out if your mouse click is selecting multpile items by holding the shift or cntrl modifiers
    if (!itemSelected && (cntrl || shift))
    {
        if (shift)
        {
            if (numSelectedItems > 0)
            {
                auto parentItem = owner.getParentItem();
                int firstSelected = owner.parentTreeView->getSelectedItem(0)->getIndexInParent();
                int lastSelected = owner.parentTreeView->getSelectedItem(numSelectedItems - 1)->getIndexInParent();

                if (thisIndex > lastSelected)
                {
                    for (; lastSelected <= thisIndex; lastSelected++)
                    {
                        parentItem->getSubItem(lastSelected)->setSelected(true, false);
                    }

                }
                else if(thisIndex < firstSelected)
                {
                    for (; thisIndex <= firstSelected; firstSelected--)
                    {
                        parentItem->getSubItem(firstSelected)->setSelected(true, false);
                    }
                }
            }
        }
        else if(cntrl)
        {
            if (numSelectedItems > 0)
            {
                owner.setSelected(true, false);
            }
        }
    }
    else if(!itemSelected && !e.mods.isPopupMenu()) //with no mods
    {

        owner.setSelected(true, true);
    }

    //right click menu
    if (e.mods.isPopupMenu())
    {
        juce::PopupMenu menu;
        juce::Rectangle<int> showPoint{ e.getMouseDownScreenX(), e.getMouseDownScreenY(), 0, 0 };
        juce::PopupMenu::Options menuOptions;

        juce::String menuName{ "Make Module" };
        if (owner.parentTreeView->getNumSelectedItems(3) > 1)
        {
            menuName += "s";
        }

        menu.addItem(RightClickMenuIds::assignToModule, menuName);
        menu.addSeparator();
        menu.addItem(RightClickMenuIds::rename_Id, "Rename");
        menu.addItem(RightClickMenuIds::reveal_Id, "Reveal");
        menu.addSeparator();
        menu.addItem(RightClickMenuIds::remove_Id, "Remove");


        menu.showMenuAsync(menuOptions.withTargetScreenArea(showPoint), juce::ModalCallbackFunction::create(handleResult, this));
    }

    owner.itemClicked(e);
}

void KrumTreeItem::EditableComp::mouseUp(const juce::MouseEvent& e)
{
    if (!(e.mods.isCtrlDown() || e.mods.isShiftDown()) && !e.mouseWasDraggedSinceMouseDown())
    {
        owner.setSelected(true, true);
    }
    //setSelectedWithMods(e, true);
}

void KrumTreeItem::EditableComp::mouseDoubleClick(const juce::MouseEvent& e)
{
    owner.itemDoubleClicked(e);
}

void KrumTreeItem::EditableComp::mouseDrag(const juce::MouseEvent& e)
{
    owner.dragInParentTree(e);
}

void KrumTreeItem::EditableComp::handleResult(int result, EditableComp* comp)
{
    if (result == RightClickMenuIds::rename_Id)
    {
        comp->showEditor();
        comp->repaint();
        comp->owner.setItemEditing(true);
    }
    else if (result == RightClickMenuIds::remove_Id)
    {
        comp->owner.tellParentToRemoveMe();
    }
    else if (result == RightClickMenuIds::reveal_Id)
    {
        comp->owner.getFile().revealToUser();
    }
    else if (result == RightClickMenuIds::assignToModule)
    {
        comp->owner.parentTreeView->makeModulesFromSelectedFiles();
    }

}

//=================================================================================================================================//


//KrumTreeHeaderItem::KrumTreeHeaderItem(KrumTreeView* pTree, juce::File fullPathName, juce::String name, int numFilesHidden)
KrumTreeHeaderItem::KrumTreeHeaderItem(juce::ValueTree& folderVTree, FavoritesTreeView* pTree)
    : folderValueTree(folderVTree), parentTreeView(pTree), InfoPanelComponent("Folder", "Folders can be renamed and removed, just like Files, these aren't the actual Folders on your system, just a representation")
{
    treeHasChanged();
    setLinesDrawnForSubItems(false);
    setInterceptsMouseClicks(false, true);
}

bool KrumTreeHeaderItem::mightContainSubItems()
{
    //this should contain subItems
    return true;
}

std::unique_ptr<juce::Component> KrumTreeHeaderItem::createItemComponent()
{

    if(folderValueTree == juce::ValueTree())
    {
        return nullptr;
    }

    auto newHeaderComp = new EditableHeaderComp(*this, bgColor);

    int numFilesExcluded = getNumFilesExcluded();

    if (numFilesExcluded > 0)
    {
        newHeaderComp->setTooltip(newHeaderComp->getTooltip() + " (" + juce::String(numFilesExcluded) + " files excluded)");
    }

    auto label = static_cast<juce::Label*>(newHeaderComp);
    auto comp = static_cast<juce::Component*>(label);
    return std::move(std::unique_ptr<juce::Component>(comp));
}

void KrumTreeHeaderItem::paintOpenCloseButton(juce::Graphics& g, const juce::Rectangle< float >& buttonArea, juce::Colour backgroundColour, bool isMouseOver)
{

    g.setColour(Colors::getBrowserBGColor());
    g.fillRect(buttonArea);

    juce::Path path;
    auto area = buttonArea.reduced(7, 5);
    g.setColour(juce::Colours::grey);

    if (!isOpen())
    {
        path.addTriangle(area.getTopLeft(), {area.getRight(), area.getCentreY()}, area.getBottomLeft());
        g.strokePath(path, juce::PathStrokeType(1.0f));
    }
    else
    {
        path.addTriangle(area.getBottomLeft(), { area.getRight(), area.getCentreY() - 2 }, area.getBottomRight());
        g.strokePath(path, juce::PathStrokeType(1.0f));
        g.fillPath(path);
    }

}

void KrumTreeHeaderItem::paintVerticalConnectingLine(juce::Graphics& g, const juce::Line<float>& line)
{
    //juce::Line<float> newLine = line;

    //g.setColour(parentTreeView->getConnectedLineColor());
    //g.drawLine(newLine);

}

void KrumTreeHeaderItem::paintHorizontalConnectingLine(juce::Graphics& g, const juce::Line<float>& line)
{
    //if (isOpen())
    //{
    //    g.setColour(parentTreeView->getConnectedLineColor());
    //    g.drawLine(line);
    //}
}

int KrumTreeHeaderItem::getItemHeight() const
{
    //return 17;
    return Dimensions::rowHeight;
}

void KrumTreeHeaderItem::itemClicked(const juce::MouseEvent& e)
{
    //repaintItem();
    //DBG(headerName + " Clicked");

}

void KrumTreeHeaderItem::itemDoubleClicked(const juce::MouseEvent& e)
{
    setOpen(!isOpen());
    //parentTreeView->updateOpenness();
    //DBG("HeaderItem Double Clicked:" + juce::String(isOpen() ? "is Open" : "is NOT Open"));
}

juce::String KrumTreeHeaderItem::getFolderPath() const
{ 
    return folderValueTree.getProperty(TreeIDs::folderPath.getParamID()).toString();
}

juce::String KrumTreeHeaderItem::getItemHeaderName() const
{ 
    return folderValueTree.getProperty(TreeIDs::folderName.getParamID()).toString();
}

void KrumTreeHeaderItem::setItemHeaderName(juce::String newName)
{
    folderValueTree.setProperty(TreeIDs::folderName.getParamID(), newName, nullptr);
}

void KrumTreeHeaderItem::setNumFilesExcluded(int numFilesHidden)
{
    folderValueTree.setProperty(TreeIDs::hiddenFiles.getParamID(), numFilesHidden, nullptr);
}

int KrumTreeHeaderItem::getNumFilesExcluded()
{
    return (int)folderValueTree.getProperty(TreeIDs::hiddenFiles.getParamID());
}

juce::File KrumTreeHeaderItem::getFile()
{
    return juce::File(getFolderPath());
}

juce::String KrumTreeHeaderItem::getUniqueName() const
{
    return juce::String(getIndexInParent() + "-" + getFolderPath());
}

void KrumTreeHeaderItem::setBGColor(juce::Colour newColor)
{
    bgColor = newColor;
}

bool KrumTreeHeaderItem::isItemEditing(bool checkChildren)
{
    if (editing)
    {
        return true;
    }
    else if (checkChildren && getNumSubItems() > 0)
    {
        for (int i = 0; i < getNumSubItems(); i++)
        {
            auto item = getSubItem(i);
            if (item->mightContainSubItems())
            {
                auto headerItem = parentTreeView->makeHeaderItem(item);
                if (headerItem && headerItem->isItemEditing(true))
                {
                    return true;
                }
            }
            else
            {
                auto treeItem = parentTreeView->makeTreeItem(item);
                if (treeItem && treeItem->isItemEditing())
                {
                    return true;
                }
            }
        }
    }

    return false;
}

void KrumTreeHeaderItem::setItemEditing(bool isEditing)
{
    editing = isEditing;
}

void KrumTreeHeaderItem::setEditable(bool isEditable)
{
    editable = isEditable;
}

bool KrumTreeHeaderItem::isEditable()
{
    return editable;
}

void KrumTreeHeaderItem::removeThisHeaderItem()
{
    getParentItem()->removeSubItem(getIndexInParent());

}

void KrumTreeHeaderItem::tellParentToRemoveMe()
{
    //parentTreeView->removeItem(getItemIdentifierString());

    auto parent = folderValueTree.getParent();
    parent.removeChild(folderValueTree, nullptr);
    removeThisHeaderItem();

}

void KrumTreeHeaderItem::clearAllChildren()
{
    int index = getIndexInParent();
    if (index == FileBrowserSectionIds::recentFolders_Ids)
    {
        //parentTreeView->clearRecent();
    }
    else if (index == FileBrowserSectionIds::favoritesFolders_Ids)
    {
        parentTreeView->clearFavorites();
    }
}


KrumTreeHeaderItem::EditableHeaderComp::EditableHeaderComp(KrumTreeHeaderItem& o, juce::Colour backColor)
    : owner(o), bgColor(backColor)
{
    

    setText(owner.getItemHeaderName(), juce::dontSendNotification);
    setTooltip(owner.getFolderPath());
    setInterceptsMouseClicks(true, true);
}

void KrumTreeHeaderItem::EditableHeaderComp::paint(juce::Graphics& g)
{
    auto area = getLocalBounds();

    juce::Colour fontC = Colors::getBrowserFontColor();

    if (owner.isSelected())
    {
        g.setColour(Colors::getHighlightColor());
        fontC = Colors::getHighlightFontColor();
    }
    else
    {
        g.setColour(Colors::getBrowserBGColor());
    }
    
    g.fillRect(area);
    
    if (!isBeingEdited())
    {
        g.setFont(area.getHeight() * Dimensions::rowTextScalar);
        g.setColour(fontC);
        //g.drawFittedText(getText(), area.withLeft(Dimensions::fileIconSize + 5), juce::Justification::centredLeft, 1);
        g.drawText(getText(), area.withLeft(Dimensions::fileIconSize + 5), juce::Justification::centredLeft, true);

        /*if (owner.isEditable())
        {
            g.setFont(g.getCurrentFont().getHeightInPoints() - 5.0f);
            g.drawFittedText("Folder", area.withTrimmedRight(5) , juce::Justification::centredRight, 1);
        }*/
    }

    if (owner.isOpen())
    {
        owner.folderOpenIcon->drawWithin(g, area.withWidth(Dimensions::fileIconSize).reduced(3).toFloat(), juce::RectanglePlacement::fillDestination, Dimensions::fileIconAlpha);
    }
    else
    {
        owner.folderIcon->drawWithin(g, area.withWidth(Dimensions::fileIconSize).reduced(3).toFloat(), juce::RectanglePlacement::fillDestination, Dimensions::fileIconAlpha);
    }
    
}

void KrumTreeHeaderItem::EditableHeaderComp::textWasEdited()
{
    owner.setItemHeaderName(getText());
    setText(owner.getItemHeaderName(), juce::dontSendNotification);
    owner.setItemEditing(true);
}

void KrumTreeHeaderItem::EditableHeaderComp::editorAboutToBeHidden(juce::TextEditor* editor)
{
    owner.setItemEditing(true);
}

void KrumTreeHeaderItem::EditableHeaderComp::mouseEnter(const juce::MouseEvent& e)
{
    owner.mouseEnter(e);
}

void KrumTreeHeaderItem::EditableHeaderComp::mouseExit(const juce::MouseEvent& e)
{
    owner.mouseExit(e);
}

void KrumTreeHeaderItem::EditableHeaderComp::mouseDown(const juce::MouseEvent& e)
{
    bool deselect = ! e.mods.isShiftDown();

    owner.setSelected(true, deselect);
    owner.itemClicked(e);

    juce::Rectangle<int> showPoint{ e.getMouseDownScreenX(), e.getMouseDownScreenY(), 0, 0 };

    if (e.mods.isPopupMenu() && owner.isEditable())
    {
        juce::PopupMenu menu;
        juce::PopupMenu::Options menuOptions;

        menu.addItem(RightClickMenuIds::rename_Id, "Rename");
        menu.addItem(RightClickMenuIds::reveal_Id, "Reveal");
        menu.addSeparator();
        menu.addItem(RightClickMenuIds::remove_Id, "Remove");

        

        menu.showMenuAsync(menuOptions.withTargetScreenArea(showPoint), juce::ModalCallbackFunction::create(handleResult, this));
    }
    /*else if(e.mods.isPopupMenu() && !owner.isEditable())
    {
        
    }*/
}

void KrumTreeHeaderItem::EditableHeaderComp::mouseDoubleClick(const juce::MouseEvent& e)
{
    owner.itemDoubleClicked(e);
}

void KrumTreeHeaderItem::EditableHeaderComp::handleResult(int result, EditableHeaderComp* comp)
{
    if (result == RightClickMenuIds::rename_Id)
    {
        comp->showEditor();
        comp->repaint();
        comp->owner.setItemEditing(true);
    }
    else if (result == RightClickMenuIds::remove_Id)
    {
        comp->owner.tellParentToRemoveMe();
    }
    else if (result == RightClickMenuIds::reveal_Id)
    {
        comp->owner.getFile().revealToUser();
    }
   /* if (result == RightClickMenuIds::clear_Id)
    {
        comp->owner.clearAllChildren();
    }*/
}

//=================================================================================================================================//
//=================================================================================================================================//

//FavoritesTreeView::FavoritesTreeView(FileChooser* fc, SimpleAudioPreviewer* prev)
FavoritesTreeView::FavoritesTreeView(KrumFileBrowser& fb)
    //: fileChooser(fc), previewer(prev)
    : fileBrowser(fb)
{

    setMultiSelectEnabled(true);
    //addMouseListener(this, true);

    setRepaintsOnMouseActivity(true);

    rootItem.reset(new RootHeaderItem(favoritesValueTree, this));
    rootItem->setLinesDrawnForSubItems(false);
    setRootItem(rootItem.get());
    setRootItemVisible(false);

    setPaintingIsUnclipped(true);

    favoritesValueTree.addListener(this);

    startTimerHz(20);

}

FavoritesTreeView::~FavoritesTreeView()
{
    DBG("FavoritesTreeView Destructor Called");
    setRootItem(nullptr);
}

void FavoritesTreeView::paint(juce::Graphics& g)
{
    auto area = getLocalBounds();

    g.setColour(Colors::getBrowserBGColor());
    g.fillRect(area.toFloat());

    if (highlightTreeView && isMouseOverOrDragging())
    {
        g.setColour(Colors::getCanDropFileColor());
        g.drawRect(area);
    }


    juce::TreeView::paint(g);

}

void FavoritesTreeView::valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& childRemoved, int indexOfRemoval)
{
    rootItem->treeHasChanged();
    fileBrowser.getPanelHeader(KrumFileBrowser::BrowserSections::favorites)->animateRemoveFile();
}

juce::Rectangle<int> FavoritesTreeView::getTreeViewBounds()
{
    return rootItem->getOwnerView()->getBounds();
}

void FavoritesTreeView::refreshChildren()
{
    auto area = getLocalBounds();

    DBG("FileBrowser Area: " + area.toString());
}

void FavoritesTreeView::deselectAllItems()
{
    rootItem->setSelected(false, true);
}

//EXTERNAL files being dropped
bool FavoritesTreeView::isInterestedInFileDrag(const juce::StringArray& files)
{
    fileBrowser.getPanelHeader(KrumFileBrowser::BrowserSections::favorites)->setShowCanDropFile(true);
    setHighlightTreeView(true);
    return true;
}

//EXTERNAL files being dropped
void FavoritesTreeView::filesDropped(const juce::StringArray& files, int x, int y)
{
    for (auto fileName : files)
    {
        juce::File leadFile(fileName);
        if (leadFile.isDirectory())
        {
            createNewFavoriteFolder(leadFile.getFullPathName());
        }
        else if (hasAudioFormat(leadFile.getFileExtension()))
        {
            createNewFavoriteFile(leadFile.getFullPathName());
        }
        else
        {
            DBG("File could not be added, no audio format found");
        }
    }

    auto fh = fileBrowser.getPanelHeader(KrumFileBrowser::BrowserSections::favorites);
    fh->setShowCanDropFile(false);
    fh->animateAddFile();

    setHighlightTreeView(false);

}


bool FavoritesTreeView::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& details)
{
    bool isInterested = details.description.toString().contains(DragStrings::fileChooserDragString);
    if (isInterested)
    {
        fileBrowser.getPanelHeader(KrumFileBrowser::BrowserSections::favorites)->setShowCanDropFile(true);
        setHighlightTreeView(true);
    }
    return isInterested;
}

void FavoritesTreeView::itemDropped(const juce::DragAndDropTarget::SourceDetails& details)
{
    auto fcHeader = fileBrowser.getPanelHeader(KrumFileBrowser::BrowserSections::fileChooser);
    auto fileChooser = static_cast<FileChooser*>(fcHeader->getPanelComponent());

    DBG("Items Dropped");
    DBG(details.description.toString());
    
    auto selectedFiles = fileChooser->getSelectedFiles();

    for (int i = 0; i < selectedFiles.size(); ++i)
    {
        auto file = selectedFiles[i];
        DBG("File " + juce::String(i + 1) + ": " + file.getFullPathName());

        if (file.isDirectory())
        {
            createNewFavoriteFolder(file.getFullPathName());
        }
        else
        {
            createNewFavoriteFile(file.getFullPathName());
        }

    }

    fileBrowser.getPanelHeader(KrumFileBrowser::BrowserSections::favorites)->setShowCanDropFile(false);
    fileBrowser.getPanelHeader(KrumFileBrowser::BrowserSections::favorites)->animateAddFile();
    setHighlightTreeView(false);

    //DBG("File Path Dropped: " + fileChooser->getSelectedFile().getFullPathName());


}

void FavoritesTreeView::createNewFavoriteFile(const juce::String& fullPathName)
{
    auto file = juce::File(fullPathName);

    if (hasAudioFormat(file.getFileExtension()))
    {
        juce::ValueTree newFavValTree{ TreeIDs::File.getParamID(), {{TreeIDs::fileName.getParamID(), file.getFileName()}, {TreeIDs::filePath.getParamID(), file.getFullPathName()}} };
        favoritesValueTree.addChild(newFavValTree, -1, nullptr);
        
        rootItem->addSubItem(new KrumTreeItem(newFavValTree, this, fileBrowser.getAudioPreviewer()));
    }
    else
    {
        DBG("Audio Format Not Supported");
    }
}

void FavoritesTreeView::createNewFavoriteFolder(const juce::String& fullPathName)
{
    juce::File folder(fullPathName);

    if (folder.isDirectory())
    {

        juce::ValueTree newFolderValTree{ TreeIDs::Folder.getParamID() };
        newFolderValTree.setProperty(TreeIDs::folderPath.getParamID(), fullPathName, nullptr);
        newFolderValTree.setProperty(TreeIDs::folderName.getParamID(), folder.getFileName(), nullptr);
        newFolderValTree.setProperty(TreeIDs::hiddenFiles.getParamID(), juce::String(0), nullptr);
        favoritesValueTree.addChild(newFolderValTree, -1, nullptr);

        auto newFavFolderNode = new KrumTreeHeaderItem(newFolderValTree, this);
        rootItem->addSubItem(newFavFolderNode);

        juce::Array<juce::File> childFiles = folder.findChildFiles(juce::File::findFilesAndDirectories + juce::File::ignoreHiddenFiles, false);
        int numHiddenFiles = 0;

        for (int j = 0; j < childFiles.size(); j++)
        {
            auto childFile = childFiles[j];
            auto childExt = childFile.getFileExtension();
            if (childFile.isDirectory())
            {
                addNewFavoriteSubFolder(childFile, numHiddenFiles, newFavFolderNode, newFolderValTree);
            }
            else if (hasAudioFormat(childExt))
            {
                auto childFilePath = childFile.getFullPathName();
                auto childFileName = childFile.getFileName();
                //make value tree
                juce::ValueTree childFileValTree{ TreeIDs::File.getParamID(), {{TreeIDs::fileName.getParamID(), childFileName}, {TreeIDs::filePath.getParamID(), childFilePath}} };
                newFolderValTree.addChild(childFileValTree, j, nullptr);
                //make tree view
                auto childFileNode = new KrumTreeItem(childFileValTree, this, fileBrowser.getAudioPreviewer());
                newFavFolderNode->addSubItem(childFileNode);
            }
            else
            {
                ++numHiddenFiles;
                DBG(childFile.getFullPathName() + " is not a supported Audio Format");
            }

        }

        if (newFolderValTree.getNumChildren() == 0)
        {
            DBG("No Files were added! They were all excluded, maybe..");
        }
        
        if (numHiddenFiles > 0)
        {
            newFavFolderNode->setNumFilesExcluded(numHiddenFiles);
        }

        DBG("New Folder Tree added");
        DBG(newFolderValTree.toXmlString());
        DBG("With Parent Tree: " + newFolderValTree.getParent().toXmlString());
    }

    sortFiles();
}

void FavoritesTreeView::addNewFavoriteSubFolder(juce::File& folder, int& numHiddenFiles, KrumTreeHeaderItem* parentNode, juce::ValueTree& parentFolderValTree)
{
    if (folder.isDirectory())
    {
        juce::ValueTree newSubFolderTree = { TreeIDs::Folder.getParamID(), {{ TreeIDs::folderName.getParamID(), folder.getFileName() }, {TreeIDs::folderPath.getParamID(), folder.getFullPathName()}, {TreeIDs::hiddenFiles.getParamID(), juce::String(0)}} };
        parentFolderValTree.addChild(newSubFolderTree, -1 ,nullptr);

        auto newSubFolderNode = new KrumTreeHeaderItem(newSubFolderTree, this);
        parentNode->addSubItem(newSubFolderNode);

        auto childFiles = folder.findChildFiles(juce::File::findFilesAndDirectories + juce::File::ignoreHiddenFiles, false);

        for (int i = 0; i < childFiles.size(); i++)
        {
            auto childFile = childFiles[i];

            if (childFile.isDirectory())
            {
                addNewFavoriteSubFolder(childFile, numHiddenFiles, newSubFolderNode, newSubFolderTree);
            }
            else if (hasAudioFormat(childFile.getFileExtension()))
            {
                auto childFilePath = childFile.getFullPathName();
                auto childFileName = childFile.getFileName();

                juce::ValueTree newChildFileValTree = { TreeIDs::File.getParamID(), {{TreeIDs::fileName.getParamID(), childFileName}, {TreeIDs::filePath.getParamID(), childFilePath}} };
                newSubFolderTree.addChild(newChildFileValTree, i, nullptr);
                
                auto childFileNode = new KrumTreeItem(newChildFileValTree, this, fileBrowser.getAudioPreviewer());
                newSubFolderNode->addSubItem(childFileNode);
            }
            else
            {
                ++numHiddenFiles;
                DBG(childFile.getFullPathName() + " is not a supported Audio Format");
            }

        }


        if (numHiddenFiles > 0)
        {
            newSubFolderNode->setNumFilesExcluded(numHiddenFiles);
        }
    }
}

void FavoritesTreeView::reCreateFavoritesFromValueTree()
{

    for (int i = 0; i < favoritesValueTree.getNumChildren(); i++)
    {
        auto childTree = favoritesValueTree.getChild(i);

        if (childTree.getType() == juce::Identifier(TreeIDs::Folder.getParamID()))
        {
            reCreateFavoriteFolder(childTree);
        }

        if (childTree.getType() == juce::Identifier(TreeIDs::File.getParamID()))
        {
            reCreateFavoriteFile(childTree);
        }
    }

    //TODO
    //auto opennessXml = fileBrowserValueTree.getChildWithName(TreeIDs::OPENSTATE).createXml();
    //restoreOpennessState(*opennessXml, false);

}

void FavoritesTreeView::reCreateFavoriteFolder(juce::ValueTree& folderValueTree)
{
    auto newFavFolderNode = new KrumTreeHeaderItem(folderValueTree, this);
    newFavFolderNode->setLinesDrawnForSubItems(true);
    
    /*auto favNode = rootItem->getSubItem(favoritesFolders_Ids);
    favNode->addSubItem(newFavFolderNode);*/

    rootItem->addSubItem(newFavFolderNode);

    for (int i = 0; i < folderValueTree.getNumChildren(); i++)
    {
        auto childTree = folderValueTree.getChild(i);

        if (childTree.getType() == juce::Identifier(TreeIDs::File.getParamID()))
        {
            auto childFileNode = new KrumTreeItem(childTree, this, fileBrowser.getAudioPreviewer());
            childFileNode->setLinesDrawnForSubItems(true);
            newFavFolderNode->addSubItem(childFileNode);
        }
        else if (childTree.getType() == juce::Identifier(TreeIDs::Folder.getParamID()))
        {
            reCreateFavoriteSubFolder(newFavFolderNode, childTree);
        }
    }

}

void FavoritesTreeView::reCreateFavoriteSubFolder(KrumTreeHeaderItem* parentNode, juce::ValueTree& folderValueTree)
{
    auto folderNode = new KrumTreeHeaderItem(folderValueTree, this);
    folderNode->setLinesDrawnForSubItems(true);
    parentNode->addSubItem(folderNode);


    for (int i = 0; i < folderValueTree.getNumChildren(); i++)
    {
        auto childTree = folderValueTree.getChild(i);

        if (childTree.getType() == juce::Identifier(TreeIDs::File.getParamID()))
        {
            auto childFile = new KrumTreeItem(childTree, this, fileBrowser.getAudioPreviewer());
            folderNode->addSubItem(childFile);
        }
        else if (childTree.getType() == juce::Identifier(TreeIDs::Folder.getParamID()))
        {
            reCreateFavoriteSubFolder(folderNode, childTree);
        }
    }

}

//creates a direct child file item in the "Favorites" section
void FavoritesTreeView::reCreateFavoriteFile(juce::ValueTree& fileValueTree)
{
    //auto favNode = rootItem->getSubItem(favoritesFolders_Ids);
    rootItem->addSubItem(new KrumTreeItem(fileValueTree, this, fileBrowser.getAudioPreviewer()));
}


void FavoritesTreeView::sortFiles(FileBrowserSortingIds sortingId)
{
    //FileBrowserSorter sorter;
    //rootItem->sortSubItems<FileBrowserSorter>(sorter);
    //rootItem->treeHasChanged();
}


void FavoritesTreeView::addDummyChild(juce::TreeViewItem* nodeToAddTo)
{
    if (nodeToAddTo == nullptr)
    {
        rootItem->addSubItem(new DummyTreeItem());
    }
    else
    {
        nodeToAddTo->addSubItem(new DummyTreeItem());
    }
}

bool FavoritesTreeView::hasAudioFormat(juce::String fileExtension)
{
    if (fileBrowser.getAudioPreviewer())
    {
        auto audioFormat = fileBrowser.getAudioPreviewer()->getFormatManager()->findFormatForFileExtension(fileExtension);
        return audioFormat != nullptr;
    }

    return false;
}

void FavoritesTreeView::removeValueTreeItem(juce::String fullPathName, FileBrowserSectionIds browserSection)
{
    for (int i = 0; i < favoritesValueTree.getNumChildren(); i++)
    {
        juce::ValueTree childTree = favoritesValueTree.getChild(i);
        auto childTreePath = childTree.getProperty(TreeIDs::filePath.getParamID());

        if (childTreePath.toString().compare(fullPathName) == 0)
        {
            DBG("Item to Remove: " + childTreePath.toString());
            favoritesValueTree.removeChild(childTree, nullptr);

        }
        else
        {
            juce::ValueTree nestedChild = findTreeItem(childTree, fullPathName); 
            if (nestedChild.isValid())
            {
                favoritesValueTree.removeChild(nestedChild, nullptr);
            }
            else
            {
                DBG("NestedChild Not Found");
            }
        }
    }
}

juce::ValueTree FavoritesTreeView::findTreeItem(juce::ValueTree parentTree, juce::String fullPathName)
{
    for (int i = 0; i < parentTree.getNumChildren(); i++)
    {
        auto childTree = parentTree.getChild(i);
        auto childTreePath = childTree.getProperty(TreeIDs::filePath.getParamID());

        if (childTreePath.toString().compare(fullPathName) == 0)
        {
            return childTree;
        }
        else if (childTree.getType() == juce::Identifier(TreeIDs::Folder.getParamID()))
        {
            return findTreeItem(childTree, fullPathName);
        }
    }

    return juce::ValueTree();

}

void FavoritesTreeView::updateOpenness()
{
    auto xml = getOpennessState(true);
    //auto openTree = fileBrowserValueTree.getChildWithName(TreeIDs::OPENSTATE);

    //favoritesValueTree.removeAllChildren(nullptr);
    //favoritesValueTree.appendChild(juce::ValueTree::fromXml(xml->toString()), nullptr);
    //auto openTree = fileBrowser.getFileBrowserValueTree().setProperty(TreeIDs::OPENSTATE, juce::var(xml->toString()), nullptr);
}



void FavoritesTreeView::clearFavorites()
{
    /*auto favNode = rootItem->getSubItem(FileBrowserSectionIds::favoritesFolders_Ids);
    favNode->clearSubItems();*/

    rootItem->clearSubItems();
    //auto favValueTree = fileBrowserValueTree.getChildWithName(TreeIDs::FAVORITES);
    favoritesValueTree.removeAllChildren(nullptr);

}

void FavoritesTreeView::removeItem(juce::String idString)
{

    auto item = findItemFromIdentifierString(idString);
    
    if (item->mightContainSubItems())
    {
        auto headerItem = makeHeaderItem(item);
        if (headerItem)
        {
            //removeValueTreeItem(headerItem->getFile().getFullPathName(), FileBrowserSectionIds::favoritesFolders_Ids);
            headerItem->removeThisHeaderItem();
        }
    }
    else
    {
        auto treeItem = makeTreeItem(item);
        if (treeItem)
        {
            juce::String sectionName;
            auto section = findSectionHeaderParent(item, sectionName);
            if (section)
            {
                FileBrowserSectionIds browserSectionId = FileBrowserSectionIds::favoritesFolders_Ids;
                
                if (sectionName.compare(TreeIDs::RECENT.getParamID()) == 0)
                {
                    browserSectionId = FileBrowserSectionIds::recentFolders_Ids;
                }

                //removeValueTreeItem(treeItem->getFile().getFullPathName(), browserSectionId);
                treeItem->removeThisItem();

            }
        }
    }

}

//void FavoritesTreeView::mouseEnter(const juce::MouseEvent& event)
//{
//    if (highlightTreeView)
//    {
//        setHighlightTreeView(true);
//    }
//
//    FavoritesTreeView::mouseEnter(event);
//}



void FavoritesTreeView::mouseDrag(const juce::MouseEvent& event)
{
    if (!areAnyItemsBeingEdited() && event.mouseWasDraggedSinceMouseDown())
    {
        if (!isDragAndDropActive())
        {
            juce::Image itemImage;
            std::unique_ptr<NumberBubble> numBub;

            auto treeItem = static_cast<KrumTreeItem*>(getSelectedItem(0)); 
            if (treeItem)
            {
                juce::var description { DragStrings::favoritesDragString + treeItem->getFile().getFullPathName() };
                auto pos = treeItem->getItemPosition(true);

                if (treeItem)
                {
                    itemImage = juce::Component::createComponentSnapshot(pos).convertedToFormat(juce::Image::ARGB);
                    itemImage.multiplyAllAlphas(0.6f);
                }

                if (getNumSelectedItems() > 1)
                {
                    juce::Graphics g (itemImage);
            
                    numBub.reset(new NumberBubble(getNumSelectedItems(), juce::Colours::red.withAlpha(0.5f), itemImage.getBounds()));

                    g.beginTransparencyLayer(0.6f);
                    numBub->paintEntireComponent(g, false);
                    g.endTransparencyLayer();
                }

                startDragging(description, this, itemImage, true);
//                if(moduleContainer)
//                {
//                    auto me = event.getEventRelativeTo(moduleContainer);
//                    moduleContainer->getEditor()->getModuleViewport()->autoScroll(me.getPosition().getX(), me.getPosition().getY(), 20, 10);
//                }
            }
        }
        else
        {
            //manages the sample swap highlighting on the module, see ModuleContainer::showModuleCanAcceptFile()
            if (moduleContainer)
            {
                if (moduleContainer->contains(event.getEventRelativeTo(moduleContainer).getPosition()))
                {
                    auto& modules = moduleContainer->getActiveModuleEditors();
                    for (int i = 0; i < modules.size(); i++)
                    {
                        auto modEd = modules[i];
                        auto relPoint = event.getEventRelativeTo(modEd);
                        
                        if (modEd->contains(event.getEventRelativeTo(modEd).getPosition()))
                        {
                            if (modEd->thumbnailHitTest(event))
                            {
                                moduleContainer->showModuleCanAcceptFile(modEd);
                            }
                            else
                            {
                                moduleContainer->hideModuleCanAcceptFile(modEd);
                            }
                        }

                        
                    }
                }
            }

            auto dropComp = moduleContainer->getPluginEditor()->getDropSampleArea();

            if (dropComp->contains(event.getEventRelativeTo(dropComp).getPosition()))
            {
                dropComp->setDraggingMouseOver(true);
            }
            else
            {
                dropComp->setDraggingMouseOver(false);
            }
        }
    }
}

//void FavoritesTreeView::mouseExit(const juce::MouseEvent& event)
//{
//    if (highlightTreeView)
//    {
//        setHighlightTreeView(false);
//    }
//
//    FavoritesTreeView::mouseExit(event);
//}

void FavoritesTreeView::dragOperationEnded(const juce::DragAndDropTarget::SourceDetails& details)
{
    if (moduleContainer)
    {
        auto& moduleEditors = moduleContainer->getActiveModuleEditors();
        for (int i = 0; i < moduleEditors.size(); i++)
        {
            auto modEd = moduleEditors[i];
            moduleContainer->hideModuleCanAcceptFile(modEd);
        }
    }

    auto dropComp = moduleContainer->getPluginEditor()->getDropSampleArea();
    dropComp->setDraggingMouseOver(false);

}

void FavoritesTreeView::setItemEditing(juce::String idString, bool isEditing)
{
    KrumTreeHeaderItem* headerItem = nullptr;
    KrumTreeItem* treeItem = nullptr;

    auto item = findItemFromIdentifierString(idString);

    if (item->mightContainSubItems())
    {
        headerItem = makeHeaderItem(item);
        if (headerItem)
        {
            headerItem->setItemEditing(isEditing);
            return;
        }
    }
    else
    {
        treeItem = makeTreeItem(item);
        if (treeItem)
        {
            treeItem->setItemEditing(isEditing);
            return;
        }
    }
}


bool FavoritesTreeView::areAnyItemsBeingEdited()
{
    for (int i = 0; i < rootItem->getNumSubItems(); i++)
    {
        auto faveFileNode = rootItem->getSubItem(i);
        if (!faveFileNode->mightContainSubItems())
        {
            auto treeItem = makeTreeItem(faveFileNode);
            if (treeItem && treeItem->isItemEditing())
            {
                return true;
            }
        }
        else
        {
            auto headerItem = makeHeaderItem(faveFileNode);
            if (headerItem && headerItem->isItemEditing(true))
            {
                return true;
            }
        }
    }

    return false;

}


juce::Array<juce::ValueTree> FavoritesTreeView::getSelectedValueTrees()
{
    juce::Array<juce::ValueTree> selectedTrees;

    for (int i = 0; i < getNumSelectedItems(); ++i)
    {
        auto krumItem = makeTreeItem(getSelectedItem(i));
        if (krumItem != nullptr && !krumItem->mightContainSubItems())
        {
            selectedTrees.add(krumItem->getValueTree());
        }
    }

    return selectedTrees;
}

//juce::ValueTree FavoritesTreeView::getFileBrowserValueTree()
//{
//
//    //auto v = ;
//    return favoritesValueTree.getParent();
//}

RootHeaderItem* FavoritesTreeView::getRootNode()
{
    return rootItem.get();
}

KrumTreeHeaderItem* FavoritesTreeView::makeHeaderItem(juce::TreeViewItem* item)
{
    KrumTreeHeaderItem* retHeader = static_cast<KrumTreeHeaderItem*>(item);
    if (retHeader)
    {
        return retHeader;
    }
    else
    {
        return nullptr;
    }
}


KrumTreeItem* FavoritesTreeView::makeTreeItem(juce::TreeViewItem* item)
{
    KrumTreeItem* retItem = static_cast<KrumTreeItem*>(item);
    if (retItem)
    {
        return retItem;
    }
    else
    {
        return nullptr;
    }
}


KrumTreeItem* FavoritesTreeView::makeTreeItem(juce::Component* item)
{
    KrumTreeItem* retItem = static_cast<KrumTreeItem*>(item);
    if (retItem)
    {
        return retItem;
    }
    else
    {
        return nullptr;
    }
}

bool FavoritesTreeView::doesFolderExistInBrowser(juce::String fullPathName)
{
    for (int i = 0; i < favoritesValueTree.getNumChildren(); i++)
    {
        auto childTree = favoritesValueTree.getChild(i);

        if (childTree.getType() == juce::Identifier(TreeIDs::Folder.getParamID()))
        {
            auto folderPathVar = childTree.getProperty(TreeIDs::folderPath.getParamID());
            if (folderPathVar.toString().compare(fullPathName) == 0)
            {
                return true;
            }
        }
    }

    return false;
}

void FavoritesTreeView::assignModuleContainer(KrumModuleContainer* newContainer)
{
    moduleContainer = newContainer;
}

juce::Colour FavoritesTreeView::getConnectedLineColor()
{
    return conLineColor;
}

juce::Colour FavoritesTreeView::getFontColor()
{
    return Colors::getBrowserFontColor();
}

void FavoritesTreeView::updateFavoritesFromTree(juce::ValueTree& newFavsTree)
{
    favoritesValueTree = newFavsTree;
    reCreateFavoritesFromValueTree();
}

KrumTreeHeaderItem* FavoritesTreeView::findSectionHeaderParent(juce::TreeViewItem* item, juce::String& sectionName)
{
    if (item->mightContainSubItems())
    {
        auto headerItem = static_cast<KrumTreeHeaderItem*>(item);
        if (headerItem)
        {
            auto parentItem = static_cast<KrumTreeHeaderItem*>(headerItem->getParentItem());
            if (parentItem)
            {
                bool fav = parentItem->getItemHeaderName().compare(TreeIDs::FAVORITES.getParamID()) == 0;
                bool recent = parentItem->getItemHeaderName().compare(TreeIDs::RECENT.getParamID()) == 0;
                if (fav || recent)
                {
                    sectionName = parentItem->getItemHeaderName();
                    return parentItem;
                }
                else
                {
                    findSectionHeaderParent(parentItem, sectionName);
                }
            }
        }
    }
    else
    {
        
        auto parentItem = static_cast<KrumTreeHeaderItem*>(item->getParentItem());
        if (parentItem)
        {
            bool fav = parentItem->getItemHeaderName().compare(TreeIDs::FAVORITES.getParamID()) == 0;
            bool recent = parentItem->getItemHeaderName().compare(TreeIDs::RECENT.getParamID()) == 0;
            if (fav || recent)
            {
                sectionName = parentItem->getItemHeaderName();
                return parentItem;
            }
            else
            {
                findSectionHeaderParent(parentItem, sectionName);
            }
        }
    }

    return nullptr;
}


void FavoritesTreeView::makeModulesFromSelectedFiles()
{
    auto selectedFileTrees = getSelectedValueTrees();
    //bool showEmptyModule = false;

    if (selectedFileTrees.size() <= moduleContainer->getNumEmptyModules())
    {
        auto modulesTree = fileBrowser.getStateValueTree().getChildWithName(TreeIDs::KRUMMODULES.getParamID());
        if (modulesTree.hasType(TreeIDs::KRUMMODULES.getParamID()))
        {
            //auto editor = moduleContainer->getPluginEditor();

            for (int fileIndex = 0; fileIndex < selectedFileTrees.size(); ++fileIndex)
            {
                moduleContainer->handleNewFile(selectedFileTrees[fileIndex]);
            }
        }
        else
        {
            DBG("Can't Make Modules From RightClick, Tree type not KRUMMODULES");
        }
        
    }

    //this makes the last empty module to drop new files on
    //if (showEmptyModule)
    //{
    //    moduleContainer->showFirstEmptyModule();
    //}

}

void FavoritesTreeView::setHighlightTreeView(bool shouldHighlight)
{
    if (shouldHighlight != highlightTreeView)
    {
        repaintHighlight = true;
    }

    highlightTreeView = shouldHighlight;
    
}

void FavoritesTreeView::timerCallback()
{
    if (repaintHighlight)
    {
        //repaint();
        fileBrowser.getPanelHeader(KrumFileBrowser::BrowserSections::favorites)->repaint();
        //repaintHighlight = false;
    }
    else if(highlightTreeView && !isMouseOver())
    {
        setHighlightTreeView(false);
    }

}

//==================================================================================================


//=================================================================================================================================//

FileChooser::CurrentPathBox::CurrentPathBox(FileChooser& fc)
    : fileChooser(fc), InfoPanelComboBox("Current Path", "This displays your currently viewed file location. You can access your drives in the drop down menu. You can also save \"Places\" that will save in the menu.")
{
    setRepaintsOnMouseActivity(true);

    setJustificationType(juce::Justification::centredLeft);
    //fileChooser.fileBrowser.getFileBrowserValueTree().addListener(this);
}

FileChooser::CurrentPathBox::~CurrentPathBox() {}

void FileChooser::CurrentPathBox::valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& childRemoved, int indexRemoved)
{
    /*if (parentTree.getType() == TreeIDs::PLACES)
    {
        showPopup();
    }*/
}

void FileChooser::CurrentPathBox::showPopup()
{
    fileChooser.updatePathBoxDropDown();
    juce::ComboBox::showPopup();
}

void FileChooser::CurrentPathBox::paint(juce::Graphics& g)
{
    juce::ComboBox::paint(g);

    auto area = getLocalBounds();

    auto& animator = juce::Desktop::getInstance().getAnimator();
    if (animator.isAnimating(this))
    {
        g.setColour(fileChooser.animationColor);
        g.fillRect(area);
    }

    if (isMouseOver())
    {
        g.setColour(Colors::getHighlightColor());
        g.drawRect(area);
    }
}

void FileChooser::CurrentPathBox::addPathBoxItem(int itemId, std::unique_ptr<PathBoxItem> pathBoxItem, juce::String title)
{
    auto menu = getRootMenu();
    pathBoxItem->setName(title);
    //menu->addCustomItem(itemId, std::move(pathBoxItem), nullptr, title);
    menu->addCustomItem(itemId, std::move(pathBoxItem), nullptr, title);
}

bool FileChooser::CurrentPathBox::isUserPlace(PathBoxItem* itemToTest)
{
    auto placesTree = fileChooser.fileBrowser.getFileBrowserValueTree().getChildWithName(TreeIDs::PLACES.getParamID());
    for (int i = 0; i < placesTree.getNumChildren(); ++i)
    {
        if (placesTree.getChild(i).getProperty(TreeIDs::placePath.getParamID()).toString().compare(itemToTest->path) == 0)
        {
            return true;
        }
    }

    return false;
}

//=================================================================================================================================//


FileChooser::PathBoxItem::PathBoxItem(juce::String p, CurrentPathBox& owner)
    : ownerComboBox(owner), juce::PopupMenu::CustomComponent(false), path(p)
{
    //setRepaintsOnMouseActivity(true);
}

FileChooser::PathBoxItem::~PathBoxItem() {}

void FileChooser::PathBoxItem::paint(juce::Graphics& g)
{
    auto area = getLocalBounds();

    g.setColour(isItemHighlighted() ? Colors::getHighlightColor() : Colors::getSectionBGColor());
    g.fillRect(area);

    g.setColour(Colors::getBrowserFontColor());
    
    g.drawFittedText(getName(), area, juce::Justification::centredLeft, 1);

}

void FileChooser::PathBoxItem::getIdealSize(int& idealWidth, int& idealHeight)
{
    getLookAndFeel().getIdealPopupMenuItemSize(getName(), false, -1, idealWidth, idealHeight);
}

void FileChooser::PathBoxItem::mouseDown(const juce::MouseEvent& e)
{
    if (e.mods.isPopupMenu())
    {
        if (ownerComboBox.isUserPlace(this))
        {
            juce::PopupMenu menu;
            juce::PopupMenu::Options options;
            options.withTargetComponent(this);
            //options.withMousePosition();
            menu.addItem(1, "Remove Place");


            menu.showMenuAsync(options, juce::ModalCallbackFunction::create(handleRightClick, this));
            DBG("Right-Click on " + getName());
        }
    }
    else
    {
        DBG("Left Click on " + getName());
        triggerMenuItem();
    }
}

void FileChooser::PathBoxItem::handleRightClick(int result, PathBoxItem* item)
{
    if (result != 0)
    {
        DBG("Remove " + item->getName());
        item->removeThisPlace();
    }
}

void FileChooser::PathBoxItem::removeThisPlace()
{
    auto placesTree = ownerComboBox.fileChooser.fileBrowser.getFileBrowserValueTree().getChildWithName(TreeIDs::PLACES.getParamID());
    for (int i = 0; i < placesTree.getNumChildren(); ++i)
    {
        if (placesTree.getChild(i).getProperty(TreeIDs::placePath.getParamID()).toString().compare(path) == 0)
        {
            placesTree.removeChild(i, nullptr);
            ownerComboBox.fileChooser.animateRemovePlace();
            ownerComboBox.hidePopup();
        }
    }
}
//=================================================================================================================================//

FileChooser::FileChooser(KrumFileBrowser& fb, SimpleAudioPreviewer& p)
    :  fileBrowser(fb), previewer(p)
{

    setDirectory(defaultLocation);
    
    fileTree.setItemHeight(Dimensions::rowHeight);
    fileTree.setMultiSelectEnabled(true);
    fileTree.setColour(juce::TreeView::ColourIds::selectedItemBackgroundColourId, Colors::getHighlightColor());
    fileTree.setDragAndDropDescription(DragStrings::fileChooserDragString);
    fileTree.refresh();

    fileChooserThread.startThread(juce::Thread::Priority::high);
    
    addAndMakeVisible(fileTree);

    addAndMakeVisible(currentPathBox);
    currentPathBox.setColour(juce::ComboBox::ColourIds::backgroundColourId, Colors::getBrowserBGColor());
    //currentPathBox.setColour(juce::ComboBox::ColourIds::backgroundColourId, juce::Colours::lightgrey);
    //currentPathBox.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::black.withAlpha(0.2f));
    currentPathBox.setColour(juce::ComboBox::ColourIds::outlineColourId, Colors::getBrowserFontColor());
    currentPathBox.setColour(juce::ComboBox::ColourIds::arrowColourId, Colors::getBackOutlineColor());
    currentPathBox.setColour(juce::PopupMenu::textColourId, Colors::getHighlightFontColor());
    currentPathBox.setEditableText(false);
    currentPathBox.addListener(this);

    addAndMakeVisible(goUpButton);
    auto upArrowImage = juce::Drawable::createFromImageData(BinaryData::arrow_upward_grey_24dp_svg, BinaryData::arrow_upward_grey_24dp_svgSize);
    goUpButton.setImages(upArrowImage.get());
    goUpButton.onClick = [this] { goUp(); };
    goUpButton.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::transparentBlack);
    goUpButton.setColour(juce::DrawableButton::ColourIds::backgroundColourId, Colors::getBrowserPathBoxColor());

    fileTree.addListener(this);
    //fileBrowser.addFileBrowserTreeListener(this);
}

FileChooser::~FileChooser()
{
    DBG("FileChooser Destructor");
}


void FileChooser::paint(juce::Graphics& g)
{
    auto area = getLocalBounds();
    g.setColour(Colors::getBrowserBGColor());
    g.fillRect(area.toFloat());
}

void FileChooser::resized()
{
    auto area = getLocalBounds();

    int upButtonSize = Dimensions::currentPathHeight;

    currentPathBox.setBounds(area.withLeft(upButtonSize).withBottom(Dimensions::currentPathHeight));
    goUpButton.setBounds(area.withRight(currentPathBox.getX()).withBottom(upButtonSize));
 
    fileTree.setBounds(area.withTop(currentPathBox.getBottom()));
    //locationTabs.setBounds(area/*.withTop(fileTree.getY())*/.withRight(fileTree.getX()));

}


void FileChooser::setDirectory(juce::File newDirectory)
{
    directoryList.setDirectory(newDirectory, true, true);
    directoryList.refresh();
    currentPathBox.setText(newDirectory.getFullPathName(), juce::dontSendNotification);

    //fileBrowser.getFileBrowserValueTree().setProperty(TreeIDs::LASTOPENPATH, juce::var(newDirectory.getFullPathName()), nullptr);

}

void FileChooser::goUp()
{
    setDirectory(directoryList.getDirectory().getParentDirectory());
}

void FileChooser::selectionChanged()
{
    //DBG("Selection Changed");

    // we want to filter out selection changes that are caused by the user clicking on the treeview
    // this is intended to catch changes made from using the keyboards up and down arrows
    if(isMouseButtonDown())
    {
		return;
	}

    auto file = getSelectedFile();

    if (!file.isDirectory() && previewer.isAutoPlayActive() 
        && fileBrowser.doesPreviewerSupport(file.getFileExtension()))
    {
        if (previewer.getCurrentFile() != file)
        {
            previewer.loadFile(file);
        }

        previewer.setWantsToPlayFile(true);
    }

}

void FileChooser::fileClicked(const juce::File& file, const juce::MouseEvent& e)
{
    if (e.mods.isPopupMenu())
    {
        juce::PopupMenu menu;
        //juce::Rectangle<int> showPoint{ getM, getMouseXYRelative().getY(), 0, 0 };
        juce::PopupMenu::Options menuOptions;

        menu.addItem(RightClickMenuIds::addToFavorite, "Add To Favorites");
        menu.addItem(RightClickMenuIds::addPlace, "Add To Places");
        menu.addItem(RightClickMenuIds::revealInOS, "Reveal");

        //setCurrentTabIndex(tabIndex, false);
        menuOptions.withTargetComponent(this);
        //with mouse Position??
        menu.showMenuAsync(menuOptions, juce::ModalCallbackFunction::create(handleRightClick, this));
    }

    if (!file.isDirectory() && previewer.isAutoPlayActive() && fileBrowser.doesPreviewerSupport(file.getFileExtension()))
    {
        if (previewer.getCurrentFile() != file)
        {
            previewer.loadFile(file);
        }

        previewer.setWantsToPlayFile(true);
    }
}

void FileChooser::fileDoubleClicked(const juce::File& file)
{
    DBG("File Double-Clicked: " + file.getFullPathName());
    if (!file.isDirectory() && !previewer.isAutoPlayActive() && fileBrowser.doesPreviewerSupport(file.getFileExtension()))
    {
        if (previewer.getCurrentFile() != file)
        {
            previewer.loadFile(file);
        }
        
        previewer.setWantsToPlayFile(true);
    }
    else if(file.isDirectory())
    {
        setDirectory(file);
    }
    else
    {
        DBG("File not previewable");
    }
}

void FileChooser::browserRootChanged(const juce::File& newRoot)
{
}

juce::File FileChooser::getSelectedFile()
{
    return fileTree.getSelectedFile(0);
}

juce::Array<juce::File> FileChooser::getSelectedFiles()
{
    juce::Array<juce::File> selectedFiles{};

    for (int i = 0; i < fileTree.getNumSelectedFiles(); ++i)
    {
        selectedFiles.add(fileTree.getSelectedFile(i));
    }

    return selectedFiles;

}

//
//void FileChooser::valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& addedChild)
//{
//    DBG("Parent: " + parentTree.getType().toString() + " added Child: " + addedChild.getType().toString());
//}

void FileChooser::handleChosenPath()
{
    juce::File file = getFileFromChosenPath();

    if (file.exists())
    {
        setDirectory(file);
    }
}

void FileChooser::updatePathBoxDropDown()
{
    currentPathBox.clear();
    pathBoxPaths.clear();
    pathBoxNames.clear();
   
    currentPathBox.setText(directoryList.getDirectory().getFullPathName());

    getPaths();
    addPaths();
    
}

juce::String FileChooser::findPathFromName(juce::String itemName)
{
    int index = -1;
    for (int i = 0; i < pathBoxNames.size(); ++i)
    {
        if (pathBoxNames[i].compare(itemName) == 0)
        {
            index = i;
        }
    }

    if (index >= 0)
    {
        return pathBoxPaths[index];
    }

    return juce::String{};

}

juce::File FileChooser::getFileFromChosenPath()
{

    juce::String currentName = currentPathBox.getText();
    int index = -1;
    for (int i = 0; i < pathBoxNames.size(); ++i)
    {
        DBG("Path Box Name: " + pathBoxNames[i]);
        if (pathBoxNames[i].compare(currentName) == 0)
        {
            index = i;
            DBG("Index = " + juce::String(index));
        }
    }

    if (index >= 0)
    {
        DBG("PathBoxSelectedPath: " + pathBoxPaths[index]);
        return juce::File { pathBoxPaths[index] };
    }

    return juce::File();
}

void FileChooser::addPaths()
{
    int drives = 0;
    int places = 1;
    int userPlaces = 2;

    int currentHeader = -1;
    userPlacesIndex = -1;
    for (int i = 0; i < pathBoxNames.size(); ++i)
    {
        if (pathBoxNames[i].isEmpty())
        {
            currentHeader++;

            //keeping like this so we can easily change layout
            if (currentHeader == drives)
            {
                currentPathBox.addSeparator();
            }
            else if (currentHeader == places)
            {
                currentPathBox.addSeparator();
            }
            else if (currentHeader == userPlaces)
            {
                currentPathBox.addSeparator();
            }

            
        }
        else if (currentHeader == drives)
        {
            currentPathBox.addPathBoxItem(currentPathBox.getNumItems() + i + 1, std::make_unique<PathBoxItem>(pathBoxPaths[i], currentPathBox), pathBoxNames[i]);
        }
        else if (currentHeader == places)
        {
            //currentPathBox.addItem(pathBoxNames[i], currentPathBox.getNumItems() + i + 1);
            currentPathBox.addPathBoxItem(currentPathBox.getNumItems() + i + 1, std::make_unique<PathBoxItem>(pathBoxPaths[i], currentPathBox), pathBoxNames[i]);
        }
        else if (currentHeader == userPlaces)
        {
            int index = currentPathBox.getNumItems() + i + 1;
            currentPathBox.addPathBoxItem(index, std::make_unique<PathBoxItem>(pathBoxPaths[i], currentPathBox), pathBoxNames[i]);
            
            if (userPlacesIndex < 0)
            {
                userPlacesIndex = index;
            }
        }

    }
}

void FileChooser::getPaths()
{

    pathBoxNames.add({});
    pathBoxPaths.add({});

#if JUCE_WINDOWS
    juce::Array<juce::File> roots;
    juce::File::findFileSystemRoots(roots);


    for (int i = 0; i < roots.size(); ++i)
    {
        const juce::File& drive = roots.getReference(i);

        juce::String name(drive.getFullPathName());
        pathBoxPaths.add(name);

        if (drive.isOnHardDisk())
        {
            juce::String volume(drive.getVolumeLabel());

            if (volume.isEmpty())
                volume = TRANS("Hard Drive");

            name << " [" << volume << ']';
        }
        else if (drive.isOnCDRomDrive())
        {
            name << " [" << TRANS("CD/DVD drive") << ']';
        }

        pathBoxNames.add(name);
    }

    pathBoxPaths.add({});
    pathBoxNames.add({});

    pathBoxPaths.add(juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getFullPathName());
    pathBoxNames.add(TRANS("Documents"));
    pathBoxPaths.add(juce::File::getSpecialLocation(juce::File::userMusicDirectory).getFullPathName());
    pathBoxNames.add(TRANS("Music"));
    pathBoxPaths.add(juce::File::getSpecialLocation(juce::File::userPicturesDirectory).getFullPathName());
    pathBoxNames.add(TRANS("Pictures"));
    pathBoxPaths.add(juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getFullPathName());
    pathBoxNames.add(TRANS("Desktop"));

#elif JUCE_MAC
    
    for (auto& volume : juce::File("/Volumes").findChildFiles(juce::File::findDirectories, false))
    {
        if (volume.isDirectory() && !volume.getFileName().startsWithChar('.'))
        {
            pathBoxPaths.add(volume.getFullPathName());
            pathBoxNames.add(volume.getFileName());
        }
    }
    
    pathBoxPaths.add({});
    pathBoxNames.add({});

    pathBoxPaths.add(juce::File::getSpecialLocation(juce::File::userHomeDirectory).getFullPathName());
    pathBoxNames.add(TRANS("Home folder"));
    pathBoxPaths.add(juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getFullPathName());
    pathBoxNames.add(TRANS("Documents"));
    pathBoxPaths.add(juce::File::getSpecialLocation(juce::File::userMusicDirectory).getFullPathName());
    pathBoxNames.add(TRANS("Music"));
    pathBoxPaths.add(juce::File::getSpecialLocation(juce::File::userPicturesDirectory).getFullPathName());
    pathBoxNames.add(TRANS("Pictures"));
    pathBoxPaths.add(juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getFullPathName());
    pathBoxNames.add(TRANS("Desktop"));
#endif

    pathBoxPaths.add({});
    pathBoxNames.add({});

    auto locationsTree = fileBrowser.getFileBrowserValueTree().getChildWithName(TreeIDs::PLACES.getParamID());
    for (int i = 0; i < locationsTree.getNumChildren(); ++i)
    {
        auto locationTree = locationsTree.getChild(i);
        pathBoxPaths.add(locationTree.getProperty(TreeIDs::placePath.getParamID()).toString());
        pathBoxNames.add(locationTree.getProperty(TreeIDs::placeName.getParamID()).toString());
    }

}


void FileChooser::comboBoxChanged(juce::ComboBox* comboBox)
{

    if (comboBox->getSelectedId() != lastSelectedId && !comboBox->isPopupActive())
    {
        handleChosenPath();
        lastSelectedId = comboBox->getSelectedId();

    }
}

void FileChooser::handleRightClick(int result, FileChooser* fileChooser)
{
    if (result == RightClickMenuIds::addToFavorite)
    {
        DBG("add to favorites");

        //auto& fbTree = fileChooser->fileBrowser.getFileBrowserValueTree();
        //auto favTree = fbTree.getChildWithName(TreeIDs::FAVORITES);

        auto numFiles = fileChooser->fileTree.getNumSelectedFiles();
        auto& fileTree = fileChooser->fileTree;
        auto selectedTrees = fileChooser->fileBrowser.getSelectedFileTrees(KrumFileBrowser::BrowserSections::fileChooser);

        for (int i = 0; i < numFiles; ++i)
        {
            auto file = fileTree.getSelectedFile(i);
            fileChooser->fileBrowser.addFileToFavorites(file);
        }
    }
    else if (result == RightClickMenuIds::addPlace)
    {
        DBG("add to location");
        auto selectedFile = fileChooser->fileTree.getSelectedFile();

        //we want to save a location, not a file
        if (!selectedFile.isDirectory() && !selectedFile.isRoot())
        {
            selectedFile = selectedFile.getParentDirectory();
        }

        const auto fbTree = fileChooser->fileBrowser.getFileBrowserValueTree();
        auto placesValueTree = fbTree.getChildWithName(TreeIDs::PLACES.getParamID());

        //we make sure a Place doesn't already exist
        for (int i = 0; i < placesValueTree.getNumChildren(); ++i)
        {
            auto placeTree = placesValueTree.getChild(i);
            auto path = placeTree.getProperty(TreeIDs::placePath.getParamID()).toString();

            if (path.compare(selectedFile.getFullPathName()) == 0)
            {
                DBG("Place already exists!");
                return;
            }
        }

        juce::ValueTree newPlace{ TreeIDs::Place.getParamID() };
        newPlace.setProperty(TreeIDs::placeName.getParamID(), selectedFile.getFileName(), nullptr);
        newPlace.setProperty(TreeIDs::placePath.getParamID(), selectedFile.getFullPathName(), nullptr);
        placesValueTree.addChild(newPlace, -1, nullptr);

        fileChooser->getPaths();
        fileChooser->animateAddPlace();
    }
    else if (result == RightClickMenuIds::revealInOS)
    {
        DBG("show in explorer");
        auto selectedFile = fileChooser->fileTree.getSelectedFile();
        selectedFile.revealToUser();
    }
}

void FileChooser::animateAddPlace()
{
    animationColor = Colors::getAddAnimationColor();
    auto& animator = juce::Desktop::getInstance().getAnimator();
    animator.fadeOut(&currentPathBox, 500);
    animator.fadeIn(&currentPathBox, 500);
}

void FileChooser::animateRemovePlace()
{
    animationColor = Colors::getRemoveAnimationColor();
    auto& animator = juce::Desktop::getInstance().getAnimator();
    animator.fadeOut(&currentPathBox, 500);
    animator.fadeIn(&currentPathBox, 500);
}

//=================================================================================================================================//

KrumFileBrowser::KrumFileBrowser(juce::ValueTree& fbValueTree, juce::AudioFormatManager& fm, 
                                juce::ValueTree& stateTree, juce::AudioProcessorValueTreeState& apvts, KrumSampler& s)
    : audioPreviewer(&fm, stateTree, apvts), fileBrowserValueTree(fbValueTree), stateValueTree(stateTree), formatManager(fm), InfoPanelComponent(FileBrowserInfoStrings::compTitle, FileBrowserInfoStrings::message),
      favoritesTreeView(*this)
{

    // Add concertina panel to viewport
    addAndMakeVisible(concertinaPanel);
    // Add panels to concertina panel
    
    // Each panel is a different section of the browser. Each has their own component for handling interaction. Components defined in KrumFileBrowser.h
    // Add custom header to each panel
    
    bool takeOwnership = false;

    // Recent Panel
    concertinaPanel.addPanel((int)BrowserSections::recent, &recentFilesList, takeOwnership);
    concertinaPanel.setCustomPanelHeader(&recentFilesList, &recentHeader, takeOwnership);

    // Favorites panel
    concertinaPanel.addPanel((int)BrowserSections::favorites, &favoritesTreeView, takeOwnership);
    concertinaPanel.setCustomPanelHeader(&favoritesTreeView, &favoritesHeader, takeOwnership);
    // File Chooser panel
    concertinaPanel.addPanel((int)BrowserSections::fileChooser, &fileChooser, takeOwnership);
    concertinaPanel.setCustomPanelHeader(&fileChooser, &filechooserHeader, takeOwnership);
    
    // Update the sections with the value tree
    auto recTree = fbValueTree.getChildWithName(TreeIDs::RECENT.getParamID());
    recentFilesList.updateFileListFromTree(recTree);
    auto favTree =fbValueTree.getChildWithName(TreeIDs::FAVORITES.getParamID());
    favoritesTreeView.updateFavoritesFromTree(favTree);

    addAndMakeVisible(audioPreviewer);
    audioPreviewer.assignSampler(&s);
    audioPreviewer.refreshSettings();
}


KrumFileBrowser::~KrumFileBrowser()
{
    //concertinaPanel.~ConcertinaPanel();
    concertinaPanel.removeAllChildren();
}

void KrumFileBrowser::paint(juce::Graphics& g)
{
    auto area = getLocalBounds();
    float cornerSize = 5.0f;
    //float outline = 1.0f;

    g.setColour(Colors::getSectionBGColor());
    //g.setColour(juce::Colours::black);
    g.fillRoundedRectangle(area.reduced(5).toFloat(), cornerSize);
}

void KrumFileBrowser::resized()
{
    auto area = getLocalBounds().reduced(10);
//    int spacer = 5;
//    int favButtonH = 35;
//    int favButtonW = 50;
//    int recentTreeViewH = 90;
//    int favTreeViewH = 120;
//    int locationsW = 40;

    concertinaPanel.setBounds(area.withTrimmedBottom(Dimensions::previewerH).reduced(EditorDimensions::shrinkage, 0));
    audioPreviewer.setBounds(area.withTop(concertinaPanel.getBottom()).withRight(area.getRight()).withHeight(Dimensions::previewerH));
    
    //TODO
    //make this reload previous concertina panel sizes
    if (init)
    {
        concertinaPanel.expandPanelFully(&favoritesTreeView, false);
        //concertinaPanel.setPanelSize(&favoritesTreeView, area.getHeight() * 0.2f, false);
        //concertinaPanel.setPanelSize(&recentFilesList, area.getHeight() * 0.2f, false);
        concertinaPanel.setPanelSize(&fileChooser, area.getHeight() * 0.3f, false);
        init = false;
    }

}


int KrumFileBrowser::getNumSelectedItems(BrowserSections section)
{
    if (section == BrowserSections::recent)
    {
        return recentFilesList.getNumSelectedRows();
    }
    else if (section == BrowserSections::favorites)
    {
        return favoritesTreeView.getNumSelectedItems();
    }

    return 0;
}

juce::Array<juce::ValueTree> KrumFileBrowser::getSelectedFileTrees(BrowserSections section)
{
    if (section == BrowserSections::recent)
    {
        return recentFilesList.getSelectedValueTrees();
    }
    else if (section == BrowserSections::favorites)
    {
        return favoritesTreeView.getSelectedValueTrees();
    }
    
    return juce::Array<juce::ValueTree>();
}

juce::Array<juce::File> KrumFileBrowser::getFileChooserSelectedFiles()
{
    return fileChooser.getSelectedFiles();
}   

void KrumFileBrowser::addFileToRecent(const juce::File file, juce::String name)
{
    if (recentFilesList.addFile(file, name))
    {
        recentHeader.animateAddFile();
    }
}

void KrumFileBrowser::addFileToFavorites(juce::File file)
{
    if (file.isDirectory())
    {
        favoritesTreeView.createNewFavoriteFolder(file.getFullPathName());
    }
    else
    {
        favoritesTreeView.createNewFavoriteFile(file.getFullPathName());
    }

    favoritesHeader.animateAddFile();

}

bool KrumFileBrowser::doesPreviewerSupport(juce::String fileExtension)
{
    auto format = audioPreviewer.getFormatManager()->findFormatForFileExtension(fileExtension);
    return format != nullptr;
}

void KrumFileBrowser::rebuildBrowser(juce::ValueTree& newTree)
{
    //auto oldTree = treeView.getFileBrowserValueTree();
    //oldTree = newTree;
    //treeView.reCreateFileBrowserFromValueTree();
    repaint();
}

PanelHeader* KrumFileBrowser::getPanelHeader(BrowserSections section)
{
    if (section == BrowserSections::recent)
    {
        return &recentHeader;
    }
    else if (section == BrowserSections::favorites)
    {
        return &favoritesHeader;
    }
    else if (section == BrowserSections::fileChooser)
    {
        return &filechooserHeader;
    }
}


SimpleAudioPreviewer* KrumFileBrowser::getAudioPreviewer()
{
    return &audioPreviewer;
}


void KrumFileBrowser::assignModuleContainer(KrumModuleContainer* container)
{
    favoritesTreeView.assignModuleContainer(container);
    moduleContainer = container;
}

void KrumFileBrowser::buildDemoKit()
{
    // check to see if the DemoKit folder already exists in the favorites. If so we return
    auto favVt = fileBrowserValueTree.getChildWithName(TreeIDs::FAVORITES.getParamID());
    for (int i = 0; i < favVt.getNumChildren(); i++)
    {
        auto childTree = favVt.getChild(i);

        if (childTree.getType() == juce::Identifier(TreeIDs::Folder.getParamID()))
        {
            auto name = childTree.getProperty(TreeIDs::folderName.getParamID());
            if (name.toString().compare("DemoKit") == 0)
            {
                DBG("Demokit found in favorites tree. Returning from buildDemoKit()");
                return;
            }
        }
        
    }
//
    
    juce::File specialLocation = juce::File::getSpecialLocation(juce::File::SpecialLocationType::userDocumentsDirectory);
    DBG("Location: " + specialLocation.getParentDirectory().getFullPathName());
    juce::String separator = juce::File::getSeparatorString();
    
    juce::File krumsamplerFolder {specialLocation.getFullPathName() + separator + "KrumSampler"};
    juce::File demoKitFolder{ krumsamplerFolder.getFullPathName() + separator + "DemoKit" };

    
    //if this folder doesn't exist OR does exist but has no files in it, we add the binary audio files
    //if (!demoKitFolder.isDirectory() || demoKitFolder.getNumberOfChildFiles(juce::File::TypesOfFileToFind::findFiles) == 0)
    if (!krumsamplerFolder.isDirectory() || !demoKitFolder.isDirectory() || demoKitFolder.getNumberOfChildFiles(juce::File::TypesOfFileToFind::findFiles) == 0)
    {
        auto result = demoKitFolder.createDirectory();
        if (result.wasOk())
        {
            juce::String demoKitPath = demoKitFolder.getFullPathName();
            DBG("DemoKit Made");
            DBG("DemoKit Location: " + demoKitPath);

            juce::File wannaKik{ demoKitPath + separator + "WannaKik.wav" };
            wannaKik.create();
            wannaKik.replaceWithData(BinaryData::WANNA_KIK____48K_wav, BinaryData::WANNA_KIK____48K_wavSize);

            juce::File twentyOneKick{ demoKitPath + separator + "TwentyOneKick.wav" };
            twentyOneKick.create();
            twentyOneKick.replaceWithData(BinaryData::_21_Pilots_Kick_Sample_wav, BinaryData::_21_Pilots_Kick_Sample_wavSize);

            juce::File eightOhEight{ demoKitPath + separator + "EightOhEight.wav" };
            eightOhEight.create();
            eightOhEight.replaceWithData(BinaryData::_808_and_House_Kick_blend_wav, BinaryData::_808_and_House_Kick_blend_wavSize);

            juce::File monsterClap{ demoKitPath + separator + "MonsterClap.wav" };
            monsterClap.create();
            monsterClap.replaceWithData(BinaryData::GW_Monster_clap_snare_wav, BinaryData::GW_Monster_clap_snare_wavSize);

            juce::File hiHatsV4{ demoKitPath + separator + "HiHatsV4.wav" };
            hiHatsV4.create();
            hiHatsV4.replaceWithData(BinaryData::HI_HATS_V4__A_wav, BinaryData::HI_HATS_V4__A_wavSize);

            juce::File hiHatsV10{ demoKitPath + separator + "HiHatsV10.wav" };
            hiHatsV10.create();
            hiHatsV10.replaceWithData(BinaryData::HI_HATS_V10__A_wav, BinaryData::HI_HATS_V10__A_wavSize);

            juce::File marvinSnap{ demoKitPath + separator + "MarvinSnap.wav" };
            marvinSnap.create();
            marvinSnap.replaceWithData(BinaryData::Marvin_Snap_wav, BinaryData::Marvin_Snap_wavSize);
        
            demoKit = demoKitFolder;

            DBG("DemoKit Child Files: " + juce::String(demoKit.getNumberOfChildFiles(juce::File::findFiles)));
            favoritesTreeView.createNewFavoriteFolder(demoKit.getFullPathName());
        }
    }
    else if (demoKitFolder.isDirectory()) // if directory exists and has files in it
    {
        demoKit = demoKitFolder;
        DBG("Demokit folders already exists");
        DBG("DemoKit Child Files: " + juce::String(demoKit.getNumberOfChildFiles(juce::File::findFiles)));
        favoritesTreeView.createNewFavoriteFolder(demoKit.getFullPathName());
    }
    
    
}

juce::ValueTree KrumFileBrowser::getFileBrowserValueTree()
{
    return fileBrowserValueTree;
}

juce::ValueTree& KrumFileBrowser::getStateValueTree()
{
    return stateValueTree;
}

juce::AudioFormatManager& KrumFileBrowser::getFormatManager()
{
    return formatManager;
}

void KrumFileBrowser::addFileBrowserTreeListener(juce::ValueTree::Listener* listener)
{
    fileBrowserValueTree.addListener(listener);
}

//================================================================================================

NumberBubble::NumberBubble(int numberToDisplay, juce::Colour backgroundColor, juce::Rectangle<int> parentBounds)
    : number(numberToDisplay), bgColor(backgroundColor)
{
    setBounds(parentBounds.getRight() - 20, 3, 15, 15);
}

NumberBubble::~NumberBubble()
{
}

void NumberBubble::paint(juce::Graphics& g)
{
    g.setColour(bgColor);
    g.fillEllipse(getBounds().toFloat());

    g.setColour(juce::Colours::white);
    g.drawFittedText(juce::String(number), getBounds(), juce::Justification::centred, 1);
}

//================================================================================================

RootHeaderItem::RootHeaderItem(juce::ValueTree& sectionVTree, FavoritesTreeView* rootItem)
    : sectionValueTree(sectionVTree), parentTreeView(rootItem), InfoPanelComponent("","")
{}

RootHeaderItem::~RootHeaderItem()
{}

void RootHeaderItem::paintOpenCloseButton(juce::Graphics& g, const juce::Rectangle<float>& area, juce::Colour bgColor, bool isMouseOver)
{
    return;
}

void RootHeaderItem::paintItem(juce::Graphics& g, int width, int height)
{
    auto title = sectionValueTree.getType().toString();
    g.setColour(parentTreeView->getFontColor());
    g.drawFittedText(title, { 0, 0, width, height }, juce::Justification::centredLeft, 1);
}

int RootHeaderItem::getItemHeight() const
{
    //return 30;
    return Dimensions::rowHeaderHeight;
}

bool RootHeaderItem::mightContainSubItems()
{
    return true;
}

void RootHeaderItem::itemClicked(const juce::MouseEvent& e)
{
    juce::Rectangle<int> showPoint{ e.getMouseDownScreenX(), e.getMouseDownScreenY(), 0, 0 };

    if (e.mods.isPopupMenu())
    {
        juce::PopupMenu menu;
        juce::PopupMenu::Options menuOptions;

        menu.addItem(RightClickMenuIds::clear_Id, "Clear (like, forever)");

        menu.showMenuAsync(menuOptions.withTargetScreenArea(showPoint), juce::ModalCallbackFunction::create(handleResult, this));
    }
}

void RootHeaderItem::handleResult(int result, RootHeaderItem* comp)
{
    if (result == RightClickMenuIds::clear_Id)
    {
        //comp->owner.clearAllChildren();
        DBG("Clear Clicked");
    }
}

//=================================================================================================================


PanelHeader::PanelHeader(juce::String headerTitle, juce::ConcertinaPanel& concertinaPanel, PanelCompId panelId)
    : InfoPanelComponent(headerTitle, ""), title(headerTitle), panel(concertinaPanel), panelCompId(panelId)
{
    setRepaintsOnMouseActivity(true);
    setNewPanelMessage(title, getInfoPanelMessage());
    startTimerHz(10);
}

PanelHeader::~PanelHeader()
{
    DBG("PanelHeader: " + juce::String(panelCompId) + " Destructor");
}

void PanelHeader::resized()
{
}

void PanelHeader::paint(juce::Graphics& g)
{
    auto area = getLocalBounds();
    auto klaf = static_cast<KrumLookAndFeel*>(&getLookAndFeel());
    bool mouseOver = isMouseOver();
    
    auto& animator = juce::Desktop::getInstance().getAnimator();


    //background
    if (animator.isAnimating(this))
    {
        g.setColour(animationColor);
    }
    else
    {
        if (mouseOver)
        {
            g.setColour(Colors::getPanelHeaderMouseOverBGColor());
        }
        else
        {
            juce::ColourGradient grade{ Colors::getPanelHeaderMouseOverBGColor() , area.getTopLeft().toFloat(), Colors::getPanelHeaderBGColor() , area.getBottomRight().toFloat(), false};
            g.setGradientFill(grade);
        }

        //g.setColour(mouseOver ? Colors::panelHeaderBGColor.brighter(0.14f) : Colors::panelHeaderBGColor);
        //g.setColour(mouseOver ? ColorPaletteColors::orangeRed.withAlpha(0.2f) : ColorPaletteColors::orangeRed.withAlpha(0.4f));
    }
    
    //g.fillRect(area);
    g.fillRoundedRectangle(area.reduced(1).toFloat(), Dimensions::panelHeaderCornerSize);

    //Topline
    juce::Line<float> topLine{area.getBottomLeft().toFloat(), area.getBottomRight().toFloat() };
    //g.setColour(Colors::panelHeaderLineColor);
    //g.drawLine(topLine, 0.5f);

    //Font
    g.setColour(Colors::getPanelHeaderFontColor());

    if (klaf)
    {
        //g.setFont(klaf->getMontExtraBoldTypeFace());
        g.setFont(area.getHeight() * 0.9f);
    }

    g.drawText(title.toUpperCase(), area.withLeft(5), juce::Justification::centredLeft, true);


    //outline if needed
    if (showCanDropFile)
    {
        g.setColour(Colors::getCanDropFileColor());
        g.drawRoundedRectangle(area.toFloat(), EditorDimensions::cornerSize, EditorDimensions::xlOutline * 1.5f);
        //g.drawRect(area);
        
    }
}

bool PanelHeader::showingCanDropFile()
{
    return showCanDropFile;
}

void PanelHeader::setShowCanDropFile(bool shouldShow)
{
    showCanDropFile = shouldShow;
}

juce::Component* PanelHeader::getPanelComponent()
{
    return panel.getPanel((int)panelCompId);
}

void PanelHeader::mouseDown(const juce::MouseEvent& e)
{
    InfoPanelComponent::mouseDown(e);

    if(panelCompId != PanelCompId::fileChooser && e.mods.isPopupMenu())
    {
        juce::PopupMenu menu;
        juce::PopupMenu::Options options;
        //options.withMousePosition();
        options.withTargetComponent(this);
        menu.addItem(1, "Clear");

        DBG("Clear: " + juce::String(panelCompId));
        
        menu.showMenuAsync(options, juce::ModalCallbackFunction::create(handleRightClick, this));
    }
}

void PanelHeader::handleRightClick(int result, PanelHeader* header)
{
    if (result != 0)
    {
        auto panelId = header->panelCompId;
        auto comp = header->getPanelComponent();
        if (panelId == PanelCompId::recent)
        {
            auto recs = static_cast<RecentFilesList*>(comp);
            if (recs)
            {
                recs->clearList();
            }
            else
            {
                DBG("RecentFiles cast null");
            }
        }
        else if (panelId == PanelCompId::favorites)
        {
            auto favs = static_cast<FavoritesTreeView*>(comp);
            if (favs)
            {
                favs->clearFavorites();
            }
            else
            {
                DBG("FavoritesTreeView cast null");
            }
        }
    }
}

juce::String PanelHeader::getInfoPanelMessage()
{
    if (panelCompId == PanelCompId::recent)
    {
        return "stores the samples you recently used";
    }
    else if(panelCompId == PanelCompId::favorites)
    {
        return "add file or folders by dropping them onto here.";
    }
    else if (panelCompId == PanelCompId::fileChooser)
    {
        return "browse your machine and drag items into the favorites to save them. You can also create saved locations for easy access";
    }
}

bool PanelHeader::isInterestedInFileDrag(const juce::StringArray& files)
{
    if (panelCompId == PanelCompId::favorites)
    {
        auto favs = static_cast<FavoritesTreeView*> (getPanelComponent());
        if (favs)
        {
            favs->isInterestedInFileDrag(files);
            return true;
        }

    }
}

void PanelHeader::filesDropped(const juce::StringArray& files, int x, int y)
{
    if (panelCompId == PanelCompId::favorites)
    {
        auto favs = static_cast<FavoritesTreeView*> (getPanelComponent());
        if (favs)
        {
            favs->filesDropped(files, x, y);
        }
    }
}


bool PanelHeader::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& details)
{
    if (details.description.toString().contains(DragStrings::fileChooserDragString))
    {
        if (panelCompId == PanelCompId::favorites)
        {
            auto favs = static_cast<FavoritesTreeView*> (getPanelComponent());
            if (favs)
            {
                showCanDropFile = true;
                
                favs->isInterestedInDragSource(details);
                return true;
            }
        }
    }

    return false;

}

void PanelHeader::itemDropped(const juce::DragAndDropTarget::SourceDetails& details)
{
    if (details.description.toString().contains(DragStrings::fileChooserDragString))
    {
        if (panelCompId == PanelCompId::favorites)
        {
            auto favs = static_cast<FavoritesTreeView*> (getPanelComponent());
            if (favs)
            {
                showCanDropFile = false;
                favs->itemDropped(details);
                animateAddFile();
            }
        }
    }
}

void PanelHeader::animateAddFile()
{
    animationColor = Colors::getAddAnimationColor();

    auto& animator = juce::Desktop::getInstance().getAnimator();
    animator.fadeOut(this, 500);
    animator.fadeIn(this, 500);
}

void PanelHeader::animateRemoveFile()
{
    animationColor = Colors::getRemoveAnimationColor();

    auto& animator = juce::Desktop::getInstance().getAnimator();
    animator.fadeOut(this, 500);
    animator.fadeIn(this, 500);
}

void PanelHeader::timerCallback()
{
    if (!isMouseButtonDownAnywhere() && showCanDropFile)
    {
        showCanDropFile = false;
    }
    repaint();
}

//===================================================================================================
