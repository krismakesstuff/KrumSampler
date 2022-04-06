/*
  ==============================================================================

    KrumFileBrowser.cpp
    Created: 26 Mar 2021 12:46:23pm
    Author:  krisc

  ==============================================================================
*/

#include "SimpleAudioPreviewer.h"
#include "KrumModuleContainer.h"
#include "KrumFileBrowser.h"
#include "KrumModuleEditor.h"
#include "PluginEditor.h"


class KrumTreeHeaderItem;

//==============================================================================
RecentFilesList::RecentFilesList(SimpleAudioPreviewer* p)
    : previewer(p)
{
    listBox.setColour(juce::ListBox::ColourIds::backgroundColourId, juce::Colours::transparentBlack);
    listBox.setRowHeight(Dimensions::rowHeight);
    listBox.setMultipleSelectionEnabled(true);
    listBox.updateContent();
    addAndMakeVisible(listBox);
}

RecentFilesList::~RecentFilesList() 
{}

//void RecentFilesList::paint(juce::Graphics& g)
//{
//    auto area = getLocalBounds();
//
//
//    //g.setColour(juce::Colours::black.withAlpha(0.1f));
//    //g.fillRect(area);
//
//    //g.setColour(juce::Colours::lightgrey);
//    //g.drawFittedText("RECENT", area.withBottom(Dimensions::titleH), juce::Justification::centredLeft, 1);
//}

void RecentFilesList::resized()
{
    auto area = getLocalBounds();

    //if(expanded)
    listBox.setBounds(area);
}

int RecentFilesList::getNumRows()
{
    return recentValueTree.getNumChildren();
}

void RecentFilesList::listBoxItemClicked(int row, const juce::MouseEvent& e)
{
    juce::File file{ getFilePath(row) };
    if (previewer)
    {
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
    juce::File file{ getFilePath(row) };

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


void RecentFilesList::paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) 
{
    juce::Rectangle<int> area = { 0, 0, width, height };
    int spacer = 5;
    
    fileIcon->drawWithin(g, area.withWidth(Dimensions::fileIconSize).reduced(3).toFloat(), juce::RectanglePlacement::stretchToFit, Dimensions::fileIconAlpha);
    
    if (rowIsSelected)
    {
        g.setColour(juce::Colours::black.withAlpha(0.5f));
        g.fillRect(area);
    }
    /* else
     {
         g.setColour(juce::Colours::black.withAlpha(0.1f));
     }*/

    g.setColour(juce::Colours::lightgrey);
    g.drawFittedText(getFileName(rowNumber), area.withX(Dimensions::fileIconSize + spacer), juce::Justification::centredLeft, 1);


}

void RecentFilesList::addFile(juce::File fileToAdd, juce::String name)
{

    //check if file already exists in recent before adding!!

    juce::ValueTree newFileTree{ TreeIDs::File };

    newFileTree.setProperty(TreeIDs::filePath, fileToAdd.getFullPathName(), nullptr);
    newFileTree.setProperty(TreeIDs::fileName, name, nullptr);

    recentValueTree.addChild(newFileTree, -1, nullptr);

    listBox.updateContent();
}

void RecentFilesList::updateFileListFromTree(juce::ValueTree& recentsTree)
{
    recentValueTree = recentsTree;
    listBox.updateContent();
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

    /*for (int i = 0; i < rowsToDescribe.size(); ++i)
    {
        rowsToDescribe[i]
    }*/

    return juce::var(DragStrings::recentsDragString);
}

juce::String RecentFilesList::getFileName(int rowNumber)
{
    for (int i = 0; i < recentValueTree.getNumChildren(); ++i)
    {
        auto recentFileTree = recentValueTree.getChild(rowNumber);
        if (recentFileTree.getType() == TreeIDs::File)
        {
            return recentFileTree.getProperty(TreeIDs::fileName).toString();
        }
    }

    return juce::String{};
}

juce::String RecentFilesList::getFilePath(int rowNumber)
{
    for (int i = 0; i < recentValueTree.getNumChildren(); ++i)
    {
        auto recentFileTree = recentValueTree.getChild(rowNumber);
        if (recentFileTree.getType() == TreeIDs::File)
        {
            return recentFileTree.getProperty(TreeIDs::filePath).toString();
        }
    }

    return juce::String();
}

//==============================================================================
KrumTreeItem::KrumTreeItem(juce::ValueTree& fileVt, FavoritesTreeView* parentView, SimpleAudioPreviewer* previewer)
: fileValueTree(fileVt), previewer(previewer), parentTreeView(parentView), InfoPanelComponent("File", "Files can be renamed or removed from this browser. NOTE: these aren't you're actual files, so any changes made aren't making changes to the actual file.")
{
    /*file = fullPathName;
    file = fullPathName;
    itemName = name;
    itemName = name;
    */

    treeHasChanged();

    //setLinesDrawnForSubItems(true);
    setDrawsInLeftMargin(true);
    setInterceptsMouseClicks(false, true);
}

bool KrumTreeItem::mightContainSubItems()
{
    return false;
}

std::unique_ptr<juce::Component> KrumTreeItem::createItemComponent()
{
    //if (!fileValueTree.getParent().isValid())
    //return nullptr;
    
    if (fileValueTree == juce::ValueTree())
    {
        return nullptr;
    }

    auto newKrumItem = new EditableComp(*this, bgColor);

    auto label = static_cast<juce::Label*>(newKrumItem);
    auto comp = static_cast<juce::Component*>(label);
    return std::move(std::unique_ptr<juce::Component>(comp));
}

//void KrumTreeItem::paintHorizontalConnectingLine(juce::Graphics& g, const juce::Line<float>& line)
//{
//    juce::Line<float> newLine = line;
//    newLine.setStart(line.getStartX() - 20, line.getStartY());
//    newLine.setEnd(line.getEndX() - 10, line.getEndY());
//
//    g.setColour(parentTreeView->getConnectedLineColor());
//    g.drawLine(newLine);
//}

int KrumTreeItem::getItemHeight() const
{
    return 15;
}

void KrumTreeItem::itemClicked(const juce::MouseEvent& e)
{
    juce::File file{ getFilePath() };
    if (previewer)
    {
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

/*void KrumTreeItem::itemSelectionChanged(bool isNowSelected)
{
}*/

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
    return juce::String(getIndexInParent() + "-" + getFilePath());
}


juce::String KrumTreeItem::getFilePath() const
{
    return fileValueTree.getProperty(TreeIDs::filePath).toString();
}

juce::String KrumTreeItem::getItemName() const
{
    return fileValueTree.getProperty(TreeIDs::fileName).toString();
}

void KrumTreeItem::setItemName(juce::String newName)
{
    fileValueTree.setProperty(TreeIDs::fileName, newName, nullptr);
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
    parentTreeView->removeItem(getItemIdentifierString());
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

    if (owner.isSelected())
    {
        g.setColour(bgColor);
        g.fillRect(area);
    }

    if (!isBeingEdited())
    {
        g.setColour(juce::Colours::lightgrey);
        g.drawFittedText(getText(), area.withLeft(25), juce::Justification::centredLeft, 1);
    }

    owner.fileIcon->drawWithin(g, area.withWidth(Dimensions::fileIconSize).reduced(3).toFloat(), juce::RectanglePlacement::fillDestination, Dimensions::fileIconAlpha);
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
    else if(!itemSelected) //with no mods
    {

        owner.setSelected(true, true);
    }

    //right click menu
    if (e.mods.isPopupMenu())
    {
        juce::PopupMenu menu;
        juce::Rectangle<int> showPoint{ e.getMouseDownScreenX(), e.getMouseDownScreenY(), 0, 0 };
        juce::PopupMenu::Options menuOptions;

        menu.addItem(RightClickMenuIds::rename_Id, "Rename");
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
}

void KrumTreeItem::EditableComp::mouseDoubleClick(const juce::MouseEvent& e)
{

    //double clicks are being called twice on Mac.. this is a work around until I find the actual issue.
#if JUCE_WINDOWS
    owner.itemDoubleClicked(e);
#endif
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
}

//=================================================================================================================================//


//KrumTreeHeaderItem::KrumTreeHeaderItem(KrumTreeView* pTree, juce::File fullPathName, juce::String name, int numFilesHidden)
KrumTreeHeaderItem::KrumTreeHeaderItem(juce::ValueTree& folderVTree, FavoritesTreeView* pTree)
    : folderValueTree(folderVTree), parentTreeView(pTree), InfoPanelComponent("Folder", "Folders can be renamed and removed, just like Files, these aren't the actual Folders on your system, just a representation")
{
    //file = fullPathName;
    //headerName = name;
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
    //if (!folderValueTree.getParent().isValid())
    
   // return nullptr;
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
        path.addTriangle(area.getBottomLeft(), area.getTopRight(), area.getBottomRight());
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
    return 17;
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
    return folderValueTree.getProperty(TreeIDs::folderPath).toString(); 
}

juce::String KrumTreeHeaderItem::getItemHeaderName() const
{ 
    return folderValueTree.getProperty(TreeIDs::folderName).toString(); 
}

void KrumTreeHeaderItem::setItemHeaderName(juce::String newName)
{
    folderValueTree.setProperty(TreeIDs::folderName, newName, nullptr);
}

void KrumTreeHeaderItem::setNumFilesExcluded(int numFilesHidden)
{
    folderValueTree.setProperty(TreeIDs::hiddenFiles, numFilesHidden, nullptr);
}

int KrumTreeHeaderItem::getNumFilesExcluded()
{
    return (int)folderValueTree.getProperty(TreeIDs::hiddenFiles);
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
    parentTreeView->removeItem(getItemIdentifierString());
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

    if (owner.isSelected())
    {
        g.setColour(bgColor.withAlpha(0.3f));
        g.fillRect(area);
    }

    if (!isBeingEdited())
    {
        g.setColour(juce::Colours::lightgrey);
        g.drawFittedText(getText(), area.withLeft(20), juce::Justification::centredLeft, 1);

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
        menu.addItem(RightClickMenuIds::remove_Id, "Remove");

        menu.showMenuAsync(menuOptions.withTargetScreenArea(showPoint), juce::ModalCallbackFunction::create(handleResult, this));
    }
    /*else if(e.mods.isPopupMenu() && !owner.isEditable())
    {
        
    }*/
}

void KrumTreeHeaderItem::EditableHeaderComp::mouseDoubleClick(const juce::MouseEvent& e)
{
    //double clicks are being called twice on Mac.. this is a work around until I find the actual issue.
#if JUCE_WINDOWS
    owner.itemDoubleClicked(e);
#endif
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
   /* if (result == RightClickMenuIds::clear_Id)
    {
        comp->owner.clearAllChildren();
    }*/
}

//=================================================================================================================================//
//=================================================================================================================================//

FavoritesTreeView::FavoritesTreeView(SimpleAudioPreviewer* prev)
    : previewer(prev)
{

    setMultiSelectEnabled(true);
    //addMouseListener(this, true);

    rootItem.reset(new RootHeaderItem(favoritesValueTree, this));
    //rootItem.reset(new KrumTreeHeaderItem(favoritesValueTree, this));
    rootItem->setLinesDrawnForSubItems(false);
    setRootItem(rootItem.get());
    setRootItemVisible(false);

    setPaintingIsUnclipped(true);
}

FavoritesTreeView::~FavoritesTreeView()
{
    setRootItem(nullptr);
}

void FavoritesTreeView::paint(juce::Graphics& g)
{
    auto area = getLocalBounds();
    

   // g.setColour(juce::Colours::white);
    //g.drawFittedText("Favorites", area.withBottom(titleH), juce::Justification::centredLeft, 1);

    //g.setColour(juce::Colours::darkgrey.darker(0.7f));
    //auto grade = juce::ColourGradient::vertical(juce::Colours::darkgrey.darker(0.7f), juce::Colours::black, area);
    //g.setGradientFill(grade);
    //g.setColour(juce::Colours::black.withAlpha(0.01f));
    //g.fillRoundedRectangle(area.expanded(5).toFloat(), 5.0f);

    juce::TreeView::paint(g);
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
    //auto wildcard = previewer->getFormatManager()->getWildcardForAllFormats();
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
}


bool FavoritesTreeView::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& details)
{
    return details.description.toString().contains(DragStrings::fileChooserDragString);
    //return false;
}

void FavoritesTreeView::itemDropped(const juce::DragAndDropTarget::SourceDetails& details)
{
    DBG("Items Dropped");
    DBG(details.description.toString());
}

void FavoritesTreeView::pickNewFavorite()
{
    const juce::String title{ "Choose A Folder(s) or File(s) to Add to Favorites, you can also drag and drop from external apps!" };
    currentFileChooser.reset(new CustomFileChooser(title, juce::File::getSpecialLocation(juce::File::userDesktopDirectory), previewer->getFormatManager()->getWildcardForAllFormats(), this));

    //Must set the callback lambda before showing
    currentFileChooser->fileChooserCallback = handleChosenFiles;

    int fileBrowserFlag = juce::FileBrowserComponent::FileChooserFlags::canSelectMultipleItems | juce::FileBrowserComponent::FileChooserFlags::openMode
                        | juce::FileBrowserComponent::FileChooserFlags::canSelectDirectories | juce::FileBrowserComponent::FileChooserFlags::canSelectFiles;
    currentFileChooser->showFileChooser(fileBrowserFlag);
}

//This does NOT check file type, so make sure to check for formatting before adding it here.
//void FavoritesTreeView::addFileToRecent(juce::File file, juce::String name)
//{
//    auto recentNode = rootItem->getSubItem(recentFolders_Ids);
//
//    //checking to make sure you don't already have this file. 
//    for (int i = 0; i < recentNode->getNumSubItems(); i++)
//    {
//        auto itItem = recentNode->getSubItem(i);
//        auto krumItem = makeTreeItem(itItem);
//        if (krumItem != nullptr && krumItem->getFile().getFullPathName().compare(file.getFullPathName()) == 0)
//        {
//            //if the recent file is already in the recent folder, we don't want to add it again
//            return;
//        }
//    }
//
//    //add this file's info to the value tree
//    auto recentTree = fileBrowserValueTree.getChildWithName(TreeIDs::RECENT);
//    juce::ValueTree newFileValTree { TreeIDs::File, {{TreeIDs::fileName, name}, {TreeIDs::filePath, file.getFullPathName()}} };
//    recentTree.addChild(newFileValTree, -1, nullptr);
//
//    //create a new TreeViewItem with the new value tree
//    recentNode->addSubItem(new KrumTreeItem(newFileValTree, this, previewer));
//}

void FavoritesTreeView::createNewFavoriteFile(const juce::String& fullPathName)
{
    auto file = juce::File(fullPathName);

    if (hasAudioFormat(file.getFileExtension()))
    {
        //creates a valueTree from the give file
        //auto favTree = fileBrowserValueTree.getChildWithName(TreeIDs::FAVORITES);
        
        juce::ValueTree newFavValTree{ TreeIDs::File, {{TreeIDs::fileName, file.getFileName()}, {TreeIDs::filePath, file.getFullPathName()}} };
        favoritesValueTree.addChild(newFavValTree, -1, nullptr);
        
        //creats a TreeViewItem from the valueTree 
        //auto favNode = rootItem->getSubItem(favoritesFolders_Ids);
        rootItem->addSubItem(new KrumTreeItem(newFavValTree, this, previewer));
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

        juce::ValueTree newFolderValTree{ TreeIDs::Folder };
        newFolderValTree.setProperty(TreeIDs::folderPath, fullPathName, nullptr);
        newFolderValTree.setProperty(TreeIDs::folderName, folder.getFileName(), nullptr);
        newFolderValTree.setProperty(TreeIDs::hiddenFiles, juce::String(0), nullptr);
        favoritesValueTree.addChild(newFolderValTree, -1, nullptr);


        //auto favNode = rootItem->getSubItem(favoritesFolders_Ids);
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
                juce::ValueTree childFileValTree{ TreeIDs::File, {{TreeIDs::fileName, childFileName}, {TreeIDs::filePath, childFilePath}} };
                newFolderValTree.addChild(childFileValTree, j, nullptr);
                //make tree view
                auto childFileNode = new KrumTreeItem(childFileValTree, this, previewer);
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
        juce::ValueTree newSubFolderTree = { TreeIDs::Folder, {{ TreeIDs::folderName, folder.getFileName() }, {TreeIDs::folderPath, folder.getFullPathName()}, {TreeIDs::hiddenFiles, juce::String(0)}} };
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

                juce::ValueTree newChildFileValTree = { TreeIDs::File, {{TreeIDs::fileName, childFileName}, {TreeIDs::filePath, childFilePath}} };
                newSubFolderTree.addChild(newChildFileValTree, i, nullptr);
                
                auto childFileNode = new KrumTreeItem(newChildFileValTree, this, previewer);
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
    /*auto recTreeNode = fileBrowserValueTree.getChildWithName(TreeIDs::RECENT);
    for (int i = 0; i < recTreeNode.getNumChildren(); i++)
    {
        auto childTree = recTreeNode.getChild(i);
        if (childTree.getType() == TreeIDs::File)
        {
            reCreateRecentFile(childTree);
        }
    }*/

   // auto favTreeNode = fileBrowserValueTree.getChildWithName(TreeIDs::FAVORITES);
    for (int i = 0; i < favoritesValueTree.getNumChildren(); i++)
    {
        auto childTree = favoritesValueTree.getChild(i);

        if (childTree.getType() == TreeIDs::Folder)
        {
            reCreateFavoriteFolder(childTree);
        }

        if (childTree.getType() == TreeIDs::File)
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

        if (childTree.getType() == TreeIDs::File)
        {
            auto childFileNode = new KrumTreeItem(childTree, this, previewer);
            childFileNode->setLinesDrawnForSubItems(true);
            newFavFolderNode->addSubItem(childFileNode);
        }
        else if (childTree.getType() == TreeIDs::Folder)
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

        if (childTree.getType() == TreeIDs::File)
        {
            auto childFile = new KrumTreeItem(childTree, this, previewer);
            folderNode->addSubItem(childFile);
        }
        else if (childTree.getType() == TreeIDs::Folder)
        {
            reCreateFavoriteSubFolder(folderNode, childTree);
        }
    }

}

//creates a direct child file item in the "Favorites" section
void FavoritesTreeView::reCreateFavoriteFile(juce::ValueTree& fileValueTree)
{
    //auto favNode = rootItem->getSubItem(favoritesFolders_Ids);
    rootItem->addSubItem(new KrumTreeItem(fileValueTree, this, previewer));
}

//Creates a direct child file item in the "Recent" section
//void FavoritesTreeView::reCreateRecentFile(juce::ValueTree& fileValueTree)
//{
//    //auto recNode = rootItem->getSubItem(FileBrowserSectionIds::recentFolders_Ids);
//    //recNode->addSubItem(new KrumTreeItem(fileValueTree, this, previewer));
//
//
//
//}

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
    if (previewer)
    {
        auto audioFormat = previewer->getFormatManager()->findFormatForFileExtension(fileExtension);
        return audioFormat != nullptr;
    }

    return false;
}

//Updates an item Name, and Number of Hidden Files if applicable
//void KrumTreeView::updateValueTree(juce::String idString)
//{
//    //updateOpenness();
//
//    DBG("ValueTree ID: " + idString);
//
//    KrumTreeHeaderItem* headerItem = nullptr;
//    KrumTreeItem* treeItem = nullptr;
//
//    auto item = findItemFromIdentifierString(idString);
//    
//    auto favTree = fileBrowserValueTree.getChildWithName(TreeIDs::FAVORITES);
//    int numChildren = favTree.getNumChildren();
//
//    if (item->mightContainSubItems())
//    {
//        headerItem = makeHeaderItem(item);
//        if (headerItem != nullptr)
//        {
//            //Finds folders only
//            for (int i = 0; i < numChildren; i++)
//            {
//                auto childTree = favTree.getChild(i);
//                if (childTree.getType() == TreeIDs::Folder)
//                {
//                    auto childPathVar = childTree.getProperty(FileBrowserValueTreeIds::pathId);
//                    if (headerItem->getFile().getFullPathName().compare(childPathVar.toString()) == 0)
//                    {
//                        childTree.setProperty(FileBrowserValueTreeIds::itemNameId, juce::var(headerItem->getItemHeaderName()), nullptr);
//                        childTree.setProperty(FileBrowserValueTreeIds::hiddenFilesId, juce::var(headerItem->getNumFilesExcluded()), nullptr);
//                        return;
//                    }
//                }
//            }
//        }
//    }
//    else
//    {
//        treeItem = makeTreeItem(item);
//
//        if (treeItem != nullptr)
//        {
//            //Finds files saved directly in favorites
//            for (int i = 0; i < numChildren; i++)
//            {
//                auto childTree = favTree.getChild(i);
//                if (childTree.getType() == TreeIDs::File)
//                {
//                    auto childPathVar = childTree.getProperty(FileBrowserValueTreeIds::pathId);
//                    if (treeItem->getFile().getFullPathName().compare(childPathVar.toString()) == 0)
//                    {
//                        childTree.setProperty(FileBrowserValueTreeIds::itemNameId, juce::var(treeItem->getItemName()), nullptr);
//                        return;
//                    }
//                }
//            }
//            
//            //Finds files in a folder in favorites
//            for (int i = 0; i < numChildren; i++)
//            {
//                auto childTree = favTree.getChild(i);
//                if (childTree.getType() == TreeIDs::Folder)
//                {
//                    for (int j = 0; j < childTree.getNumChildren(); j++)
//                    {
//                        auto childFileTree = childTree.getChild(j);
//                        if (childFileTree.getType() == TreeIDs::File)
//                        {
//                            auto childPathVar = childFileTree.getProperty(FileBrowserValueTreeIds::pathId);
//                            if (treeItem->getFile().getFullPathName().compare(childPathVar.toString()) == 0)
//                            {
//                                childFileTree.setProperty(FileBrowserValueTreeIds::itemNameId, juce::var(treeItem->getItemName()), nullptr);
//                                return;
//                            }
//                        }
//                    }
//                }
//            }
//        }
//    }
//    
//    DBG("Didn't save change");
//  
//}

void FavoritesTreeView::removeValueTreeItem(juce::String fullPathName, FileBrowserSectionIds browserSection)
{

    //juce::ValueTree sectionTree = fileBrowserValueTree.getChild(browserSection);

    //auto favTree = fileBrowserValueTree.getChildWithName(TreeIDs::FAVORITES);


    for (int i = 0; i < favoritesValueTree.getNumChildren(); i++)
    {
        juce::ValueTree childTree = favoritesValueTree.getChild(i);
        auto childTreePath = childTree.getProperty(TreeIDs::filePath);

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
        auto childTreePath = childTree.getProperty(TreeIDs::filePath);

        if (childTreePath.toString().compare(fullPathName) == 0)
        {
            return childTree;
        }
        else if (childTree.getType() == TreeIDs::Folder)
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

    favoritesValueTree.removeAllChildren(nullptr);
    favoritesValueTree.appendChild(juce::ValueTree::fromXml(xml->toString()), nullptr);
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
            removeValueTreeItem(headerItem->getFile().getFullPathName(), FileBrowserSectionIds::favoritesFolders_Ids);
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
                
                if (sectionName.compare(TreeIDs::RECENT.toString()) == 0)
                {
                    browserSectionId = FileBrowserSectionIds::recentFolders_Ids;
                }

                removeValueTreeItem(treeItem->getFile().getFullPathName(), browserSectionId);
                treeItem->removeThisItem();

            }
        }
    }

}

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
            if (moduleContainer)
            {
                //auto containerPoint = event.getEventRelativeTo(moduleContainer);
                if (moduleContainer->contains(event.getEventRelativeTo(moduleContainer).getPosition()))
                {
                    auto& modules = moduleContainer->getModuleDisplayOrder();
                    for (int i = 0; i < modules.size(); i++)
                    {
                        auto modEd = modules[i];
                        auto relPoint = event.getEventRelativeTo(modEd);
                        if (modEd->contains(event.getEventRelativeTo(modEd).getPosition()))
                        {
                            if (modEd->thumbnailHitTest(event) /*&& modEd->canThumbnailAcceptFile()*/)
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
        }
    }
}

void FavoritesTreeView::dragOperationEnded(const juce::DragAndDropTarget::SourceDetails& details)
{
    if (moduleContainer)
    {
        auto& moduleEditors = moduleContainer->getModuleDisplayOrder();
        for (int i = 0; i < moduleEditors.size(); i++)
        {
            auto modEd = moduleEditors[i];
            moduleContainer->hideModuleCanAcceptFile(modEd);
        }
    }
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
    //auto faveNode = rootItem->getSubItem(FileBrowserSectionIds::favoritesFolders_Ids);

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

juce::ValueTree& FavoritesTreeView::getFileBrowserValueTree()
{
    return favoritesValueTree.getParent();
}

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

    //auto favTree = fileBrowserValueTree.getChildWithName(TreeIDs::FAVORITES);

    for (int i = 0; i < favoritesValueTree.getNumChildren(); i++)
    {
        auto childTree = favoritesValueTree.getChild(i);

        if (childTree.getType() == TreeIDs::Folder)
        {
            auto folderPathVar = childTree.getProperty(TreeIDs::folderPath);
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
    return fontColor;
}

void FavoritesTreeView::updateFavoritesFromTree(juce::ValueTree& newFavsTree)
{
    favoritesValueTree = newFavsTree;
    reCreateFavoritesFromValueTree();

}

void FavoritesTreeView::handleChosenFiles(const juce::FileChooser& fileChooser)
{
    auto cFileChooser = static_cast<const CustomFileChooser*>(&fileChooser);
    juce::Array<juce::File> resultFiles = fileChooser.getResults();

    for (int i = 0; i < resultFiles.size(); i++)
    {
        auto itFile = resultFiles[i];
        if (itFile.isDirectory())
        {
            cFileChooser->owner->createNewFavoriteFolder(itFile.getFullPathName());
        }
        else
        {
            cFileChooser->owner->createNewFavoriteFile(itFile.getFullPathName());
        }
    }

    /*auto favNode = cFileChooser->owner->rootItem;
    FileBrowserSorter sorter;
    favNode->sortSubItems(sorter);
    favNode->treeHasChanged();*/

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
                bool fav = parentItem->getItemHeaderName().compare(TreeIDs::FAVORITES.toString()) == 0;
                bool recent = parentItem->getItemHeaderName().compare(TreeIDs::RECENT.toString()) == 0;
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
            bool fav = parentItem->getItemHeaderName().compare(TreeIDs::FAVORITES.toString()) == 0;
            bool recent = parentItem->getItemHeaderName().compare(TreeIDs::RECENT.toString()) == 0;
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

LocationTabBar::LocationTabBar(FileChooser& fc)
    : fileChooser(fc), juce::TabbedComponent(juce::TabbedButtonBar::Orientation::TabsAtLeft)
{
    //find roots, add tabs
    setTabBarDepth(Dimensions::locationTabDepth);
    //findAndAddRoots();
    //setCurrentTabIndex(0);
    //setOutline(0);
    //setIndent(0);
    //setColour(juce::TabbedComponent::ColourIds::outlineColourId, juce::Colours::transparentBlack);
    //need to save locations to ValueTree

    //addTab("Desktop", bgColor, createTabButton("Desktop", 0), true);
    //setCurrentTabIndex(0);

}

LocationTabBar::~LocationTabBar() 
{}

void LocationTabBar::addLocation(juce::File location, juce::String name)
{
    //we make sure a tab doesn't already exist
    for (int i = 0; i < locationsValueTree.getNumChildren(); ++i)
    {
        auto locationTree = locationsValueTree.getChild(i);
        auto path = locationTree.getProperty(TreeIDs::locationPath).toString();
        
        if (path.compare(location.getFullPathName()) == 0)
        {
            DBG("Location already exists!");
            return;
        }
    }

    juce::ValueTree newLocation{ TreeIDs::Location };
    newLocation.setProperty(TreeIDs::locationName, name, nullptr);
    newLocation.setProperty(TreeIDs::locationPath, location.getFullPathName(), nullptr);
    locationsValueTree.addChild(newLocation, -1, nullptr);

    addTab(name, bgColor, createTabButton(name, -1), true);
    repaint();
}

void LocationTabBar::addTabsFromLocationTree(juce::ValueTree& locationsTree)
{
    locationsValueTree = locationsTree;

    for (int i = 0; i < locationsValueTree.getNumChildren(); ++i)
    {
        auto locationTree = locationsValueTree.getChild(i);
        auto tabName = locationTree.getProperty(TreeIDs::locationName).toString();
        auto tabPath = locationTree.getProperty(TreeIDs::locationPath).toString();

        DBG("Tab Name: " + tabName + ", Tab Path: " + tabPath);
        addTab(tabName, bgColor, createTabButton(tabName, -1), true);
    }
    

    setCurrentTabIndex(0);

    repaint();

}


void LocationTabBar::currentTabChanged(int newCurrentTab, const juce::String & newCurrentTabName)
{
    lastSelectedIndex = newCurrentTab;

    for (int i = 0; i < locationsValueTree.getNumChildren(); ++i)
    {
        auto location = locationsValueTree.getChild(i);
        auto tabName = location.getProperty(TreeIDs::locationName).toString();
        auto tabPath = location.getProperty(TreeIDs::locationPath).toString();
    
        if (tabName.compare(newCurrentTabName) == 0)
        {
            fileChooser.setDirectory(juce::File(tabPath));
            break;
        }
    }

    repaint();

    DBG("Current Tab Changed");
}

void LocationTabBar::popupMenuClickOnTab(int tabIndex, const juce::String& tabName)
{

    juce::PopupMenu menu;
    //juce::Rectangle<int> showPoint{ getM, getMouseXYRelative().getY(), 0, 0 };
    juce::PopupMenu::Options menuOptions;
    //auto button = getTabbedButtonBar().getTabButton(tabIndex);

    menu.addItem(RightClickMenuIds::rename_Id, "Rename");
    menu.addItem(RightClickMenuIds::remove_Id, "Remove");

    //setCurrentTabIndex(tabIndex, false);
    menu.showMenuAsync(menuOptions.withMousePosition(), juce::ModalCallbackFunction::create(handleTabRightClick, this));
    
}

void LocationTabBar::handleTabRightClick(int result, LocationTabBar* tabBar)
{
    DBG("CurrentTab: " + tabBar->getCurrentTabName() + ", index = " + juce::String(tabBar->getCurrentTabIndex()));
    
    if (result == RightClickMenuIds::rename_Id)
    {

    }
    else if (result == RightClickMenuIds::remove_Id)
    {
    }

    //tabBar->setCurrentTabIndex(tabBar->lastSelectedIndex);
}


//void LocationTabBar::findAndAddRoots()
//{
//   
//    {
//        /*auto name = root.getFileName();
//        auto path = root.getFullPathName();*/
//
//        addLocation(root, root.getFileName());
//    }
//    repaint();
//}

//=================================================================================================================================//

FileChooser::FileChooser()
    :locationTabs(*this)
{
    //getLookAndFeel().setDefaultLookAndFeel()

    directoryList.setDirectory(defaultLocation, true, true);
    fileTree.setItemHeight(Dimensions::rowHeight);
    fileTree.setColour(juce::TreeView::ColourIds::selectedItemBackgroundColourId, juce::Colours::darkgrey.darker(0.3f));
    fileTree.setDragAndDropDescription(DragStrings::fileChooserDragString);
    fileTree.refresh();
    fileChooserThread.startThread(4);
    
    addAndMakeVisible(fileTree);

    addAndMakeVisible(locationTabs);

    addAndMakeVisible(currentPathBox);
    currentPathBox.setColour(juce::ComboBox::ColourIds::backgroundColourId, juce::Colours::black.withAlpha(0.2f));
    currentPathBox.setColour(juce::ComboBox::ColourIds::outlineColourId, juce::Colours::black.withAlpha(0.2f));
    currentPathBox.setColour(juce::ComboBox::ColourIds::arrowColourId, juce::Colours::lightgrey.darker(0.5f));
    currentPathBox.setEditableText(true);
    //currentPathBox.onChange
    

    addAndMakeVisible(goUpButton);
    goUpButton.onClick = [this] { goUp(); };

}

FileChooser::~FileChooser()
{}


void FileChooser::paint(juce::Graphics& g)
{
    auto area = getLocalBounds();

    //g.setColour(juce::Colours::white);
    //g.drawFittedText("File Browser", area.withBottom(Dimensions::titleH), juce::Justification::centredLeft, 1);
}

void FileChooser::resized()
{
    auto area = getLocalBounds();

    int upButtonSize = Dimensions::currentPathHeight;

    currentPathBox.setBounds(area.withLeft(Dimensions::locationTabDepth).withBottom(Dimensions::currentPathHeight).withRight(area.getWidth() - upButtonSize));
    goUpButton.setBounds(area.withLeft(currentPathBox.getRight()).withBottom(upButtonSize));
 
    fileTree.setBounds(area.withTop(currentPathBox.getBottom()).withTrimmedLeft(Dimensions::locationTabDepth));
    locationTabs.setBounds(area/*.withTop(fileTree.getY())*/.withRight(fileTree.getX()));

}

void FileChooser::addLocationTabsFromTree(juce::ValueTree& locationsTree)
{
    locationTabs.addTabsFromLocationTree(locationsTree);
    repaint();
}

void FileChooser::setDirectory(juce::File newDirectory)
{
    directoryList.setDirectory(newDirectory, true, true);
    directoryList.refresh();

    currentPathBox.setText(newDirectory.getFullPathName());
}

void FileChooser::goUp()
{
    setDirectory(directoryList.getDirectory().getParentDirectory());
}

//void FileChooser::mouseDrag(const juce::MouseEvent& e)
//{
//}

//=================================================================================================================================//

KrumFileBrowser::KrumFileBrowser(juce::ValueTree& fbValueTree, juce::AudioFormatManager& formatManager, 
                                juce::ValueTree& stateTree, juce::AudioProcessorValueTreeState& apvts, KrumSampler& s)
    : audioPreviewer(&formatManager, stateTree, apvts), fileBrowserValueTree(fbValueTree), /*treeView(fbValueTree, &audioPreviewer),*/ InfoPanelComponent(FileBrowserInfoStrings::compTitle, FileBrowserInfoStrings::message),
      favoritesTreeView(&audioPreviewer)
{

    /*recentSection.reset(new RecentSection(recentFilesList));
    favoritesSection.reset(new FavoritesSection(favoritesTreeView));
    fileChooserSection.reset(new FileChooserSection(fileChooser));

    recentSection->updateOwnedComp(fbValueTree.getChildWithName(TreeIDs::RECENT));
    favoritesSection->updateOwnedComp(fbValueTree.getChildWithName(TreeIDs::FAVORITES));
    fileChooserSection->updateOwnedComp(fbValueTree.getChildWithName(TreeIDs::LOCATIONS));*/


    //TODO set default sizes, and save the last used size

    addAndMakeVisible(concertinaPanel);
    concertinaPanel.addPanel(0, &recentFilesList, false);
    concertinaPanel.setCustomPanelHeader(&recentFilesList, &recentHeader, false);
    concertinaPanel.addPanel(1, &favoritesTreeView, false);
    concertinaPanel.setCustomPanelHeader(&favoritesTreeView, &favoritesHeader, false);
    concertinaPanel.addPanel(2, &fileChooser, false);
    concertinaPanel.setCustomPanelHeader(&fileChooser, &filechooserHeader, false);

    recentFilesList.updateFileListFromTree(fbValueTree.getChildWithName(TreeIDs::RECENT));
    favoritesTreeView.updateFavoritesFromTree(fbValueTree.getChildWithName(TreeIDs::FAVORITES));
    fileChooser.addLocationTabsFromTree(fbValueTree.getChildWithName(TreeIDs::LOCATIONS));


    addAndMakeVisible(audioPreviewer);
    audioPreviewer.assignSampler(&s);
    audioPreviewer.refreshSettings();

    /*recentSection->setSiblingComponents(nullptr, favoritesSection.get());
    favoritesSection->setSiblingComponents(recentSection.get(), fileChooserSection.get());
    fileChooserSection->setSiblingComponents(favoritesSection.get(), nullptr);
    
    addAndMakeVisible(recentSection.get());

    addAndMakeVisible(favoritesSection.get());


    addAndMakeVisible(fileChooserSection.get());


    auto favButtonImage = juce::Drawable::createFromImageData(BinaryData::add_white_24dp_svg, BinaryData::add_white_24dp_svgSize);

    addFavoriteButton.setImages(favButtonImage.get());
    addFavoriteButton.setButtonText("Add New Favorite Folder");
    //addFavoriteButton.onClick = [this] { treeView.pickNewFavorite(); };

    //addFavoriteButton.setConnectedEdges(juce::Button::ConnectedOnBottom);
    //addFavoriteButton.setColour(juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    //addFavoriteButton.setColour(juce::TextButton::buttonOnColourId, fontColor.contrasting(0.2f));
    //addFavoriteButton.setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);

    //addFavoriteButton.setTooltip("Add Files or Folders that will stay with this preset");
    */

}


KrumFileBrowser::~KrumFileBrowser()
{}

void KrumFileBrowser::paint(juce::Graphics& g)
{
    auto area = getLocalBounds();
    float cornerSize = 5.0f;
    float outline = 1.0f;

    //g.setColour(fontColor);
    //g.setFont(15.0f);
    //g.drawFittedText("File Browser", area.withBottom(titleH).withTrimmedTop(5), juce::Justification::centredLeft, 1);

    //auto fileTreeFillBounds = treeView.getBounds().expanded(5).withBottom(area.getBottom() - previewerH).toFloat();
    
    //g.setColour(juce::Colours::darkgrey);
    //g.drawFittedText("Recent", area.withTrimmedTop(titleH), juce::Justification::centredLeft, 1);
    //g.drawFittedText("Favorites", area.withTrimmedTop(area.getHeight() * 0.50f), juce::Justification::centredLeft, 1);


    //g.drawRoundedRectangle(fileTreeFillBounds, cornerSize, outline);
    //g.setColour(juce::Colours::darkgrey.darker());
    //auto grade = juce::ColourGradient::vertical(juce::Colours::darkgrey.darker(),juce::Colours::black, area);
    //g.setGradientFill(grade);
    g.setColour(juce::Colours::black.withAlpha(0.001f));
    //g.setColour(juce::Colours::red);
    g.fillRoundedRectangle(area.toFloat(), cornerSize);

}

void KrumFileBrowser::resized()
{
    auto area = getLocalBounds().reduced(10);
    int spacer = 5;
    int favButtonH = 35;
    int favButtonW = 50;
    int recentTreeViewH = 90;
    int favTreeViewH = 120;
    int locationsW = 40;

    //recentFilesList.setBounds(area.withBottom(recentTreeViewH));
    //favoritesTreeView.setBounds(area./*withLeft(locationsW).*///withTop(recentSection.getBottom() + spacer).withHeight(favTreeViewH));
    //fileChooser.setBounds(area./*withLeft(locationsW).*/withTop(favoritesSection.getBottom() + spacer).withBottom(area.getBottom() - previewerH));
    /*if (recentSection.get())
    {
    }*/


    
    //recentSection->setBounds(area.withBottom(recentTreeViewH));
    //favoritesSection->setBounds(area./*withLeft(locationsW).*/withTop(recentSection->getBottom() + spacer).withHeight(favTreeViewH));
    //fileChooserSection->setBounds(area./*withLeft(locationsW).*/withTop(favoritesSection->getBottom() + spacer).withBottom(area.getBottom() - previewerH));
 
    concertinaPanel.setBounds(area.withTrimmedBottom(previewerH));
    audioPreviewer.setBounds(area.withTop(concertinaPanel.getBottom()).withRight(area.getRight()).withHeight(previewerH));
    
}

int KrumFileBrowser::getNumSelectedItems(BrowserSections section)
{
    if (section == BrowserSections::recent)
    {
        return recentFilesList.getNumSelectedRows();
        //return recentSection->getFilesList()->getNumSelectedRows();
    }
    else if (section == BrowserSections::favorites)
    {
        //return favoritesSection->getTreeView()->getNumSelectedItems();
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

void KrumFileBrowser::addFileToRecent(const juce::File file, juce::String name)
{
    //treeView.addFileToRecent(file, name);
    //recentSection->getFilesList()->addFile(file, name);
    recentFilesList.addFile(file, name);
}

bool KrumFileBrowser::doesPreviewerSupport(juce::String fileExtension)
{
    return false;
}

void KrumFileBrowser::rebuildBrowser(juce::ValueTree& newTree)
{
    //auto oldTree = treeView.getFileBrowserValueTree();
    //oldTree = newTree;
    //treeView.reCreateFileBrowserFromValueTree();
    repaint();
}

//SimpleAudioPreviewer* KrumFileBrowser::getAudioPreviewer()
//{
//    return audioPreviewer;
//}

//void KrumFileBrowser::assignAudioPreviewer(SimpleAudioPreviewer* previewer)
//{
//    if (previewer != nullptr)
//    {
//        audioPreviewer = previewer;
//        addAndMakeVisible(audioPreviewer);
//        resized();
//        repaint();
//    }
//}

void KrumFileBrowser::assignModuleContainer(KrumModuleContainer* container)
{
    //treeView.assignModuleContainer(container);
}

void KrumFileBrowser::buildDemoKit()
{
    //Find the demo kit put there by the installer, instead of this nonsense below...
    juce::File specialLocation = juce::File::getSpecialLocation(juce::File::SpecialLocationType::currentExecutableFile);
    DBG("Location: " + specialLocation.getParentDirectory().getFullPathName());
    juce::String separator = juce::File::getSeparatorString();
    juce::File demoKitFolder{ specialLocation.getParentDirectory().getFullPathName() + separator + "DemoKit" };

    auto favVt = fileBrowserValueTree.getChildWithName(TreeIDs::FAVORITES);

    for (int i = 0; i < favVt.getNumChildren(); i++)
    {
        auto childTree = favVt.getChild(i);

        if (childTree.getType() == TreeIDs::Folder)
        {
            auto name = childTree.getProperty(TreeIDs::folderName);
            if (name.toString().compare("DemoKit") == 0)
            {
                return;
            }
        }
        
    }
    
    //if this folder doesn't exist OR does exist but has no files in it, we add the binary audio files
    if (!demoKitFolder.isDirectory() || demoKitFolder.getNumberOfChildFiles(juce::File::TypesOfFileToFind::findFiles) == 0)
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
    else if (demoKitFolder.isDirectory())
    {
        demoKit = demoKitFolder;

        DBG("DemoKit Child Files: " + juce::String(demoKit.getNumberOfChildFiles(juce::File::findFiles)));
        favoritesTreeView.createNewFavoriteFolder(demoKit.getFullPathName());
    }
    
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
{
}

//SectionHeader::SectionHeader()
//    :sectionValueTree(juce::ValueTree()), parentTreeView(nullptr), InfoPanelComponent("", "")
//{
//}

RootHeaderItem::~RootHeaderItem()
{
}

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
    return 30;
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
}

PanelHeader::~PanelHeader()
{}

void PanelHeader::resized()
{
    auto area = getLocalBounds();

    int exButtonSize = 15;

    //arrowButton.setBounds(area.getRight() - exButtonSize, area.getY(), exButtonSize, exButtonSize);
    //setButtonShape(area.withLeft(area.getWidth() - 30).reduced(10).toFloat());

}

void PanelHeader::paint(juce::Graphics& g)
{
    auto area = getLocalBounds();
    auto klaf = static_cast<KrumLookAndFeel*>(&getLookAndFeel());

    g.setColour(isMouseOver() ? juce::Colours::black.withAlpha(0.4f) : juce::Colours::black.withAlpha(0.2f));
    g.fillRect(area);


    juce::Line<float> topLine{area.getBottomLeft().toFloat(), area.getBottomRight().toFloat() };
    g.setColour(juce::Colours::lightgrey.darker(0.8f));
    g.drawLine(topLine, 0.5f);


    g.setColour(juce::Colours::lightgrey.darker(0.2f));

    if (klaf)
    {
        g.setFont(klaf->getMontBoldTypeface());
    }

    g.drawFittedText(title, area.withLeft(5), juce::Justification::centredLeft, 1);
}

juce::Component* PanelHeader::getPanelComponent(PanelCompId compId)
{
    return panel.getPanel((int)compId);
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





//SectionComp::SectionComp(juce::String title, juce::String infoPanelMessage, juce::Component* ownedComp, int ownedIndex)
//    //: InfoPanelComponent(title, infoPanelMessage), ownedCompIndex (ownedIndex)
//{
//    setName(title);
//    addAndMakeVisible(ownedComp, ownedCompIndex);
//    //ownedComp->setInterceptsMouseClicks(false, true);
//    //setInterceptsMouseClicks(false, true);
//
//    setRepaintsOnMouseActivity(true);
//
//}
//
//SectionComp::~SectionComp()
//{
//}
//
//void SectionComp::resized()
//{
//    auto area = getLocalBounds();
//
//    resizer.setBounds(area.withTop(Dimensions::titleH));
//    getOwnedComp()->setBounds(area.withTrimmedTop(Dimensions::titleH));
//}
//
//void SectionComp::setTopSibling(SectionComp* topSibling)
//{
//    //siblings.add(topSibling);
//}
//
//void SectionComp::setBottomSibling(SectionComp* botSibling)
//{
//    //siblings.add(botSibling);
//}
//
//void SectionComp::setSiblingComponents(juce::Component* topComponent, juce::Component* bottomComp)
//{
//    resized();
//    resizer.setResizeComponents(topComponent, bottomComp);
//    addAndMakeVisible(resizer);
//    repaint();
//}
//
//juce::Component* SectionComp::getOwnedComp()
//{
//    return getChildComponent(ownedCompIndex);
//}
//
////=====================================================================================
//
//SectionResizerEdge::SectionResizerEdge()
//// : juce::ResizableEdgeComponent(compToResize, constrainer, edge)
//    //: topResizer(), bottomResizer()
//{
//    //setInterceptsMouseClicks(true, false);
//}
//
//SectionResizerEdge::~SectionResizerEdge()
//{
//}
//
//void SectionResizerEdge::resized()
//{
//    auto area = getLocalBounds();
//
//    if (topResizer != nullptr)
//    {
//        topResizer->setBounds(area.withTrimmedBottom(area.getHeight() /2));
//    }
//
//    if (bottomResizer != nullptr)
//    {
//        bottomResizer->setBounds(area.withTrimmedTop(area.getHeight() / 2));
//    }
//}
//
//void SectionResizerEdge::setResizeComponents(juce::Component* topComponent, juce::Component* bottomComponent)
//{
//    topResizer.reset(new juce::ResizableEdgeComponent(topComponent, nullptr, juce::ResizableEdgeComponent::Edge::topEdge));
//    bottomResizer.reset(new juce::ResizableEdgeComponent(bottomComponent, nullptr, juce::ResizableEdgeComponent::Edge::bottomEdge));
//
//    addAndMakeVisible(topResizer.get());
//    addAndMakeVisible(bottomResizer.get());
//
//    auto area = getLocalBounds();
//    topResizer->setBounds(area);
//    bottomResizer->setBounds(area);
//
//    topResizer->toFront(false);
//    bottomResizer->toFront(false);
//    
//    resized();
//    repaint();
//}
//
//void SectionResizerEdge::mouseEnter(const juce::MouseEvent& e)
//{
//    setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
//}
//
//void SectionResizerEdge::mouseExit(const juce::MouseEvent& e)
//{
//    setMouseCursor(juce::MouseCursor::UpDownResizeCursor);
//}
////
////
////void SectionComp::mouseExit(const juce::MouseEvent& e)
////{
////    InfoPanelComponent::mouseExit(e);
////
////    if (contains(e.getPosition()))
////    {
////        setMouseCursor(juce::MouseCursor::NormalCursor);
////    }
////    else
////    {
////        getOwnedComp()->mouseExit(e);
////    }
////
////    getOwnedComp()->mouseExit(e);
////}
////
////void SectionComp::mouseDrag(const juce::MouseEvent& e)
////{
////    if (e.mouseWasDraggedSinceMouseDown())
////    {
////        auto mouseDownPos = e.getMous
////
////        int startX = e.getDistanceFromDragStartX();
////        int startY = e.getDistanceFromDragStartY();
////
////        auto currentXY = e.getScreenPosition();
////        auto dragDistance = e.getDistanceFromDragStart();
////
////        if (e.getMouseDownY() < currentXY.y)
////        {
////            dragDistance *= -1;
////        }
////
////
////        SectionComp* topSibling = siblings.getUnchecked((int)SectionComp::SiblingID::top);
////        SectionComp* bottomSibling = siblings.getUnchecked((int)SectionComp::SiblingID::bottom);
////        
////        if (topSibling)
////        {
////            auto topSiblingBounds = topSibling->getBoundsInParent();
////            topSibling->setTopLeftPosition(topSibling->getX(), topSibling->getY() + dragDistance);
////            topSibling->resized();
////            topSibling->repaint();
////        }
////
////        if (bottomSibling)
////        {
////            auto botSiblingBounds = bottomSibling->getBoundsInParent();
////            bottomSibling->setTopLeftPosition(bottomSibling->getX(), bottomSibling->getY() + dragDistance);
////            bottomSibling->resized();
////            bottomSibling->repaint();
////
////        }
////
////    }
////
////
////    getOwnedComp()->mouseDrag(e);
////}
////
////void SectionComp::mouseDown(const juce::MouseEvent& e)
////{
////    InfoPanelComponent::mouseDown(e);
////
////    if (contains(e.getPosition()))
////    {
////        setMouseCursor(juce::MouseCursor::NormalCursor);
////    }
////    else
////    {
////        getOwnedComp()->mouseExit(e);
////    }
////}
////
////void SectionComp::mouseUp(const juce::MouseEvent& e)
////{
////    InfoPanelComponent::mouseUp(e);
////
////    if (contains(e.getPosition()))
////    {
////        setMouseCursor(juce::MouseCursor::NormalCursor);
////    }
////    else
////    {
////        getOwnedComp()->mouseUp(e);
////    }
////}
////
////void SectionComp::mouseDoubleClick(const juce::MouseEvent& e)
////{
////    InfoPanelComponent::mouseDoubleClick(e);
////
////    if (contains(e.getPosition()))
////    {
////        expand or shrink size of siblings
////    }
////    else
////    {
////        getOwnedComp()->mouseDoubleClick(e);
////    }
////}
////
////void SectionComp::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
////{
////
////    if (contains(e.getPosition()))
////    {
////    }
////    else
////    {
////        getOwnedComp()->mouseWheelMove(e, wheel);
////    }
////}
////
////void SectionComp::mouseMagnify(const juce::MouseEvent& e, float scaleFactor)
////{
////    if (contains(e.getPosition()))
////    {
////    }
////    else
////    {
////        getOwnedComp()->mouseMagnify(e, scaleFactor);
////    }
////}
////
////bool SectionComp::hitTest(int x, int y)
////{
////    return y < Dimensions::titleH;
////}
//
////=================================================================================================================
//
//RecentSection::RecentSection(RecentFilesList& recentFilesList)
//    : SectionComp("RECENT", "stores your recently used samples", &recentFilesList)
//{
//}
//
//RecentSection::~RecentSection()
//{
//}
//
//void RecentSection::paint(juce::Graphics& g)
//{
//    auto area = getLocalBounds();
//
//    g.setColour(juce::Colours::lightgrey);
//    g.drawFittedText(getName(), area.withBottom(Dimensions::titleH), juce::Justification::centred, 1);
//
//    g.setColour(juce::Colours::black.withAlpha(0.1f));
//    g.fillRect(area.withBottom(Dimensions::titleH));
//}
//
//void RecentSection::resized()
//{
//    auto area = getLocalBounds();
//
//    getOwnedComp()->setBounds(area.withTop(Dimensions::titleH));
//}
//
//void RecentSection::updateOwnedComp(juce::ValueTree& updatedTree)
//{
//    getFilesList()->updateFileListFromTree(updatedTree);
//
//    /*auto recentFilesList = static_cast<RecentFilesList*>(getOwnedComp());
//    if (recentFilesList)
//    {
//        recentFilesList->updateFileListFromTree(updatedTree);
//    }
//    else
//    {
//        DBG("RecentFilesList is NULL");
//    }*/
//}
//
//RecentFilesList* RecentSection::getFilesList()
//{
//    auto recentFilesList = static_cast<RecentFilesList*>(getOwnedComp());
//    if (recentFilesList)
//    {
//        return recentFilesList;
//    }
//    
//    DBG("FileList is Null");
//    return nullptr;
//}
//
////=================================================================================================================
//
//FavoritesSection::FavoritesSection(FavoritesTreeView& favoritesTreeView)
//    :SectionComp("FAVORITES", "Drop files here to store them as favorites, you can drag files from here to the modules", &favoritesTreeView)
//{
//}
//
//FavoritesSection::~FavoritesSection()
//{
//}
//
//void FavoritesSection::paint(juce::Graphics& g)
//{
//    auto area = getLocalBounds();
//
//    g.setColour(juce::Colours::lightgrey);
//    g.drawFittedText(getName(), area.withBottom(Dimensions::titleH), juce::Justification::centred, 1);
//    
//    g.setColour(juce::Colours::black.withAlpha(0.1f));
//    g.fillRect(area.withBottom(Dimensions::titleH));
//}
//
//void FavoritesSection::resized()
//{
//    auto area = getLocalBounds();
//    getOwnedComp()->setBounds(area.withTop(Dimensions::titleH));
//}
//
//void FavoritesSection::updateOwnedComp(juce::ValueTree& updatedTree)
//{
//    getTreeView()->updateFavoritesFromTree(updatedTree);
//}
//
//FavoritesTreeView* FavoritesSection::getTreeView()
//{
//    auto favTreeView = static_cast<FavoritesTreeView*>(getOwnedComp());
//    if (favTreeView)
//    {
//        return favTreeView;
//    }
//    
//    DBG("FavoritesTreeView is NULL");
//    return nullptr;
//}
//
//
////=================================================================================================================
//
//FileChooserSection::FileChooserSection(FileChooser& fileChooser)
//    :SectionComp("FILE BROWSER", "Browse files on your computer, drag from here into favorites", &fileChooser)
//{
//}
//
//FileChooserSection::~FileChooserSection()
//{
//}
//
//void FileChooserSection::paint(juce::Graphics& g)
//{
//    auto area = getLocalBounds();
//
//    g.setColour(juce::Colours::lightgrey);
//    g.drawFittedText(getName(), area.withBottom(Dimensions::titleH), juce::Justification::centred, 1);
//
//    g.setColour(juce::Colours::black.withAlpha(0.1f));
//    g.fillRect(area.withBottom(Dimensions::titleH));
//}
//
//void FileChooserSection::resized()
//{
//    auto area = getLocalBounds();
//    getOwnedComp()->setBounds(area.withTop(Dimensions::titleH));
//}
//
//void FileChooserSection::updateOwnedComp(juce::ValueTree& updatedTree)
//{
//    getFileChooser()->addLocationTabsFromTree(updatedTree);
//}
//
//FileChooser* FileChooserSection::getFileChooser()
//{
//    auto fileChooser = static_cast<FileChooser*>(getOwnedComp());
//    if (fileChooser)
//    {
//        return fileChooser;
//    }
//
//    DBG("FileChooser is NULL");
//    return nullptr;
//}

//===================================================================================================
