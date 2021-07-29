/*
  ==============================================================================

    KrumFileBrowser.cpp
    Created: 26 Mar 2021 12:46:23pm
    Author:  krisc

  ==============================================================================
*/

#include "KrumFileBrowser.h"

class KrumTreeHeaderItem;


KrumTreeItem::KrumTreeItem(KrumTreeView* parentTreeView, SimpleAudioPreviewer* preview, juce::File fullPathName, juce::String name)
        : previewer(preview), parentTree(parentTreeView)
    {
        file = fullPathName;
        itemName = name;
        treeHasChanged();

        setLinesDrawnForSubItems(true);
        setDrawsInLeftMargin(true);
    } 

    bool KrumTreeItem::mightContainSubItems()
    {
        return false;
    }
    
    juce::Component* KrumTreeItem::createItemComponent()
    {
        auto newLabel = new EditableComp(*this, itemName, bgColor);
        
        return newLabel;
    }

    /*void paintItem(juce::Graphics& g, int width, int height) override
    {
        juce::Rectangle<int> area {0, 0, width, height};

        g.setColour(isSelected() ? bgColor.brighter() : bgColor);
        g.fillRect(area);

        if (!editing)
        {
            g.setColour(juce::Colours::white);
            g.drawFittedText(itemName, area.withX(5), juce::Justification::centredLeft, 1);
        }

    }*/

    void KrumTreeItem::paintHorizontalConnectingLine(juce::Graphics& g, const juce::Line<float>& line)
    {
        juce::Line<float> newLine = line;
        newLine.setStart(line.getStartX() - 20, line.getStartY());
        newLine.setEnd(line.getEndX() - 10, line.getEndY());

        //g.setColour(juce::Colours::darkgrey.darker(0.6f));
        g.setColour(juce::Colours::black);
        g.drawLine(newLine);
    }

    void KrumTreeItem::itemClicked(const juce::MouseEvent& e)
    {
       
        previewer->loadFile(file);
        if (previewer->isAutoPlayActive())
        {
            previewer->playOrStop();
        }

    }

    void KrumTreeItem::itemDoubleClicked(const juce::MouseEvent& e)
    {
        previewer->loadFile(file);
        previewer->playOrStop();
    }

    void KrumTreeItem::itemSelectionChanged(bool isNowSelected)
    {
        if (isNowSelected)
        {
            previewer->loadFile(file);
            if (previewer->isAutoPlayActive())
            {
                previewer->playOrStop();
            }
        }
    }

    void KrumTreeItem::closeLabelEditor(juce::Label* label)
    {
        if (label->getText().isNotEmpty())
        {
            itemName = label->getText();
        }

        setItemEditing(false);
        treeHasChanged();
        repaintItem();
    }

    juce::String KrumTreeItem::getUniqueName() const
    {
        return juce::String(getIndexInParent() + "-" + file.getFileName());
    }


    juce::File& KrumTreeItem::getFile()
    { 
        return file; 
    }

    juce::String KrumTreeItem::getItemName()
    { 
        return itemName; 
    }

    void KrumTreeItem::setItemName(juce::String newName)
    { 
        itemName = newName.isNotEmpty() ? newName : file.getFileName(); 
        parentTree->updateValueTree(getItemIdentifierString());
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
        parentTree->removeItem(getItemIdentifierString());
    }

    void KrumTreeItem::setBGColor(juce::Colour newColor)
    {
        bgColor = newColor;
    } 

    KrumTreeItem::EditableComp::EditableComp (KrumTreeItem& o, juce::String itemName, juce::Colour backColor)
        : owner(o), bgColor(backColor)
    {
        setText(itemName, juce::dontSendNotification);
        setTooltip(owner.getFile().getFullPathName());
    }

    void KrumTreeItem::EditableComp::paint(juce::Graphics& g)
    {
        auto area = getLocalBounds();

        g.setColour( owner.isSelected() ? bgColor : bgColor.darker(0.7f));
        g.fillRect(area);


        if (!isBeingEdited())
        {
            g.setColour(juce::Colours::lightgrey);
            g.drawFittedText(getText(), area.withLeft(5), juce::Justification::centredLeft, 1);
        }

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

    void KrumTreeItem::EditableComp::mouseDown(const juce::MouseEvent& e)
    {
        bool shift = e.mods.isShiftDown();
        bool cntrl = e.mods.isCtrlDown();
        bool itemSelected = owner.isSelected();
        int thisIndex = owner.getIndexInParent();
        int numSelectedItems = owner.parentTree->getNumSelectedItems();

        if (!itemSelected && (cntrl || shift))
        {
            if (shift)
            {
                if (numSelectedItems > 0)
                {
                    auto parentItem = owner.getParentItem();
                    int firstSelected = owner.parentTree->getSelectedItem(0)->getIndexInParent();
                    int lastSelected = owner.parentTree->getSelectedItem(numSelectedItems - 1)->getIndexInParent();

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
            DBG("Index in parent: " + juce::String(owner.getIndexInParent()));

        }

        if (e.mods.isPopupMenu())
        {
            juce::PopupMenu menu;

            menu.addItem(1, "Rename");
            menu.addItem(2, "Remove");

            juce::Rectangle<int> showPoint{ e.getMouseDownScreenX(), e.getMouseDownScreenY(), 0, 0 };
            int result = menu.showAt(showPoint);
            if (result == 1)
            {
                showEditor();
                repaint();
                owner.setItemEditing(true);
            }
            else if (result == 2)
            {
                owner.tellParentToRemoveMe();
            }
        }
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
        owner.itemDoubleClicked(e);
    }


   





//=================================================================================================================================//


    KrumTreeHeaderItem::KrumTreeHeaderItem(KrumTreeView* pTree, juce::File fullPathName, juce::String name, int numFilesHidden)
        : parentTree(pTree), numFilesExcluded(numFilesHidden)
    {
        file = fullPathName;
        headerName = name;
        treeHasChanged();
        setLinesDrawnForSubItems(true);

    }

    bool KrumTreeHeaderItem::mightContainSubItems()
    {
        //this should contain subItems
        return true;
    }

    juce::Component* KrumTreeHeaderItem::createItemComponent()
    {
        auto newHeader = new EditableHeaderComp(*this, headerName, bgColor);
        if (numFilesExcluded > 0)
        {
            newHeader->setTooltip(newHeader->getTooltip() + " (" + juce::String(numFilesExcluded) + " files excluded)");
        }
        return newHeader;
    }

    void KrumTreeHeaderItem::paintOpenCloseButton(juce::Graphics&, const juce::Rectangle< float >& area, juce::Colour backgroundColour, bool isMouseOver)
    {

    }

    void KrumTreeHeaderItem::paintVerticalConnectingLine(juce::Graphics& g, const juce::Line<float>& line)
    {
        juce::Line<float> newLine = line;

        g.setColour(juce::Colours::black);
        g.drawLine(newLine);

    }

    void KrumTreeHeaderItem::paintHorizontalConnectingLine(juce::Graphics& g, const juce::Line<float>& line)
    {
        if (isOpen())
        {
            //g.setColour(juce::Colours::darkgrey.darker(0.6f));
            g.setColour(juce::Colours::black);
            g.drawLine(line);
        }
    }

    void KrumTreeHeaderItem::itemClicked(const juce::MouseEvent& e)
    {
        //repaintItem();
        //DBG(headerName + " Clicked");
    }

    void KrumTreeHeaderItem::itemDoubleClicked(const juce::MouseEvent& e)
    {
        setOpen(!isOpen());
    }

    juce::File& KrumTreeHeaderItem::getFile() { return file; }

    juce::String KrumTreeHeaderItem::getItemHeaderName() { return headerName; }
    void KrumTreeHeaderItem::setItemHeaderName(juce::String newName)
    { 
        headerName = newName.isNotEmpty() ? newName : file.getFileName();
        parentTree->updateValueTree(getItemIdentifierString());
    }

    juce::String KrumTreeHeaderItem::getUniqueName() const
    {
        return juce::String(getIndexInParent() + "-" + file.getFileName());
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
                    auto headerItem = parentTree->makeHeaderItem(item);
                    if (headerItem && headerItem->isItemEditing(true))
                    {
                        return true;
                    }
                }
                else
                {
                    auto treeItem = parentTree->makeTreeItem(item);
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
        parentTree->removeItem(getItemIdentifierString());
    }

    void KrumTreeHeaderItem::clearAllChildren()
    {
        int index = getIndexInParent();
        if (index == FileBrowserSectionIds::recentFolders_Ids)
        {
            parentTree->clearRecent();
        }
        else if (index == FileBrowserSectionIds::favoritesFolders_Ids)
        {
            parentTree->clearFavorites();
        }
    }

    void KrumTreeHeaderItem::setNumFilesExcluded(int numFilesHidden)
    {
        numFilesExcluded = numFilesHidden;
        parentTree->updateValueTree(getItemIdentifierString());
    }

    int KrumTreeHeaderItem::getNumFilesExcluded()
    {
        return numFilesExcluded;
    }

    KrumTreeHeaderItem::EditableHeaderComp::EditableHeaderComp(KrumTreeHeaderItem& o, juce::String itemName, juce::Colour backColor)
        : owner(o), bgColor(backColor)
    {
        setText(itemName, juce::dontSendNotification);
        setTooltip(owner.getFile().getFullPathName());
    }

    void KrumTreeHeaderItem::EditableHeaderComp::paint(juce::Graphics& g)
    {
        auto area = getLocalBounds();
        g.setColour(owner.isSelected() ? bgColor : bgColor.darker(0.7f));
        g.fillRect(area);

        if (!isBeingEdited())
        {
            g.setColour(juce::Colours::lightgrey);
            g.drawFittedText(getText(), area.withLeft(5), juce::Justification::centredLeft, 1);

            /*if (owner.getNumFilesExcluded() > 0)
            {
                g.setFont(g.getCurrentFont().getHeightInPoints() - 6.0f);
                int leftStart = g.getCurrentFont().getStringWidth(getText());
                g.drawFittedText(juce::String(owner.getNumFilesExcluded()) + " excluded.", area.withTrimmedLeft(leftStart), juce::Justification::centredLeft, 1);
            }*/

            if (owner.isEditable())
            {
                g.setFont(g.getCurrentFont().getHeightInPoints() - 5.0f);
                g.drawFittedText("Folder", area.withTrimmedRight(5) , juce::Justification::centredRight, 1);
            }
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

        //owner.tellParentIfEditing(false);
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

            menu.addItem(1, "Rename");
            menu.addItem(2, "Remove");

            int result = menu.showAt(showPoint);
            if (result == 1)
            {

                //need to update Value Tree
                showEditor();
                repaint();
                owner.setItemEditing(true);
                //owner.tellParentIfEditing(true);
            }
            else if (result == 2)
            {
                owner.tellParentToRemoveMe();
            }
        }
        else if(e.mods.isPopupMenu() && !owner.isEditable())
        {
            juce::PopupMenu menu;
            menu.addItem(1, "Clear (like, forever)");

            int result = menu.showAt(showPoint);
            if (result)
            {
                owner.clearAllChildren();
            }
        }

    }

    void KrumTreeHeaderItem::EditableHeaderComp::mouseDoubleClick(const juce::MouseEvent& e)
    {
        owner.itemDoubleClicked(e);
    }


//=================================================================================================================================//
//=================================================================================================================================//

KrumTreeView::KrumTreeView(juce::ValueTree& fileBrowserTree, SimpleAudioPreviewer* prev)
    : fileBrowserValueTree(fileBrowserTree), previewer(prev)
{
    
    setMultiSelectEnabled(true);
    addMouseListener(this, true);
    

    rootNode.reset(new KrumTreeHeaderItem(this, juce::File(), "User Folders"));
    rootNode->setLinesDrawnForSubItems(true);
    setRootItem(rootNode.get());
    setRootItemVisible(false);

    auto recentNode = new KrumTreeHeaderItem(this, juce::File(), FileBrowserValueTreeIds::recentFolderId);
    recentNode->setOpen(true);
    recentNode->setLinesDrawnForSubItems(true);
    recentNode->setBGColor(juce::Colours::cadetblue);
    recentNode->setEditable(false);

    auto favNode = new KrumTreeHeaderItem(this, juce::File(), FileBrowserValueTreeIds::favoritesFolderId);
    favNode->setOpen(true);
    favNode->setLinesDrawnForSubItems(true);
    favNode->setBGColor(juce::Colours::cadetblue);
    favNode->setEditable(false);

    rootNode->addSubItem(recentNode, FileBrowserSectionIds::recentFolders_Ids);
    rootNode->addSubItem(favNode, FileBrowserSectionIds::favoritesFolders_Ids);
    addDummyChild();

    if (fileBrowserValueTree.getChildWithName(favNode->getItemHeaderName()).getNumChildren() > 0)
    {
        reCreateFileBrowserFromTree();
        
        sortFiles();
    } 

    setPaintingIsUnclipped(true);

   

}

KrumTreeView::~KrumTreeView()
{
    setRootItem(nullptr);
}

void KrumTreeView::paint(juce::Graphics& g) 
{

    auto area = getLocalBounds();
    g.setColour(juce::Colours::darkgrey.darker(0.7f));

    g.fillRoundedRectangle(area.expanded(5).toFloat(), 5.0f);

    juce::TreeView::paint(g);

}

juce::Rectangle<int> KrumTreeView::getTreeViewBounds()
{
    return rootNode->getOwnerView()->getBounds();
}


void KrumTreeView::refreshChildren()
{
    auto area = getLocalBounds();

    DBG("FileBrowser Area: " + area.toString());

}

void KrumTreeView::deselectAllItems()
{
    rootNode->setSelected(false, true);
}

bool KrumTreeView::isInterestedInFileDrag(const juce::StringArray& files) 
{
    
    auto wildcard = previewer->getFormatManager().getWildcardForAllFormats();
    
   /* for (auto fileString : files)
    {
        juce::File file(fileString);
        if (file.isDirectory() || wildcard.containsWholeWord(file.getFileExtension()))
        {
            return true;
        }
        else
        {
            DBG("File not supported: " + fileString);
        }
    }
    
    return false;*/

    return true;
}

void KrumTreeView::filesDropped(const juce::StringArray& files, int x, int y) 
{
   
    for (auto fileName : files)
    {
        juce::File leadFile(fileName);
        if (leadFile.isDirectory())
        {
            createNewFavoriteFolder(leadFile.getFullPathName());
        }
        else
        {
            createNewFavoriteFile(leadFile.getFullPathName());
        }
    }

}

void KrumTreeView::pickNewFavorite()
{
    const juce::String title{ "Choose A Folder(s) or File(s) to Add to Favorites, you can also drag and drop from external apps!" };

    juce::FileChooser fileChooser(title, juce::File::getSpecialLocation(juce::File::userDesktopDirectory),
                                    previewer->getFormatManager().getWildcardForAllFormats(), true);

    
    if (fileChooser.browseForMultipleFilesOrDirectories())
    {
        juce::Array<juce::File> resultFiles = fileChooser.getResults();

        for (int i = 0; i < resultFiles.size(); i++)
        {
            auto itFile = resultFiles[i];
            if (itFile.isDirectory())
            {
                createNewFavoriteFolder(itFile.getFullPathName());
            }
            else
            {
                createNewFavoriteFile(itFile.getFullPathName());
            }
        }

        auto favNode = rootNode->getSubItem(FileBrowserSectionIds::favoritesFolders_Ids);
        FileBrowserSorter sorter;
        favNode->sortSubItems(sorter);
        favNode->treeHasChanged();

    }

}

//This does NOT check file type, so make sure to you check for formatting before adding it here.
void KrumTreeView::addFileToRecent(juce::File file, juce::String name) 
{
    auto recentNode = rootNode->getSubItem(recentFolders_Ids);

    for (int i = 0; i < recentNode->getNumSubItems(); i++)
    {
        auto itItem = recentNode->getSubItem(i);
        auto krumItem = makeTreeItem(itItem);
        if (krumItem != nullptr && krumItem->getFile().getFullPathName().compare(file.getFullPathName()) == 0)
        {
            //if the recent file is already in the recent folder, we don't want to add it again
            return; 
        }
    }

    recentNode->addSubItem(new KrumTreeItem(this ,previewer, file, name));

    auto recentTree = fileBrowserValueTree.getChildWithName("Recent");
    recentTree.addChild({ "File", {{"name", name}, {"path", file.getFullPathName()}} }, recentTree.getNumChildren() + 1, nullptr);

}

void KrumTreeView::createNewFavoriteFile(const juce::String& fullPathName)
{
    auto file = juce::File(fullPathName);

    if (hasAudioFormat(file.getFileExtension()))
    {
        auto favNode = rootNode->getSubItem(favoritesFolders_Ids);
        favNode->addSubItem(new KrumTreeItem(this, previewer, file, file.getFileName()));

        auto favTree = fileBrowserValueTree.getChildWithName("Favorites");
        favTree.addChild({ "File", {{"name", file.getFileName()}, {"path", file.getFullPathName()}} }, favTree.getNumChildren() + 1, nullptr);
    }
    else
    {
        DBG("Audio Format Not Supported");
    }
}

void KrumTreeView::createNewFavoriteFolder(const juce::String& fullPathName)
{
    juce::File folder(fullPathName);

    if (folder.isDirectory())
    {
        auto favNode = rootNode->getSubItem(favoritesFolders_Ids);
        auto newFavFolderNode = new KrumTreeHeaderItem(this, folder, folder.getFileName());
        favNode->addSubItem(newFavFolderNode);

        auto favTree = fileBrowserValueTree.getChildWithName("Favorites");
        juce::ValueTree newFolderTree{ "Folder", {{ "name", folder.getFileName() }, {"path", fullPathName}, {"hiddenFiles", juce::String(0)}} };
        
        juce::Array<juce::File> childFiles = folder.findChildFiles(juce::File::findFilesAndDirectories + juce::File::ignoreHiddenFiles, false);
        int numHiddenFiles = 0;
        
        for (int j = 0; j < childFiles.size(); j++)
        {
            auto childFile = childFiles[j];
            
            if (childFile.isDirectory())
            {
                addNewFavoriteSubFolder(childFile, numHiddenFiles, newFavFolderNode, newFolderTree);
            }
            else if(hasAudioFormat(childFile.getFileExtension()))
            {
                auto childFilePath = childFile.getFullPathName();
                auto childFileName = childFile.getFileName();
                auto childFileNode = new KrumTreeItem(this, previewer, childFile, childFileName);
                
                newFavFolderNode->addSubItem(childFileNode);
                newFolderTree.addChild({ "File", {{"name", childFileName}, {"path", childFilePath}} }, j, nullptr);
            }
            else
            {
                ++numHiddenFiles;
                DBG(childFile.getFullPathName() + " is not a supported Audio Format");
            }

        }
        
        favTree.addChild(newFolderTree, favTree.getNumChildren() + 1, nullptr);
        
        if (numHiddenFiles > 0)
        {
            newFavFolderNode->setNumFilesExcluded(numHiddenFiles);
        }

        DBG(fileBrowserValueTree.toXmlString());

    }

    sortFiles();


}

void KrumTreeView::addNewFavoriteSubFolder(juce::File& folder, int& numHiddenFiles, KrumTreeHeaderItem* parentNode, juce::ValueTree& parentTree)
{
    if (folder.isDirectory())
    {
        auto newSubFolderNode = new KrumTreeHeaderItem(this, folder , folder.getFileName());
        parentNode->addSubItem(newSubFolderNode);

        juce::ValueTree newSubFolderTree = { "Folder", {{ "name", folder.getFileName() }, {"path", folder.getFullPathName()}, {"hiddenFiles", juce::String(0)}} };

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
                auto childFileNode = new KrumTreeItem(this, previewer, childFile, childFileName);

                newSubFolderNode->addSubItem(childFileNode);
                newSubFolderTree.addChild({ "File", {{"name", childFileName}, {"path", childFilePath}} }, i, nullptr);
            }
            else
            {
                ++numHiddenFiles;
                DBG(childFile.getFullPathName() + " is not a supported Audio Format");
            }

        }

        parentTree.addChild(newSubFolderTree, parentTree.getNumChildren() + 1 ,nullptr);

        if (numHiddenFiles > 0)
        {
            newSubFolderNode->setNumFilesExcluded(numHiddenFiles);
        }

    }
    
}


void KrumTreeView::reCreateFileBrowserFromTree()
{
    auto recTreeNode = fileBrowserValueTree.getChildWithName(FileBrowserValueTreeIds::recentFolderId);
    for (int i = 0; i < recTreeNode.getNumChildren(); i++)
    {
        auto childTree = recTreeNode.getChild(i);
        if (childTree.getType().toString().compare(FileBrowserValueTreeIds::fileId) == 0)
        {
            auto fileNameVar = childTree.getProperty(FileBrowserValueTreeIds::itemNameId);
            auto filePathVar = childTree.getProperty(FileBrowserValueTreeIds::pathId);
            reCreateRecentFile(fileNameVar.toString(), filePathVar.toString());
        }

    }

    auto favTreeNode = fileBrowserValueTree.getChildWithName(FileBrowserValueTreeIds::favoritesFolderId);
    for (int i = 0; i < favTreeNode.getNumChildren(); i++)
    {
        auto childTree = favTreeNode.getChild(i);

        if (childTree.getType().toString().compare(FileBrowserValueTreeIds::folderId) == 0)
        {
            auto folderNameVar = childTree.getProperty(FileBrowserValueTreeIds::itemNameId);
            auto folderPathVar = childTree.getProperty(FileBrowserValueTreeIds::pathId);
            auto folderHiddenFileVar = childTree.getProperty(FileBrowserValueTreeIds::hiddenFilesId);
            reCreateFavoriteFolder(childTree, folderNameVar.toString(), folderPathVar.toString(), folderHiddenFileVar.toString().getIntValue());
        }

        if (childTree.getType().toString().compare(FileBrowserValueTreeIds::fileId) == 0)
        {
            auto fileNameVar = childTree.getProperty(FileBrowserValueTreeIds::itemNameId);
            auto filePathVar = childTree.getProperty(FileBrowserValueTreeIds::pathId);
            reCreateFavoriteFile(fileNameVar.toString(), filePathVar.toString());
        }
    }


    auto opennessXml = fileBrowserValueTree.getChildWithName(FileBrowserValueTreeIds::opennessId).createXml();
    restoreOpennessState(*opennessXml, false);

}

void KrumTreeView::reCreateFavoriteFolder(juce::ValueTree& tree, juce::String name, juce::String fullPath, int numHiddenFiles)
{
    //should break these out into seperate functions??

    juce::File folder(fullPath);
    auto favNode = rootNode->getSubItem(favoritesFolders_Ids);
    auto newFavFolderNode = new KrumTreeHeaderItem(this, folder, name, numHiddenFiles);
    newFavFolderNode->setLinesDrawnForSubItems(true);


    for (int i = 0; i < tree.getNumChildren(); i++)
    {
        auto childTree = tree.getChild(i);

        if (childTree.getType().toString().compare(FileBrowserValueTreeIds::fileId) == 0)
        {
            auto fileNameVar = childTree.getProperty(FileBrowserValueTreeIds::itemNameId);
            auto filePathVar = childTree.getProperty(FileBrowserValueTreeIds::pathId);

            auto childFileNode = new KrumTreeItem(this, previewer, juce::File(filePathVar.toString()), fileNameVar.toString());
            childFileNode->setLinesDrawnForSubItems(true);
            newFavFolderNode->addSubItem(childFileNode);

        }
        else if (childTree.getType().toString().compare(FileBrowserValueTreeIds::folderId) == 0)
        {

            reCreateFavoriteSubFolder(newFavFolderNode, childTree);

            
            /*for (int j = 0; j < childTree.getNumChildren(); j++)
            {
                auto subChildTree = childTree.getChild(j);
                if (subChildTree.getType().toString().compare(FileBrowserValueTreeIds::fileId) == 0)
                {
                    auto fileNameVar = childTree.getProperty(FileBrowserValueTreeIds::itemNameId);
                    auto filePathVar = childTree.getProperty(FileBrowserValueTreeIds::pathId);

                    auto childFileNode = new KrumTreeItem(this, previewer, juce::File(filePathVar.toString()), fileNameVar.toString());
                    childFileNode->setLinesDrawnForSubItems(true);
                    newFavFolderNode->addSubItem(childFileNode);
                }
                else if (subChildTree.getType().toString().compare(FileBrowserValueTreeIds::folderId) == 0)
                {
                    reCreateFavoriteSubFolder(newFavFolderNode, subChildTree);
                }

            }*/
        }
    }

    favNode->addSubItem(newFavFolderNode);
}

void KrumTreeView::reCreateFavoriteSubFolder(KrumTreeHeaderItem* parentNode, juce::ValueTree& parentTree)
{
    auto folderNameVar = parentTree.getProperty(FileBrowserValueTreeIds::itemNameId);
    auto folderPathVar = parentTree.getProperty(FileBrowserValueTreeIds::pathId);
    auto folderHiddenFiVar = parentTree.getProperty(FileBrowserValueTreeIds::hiddenFilesId);

    auto folderNode = new KrumTreeHeaderItem(this, juce::File(folderPathVar.toString()), folderNameVar.toString(), folderHiddenFiVar.toString().getIntValue());
    folderNode->setLinesDrawnForSubItems(true);


    for (int i = 0; i < parentTree.getNumChildren(); i++)
    {
        auto childTree = parentTree.getChild(i);
        auto childTypeString = childTree.getType().toString();

        if (childTypeString.compare(FileBrowserValueTreeIds::fileId) == 0)
        {
            auto fileNameVar = childTree.getProperty(FileBrowserValueTreeIds::itemNameId);
            auto filePathVar = childTree.getProperty(FileBrowserValueTreeIds::pathId);

            auto childFile = new KrumTreeItem(this, previewer, juce::File(filePathVar.toString()), fileNameVar.toString());
            folderNode->addSubItem(childFile);

        }
        else if (childTypeString.compare(FileBrowserValueTreeIds::folderId) == 0)
        {
            reCreateFavoriteSubFolder(folderNode, childTree);
        }
    }

    parentNode->addSubItem(folderNode);
}

//creates a direct child file item in the "Favorites" section
void KrumTreeView::reCreateFavoriteFile(juce::String name, juce::String fullPath)
{
    juce::File file(fullPath);
    auto favNode = rootNode->getSubItem(favoritesFolders_Ids);
    favNode->addSubItem(new KrumTreeItem(this, previewer, file, name));
}

//Creates a direct child file item in the "Recent" section
void KrumTreeView::reCreateRecentFile(juce::String name, juce::String fullPath)
{
    juce::File file(fullPath);
    auto recNode = rootNode->getSubItem(FileBrowserSectionIds::recentFolders_Ids);
    recNode->addSubItem(new KrumTreeItem(this, previewer, file, name));

}

void KrumTreeView::sortFiles(FileBrowserSortingIds sortingId)
{

    FileBrowserSorter sorter;
    rootNode->sortSubItems<FileBrowserSorter>(sorter);
    rootNode->treeHasChanged();

}


void KrumTreeView::addDummyChild(juce::TreeViewItem* nodeToAddTo)
{
    if (nodeToAddTo == nullptr)
    {
        rootNode->addSubItem(new DummyTreeItem());
    }
    else
    {
        nodeToAddTo->addSubItem(new DummyTreeItem());
    }
}

bool KrumTreeView::hasAudioFormat(juce::String fileExtension)
{
    auto audioFormat = previewer->getFormatManager().findFormatForFileExtension(fileExtension);
    return audioFormat != nullptr;
}





//Updates an item Name and Number of Hidden Files, if applicable
void KrumTreeView::updateValueTree(juce::String idString)
{
    //updateOpenness();

    DBG("ValueTree ID: " + idString);

    KrumTreeHeaderItem* headerItem = nullptr;
    KrumTreeItem* treeItem = nullptr;

    auto item = findItemFromIdentifierString(idString);
    
    auto favTree = fileBrowserValueTree.getChildWithName(FileBrowserValueTreeIds::favoritesFolderId);
    int numChildren = favTree.getNumChildren();

    if (item->mightContainSubItems())
    {
        headerItem = makeHeaderItem(item);
        if (headerItem != nullptr)
        {
            //Finds folders only
            for (int i = 0; i < numChildren; i++)
            {
                auto childTree = favTree.getChild(i);
                if (childTree.getType().toString().compare(FileBrowserValueTreeIds::folderId) == 0)
                {
                    auto childPathVar = childTree.getProperty(FileBrowserValueTreeIds::pathId);
                    if (headerItem->getFile().getFullPathName().compare(childPathVar.toString()) == 0)
                    {
                        childTree.setProperty(FileBrowserValueTreeIds::itemNameId, juce::var(headerItem->getItemHeaderName()), nullptr);
                        childTree.setProperty(FileBrowserValueTreeIds::hiddenFilesId, juce::var(headerItem->getNumFilesExcluded()), nullptr);
                        return;
                    }
                }
            }
        }
    }
    else
    {
        treeItem = makeTreeItem(item);

        if (treeItem != nullptr)
        {
            //Finds files saved directly in favorites
            for (int i = 0; i < numChildren; i++)
            {
                auto childTree = favTree.getChild(i);
                if (childTree.getType().toString().compare(FileBrowserValueTreeIds::fileId) == 0)
                {
                    auto childPathVar = childTree.getProperty(FileBrowserValueTreeIds::pathId);
                    if (treeItem->getFile().getFullPathName().compare(childPathVar.toString()) == 0)
                    {
                        childTree.setProperty(FileBrowserValueTreeIds::itemNameId, juce::var(treeItem->getItemName()), nullptr);
                        return;
                    }
                }
            }

            
            //Finds files in a folder in favorites
            for (int i = 0; i < numChildren; i++)
            {
                auto childTree = favTree.getChild(i);
                if (childTree.getType().toString().compare(FileBrowserValueTreeIds::folderId) == 0)
                {
                    for (int j = 0; j < childTree.getNumChildren(); j++)
                    {
                        auto childFileTree = childTree.getChild(j);
                        if (childFileTree.getType().toString().compare(FileBrowserValueTreeIds::fileId) == 0)
                        {
                            auto childPathVar = childFileTree.getProperty(FileBrowserValueTreeIds::pathId);
                            if (treeItem->getFile().getFullPathName().compare(childPathVar.toString()) == 0)
                            {
                                childFileTree.setProperty(FileBrowserValueTreeIds::itemNameId, juce::var(treeItem->getItemName()), nullptr);
                                return;
                            }
                        }
                    }
                }
            }
        }
    }
    
    DBG("Didn't save change");
  
}

void KrumTreeView::removeValueTreeItem(juce::String fullPathName, FileBrowserSectionIds browserSection)
{

    juce::ValueTree sectionTree = fileBrowserValueTree.getChild(browserSection);

    for (int i = 0; i < sectionTree.getNumChildren(); i++)
    {
        juce::ValueTree childTree = sectionTree.getChild(i);
        auto childTreePath = childTree.getProperty(FileBrowserValueTreeIds::pathId);

        if (childTreePath.toString().compare(fullPathName) == 0)
        {
            DBG("Item to Remove: " + childTreePath.toString());
            sectionTree.removeChild(childTree, nullptr);

        }
        else
        {
            juce::ValueTree nestedChild = findTreeItem(childTree, fullPathName); 
            if (nestedChild.isValid())
            {
                sectionTree.removeChild(nestedChild, nullptr);
            }
            else
            {
                DBG("NestedChild Not Found");
            }
        }
    }

    

    /*for(int i = 0; i < sectionTree.getNumChildren(); i++)
    {
        const juce::ValueTree& childTree = sectionTree.getChild(i);

        if (childTree.getType().toString().compare(FileBrowserValueTreeIds::folderId) == 0)
        {
            for (int j = 0; j < childTree.getNumChildren(); j++)
            {
                auto& childFileTree = childTree.getChild(j);
                auto childFileTreePath = childFileTree.getProperty(FileBrowserValueTreeIds::pathId);

                if (childFileTreePath.toString().compare(fullPathName) == 0)
                {
                    childFileTree.removeChild(childFileTree, nullptr);
                    break;
                }
            }
        }
        

    }*/

}
juce::ValueTree KrumTreeView::findTreeItem(juce::ValueTree parentTree, juce::String fullPathName)
{
    for (int i = 0; i < parentTree.getNumChildren(); i++)
    {
        auto childTree = parentTree.getChild(i);
        auto childTreePath = childTree.getProperty(FileBrowserValueTreeIds::pathId);

        if (childTreePath.toString().compare(fullPathName) == 0)
        {
            return childTree;
        }
        else if (childTree.getType().toString().compare(FileBrowserValueTreeIds::folderId) == 0)
        {
            return findTreeItem(childTree, fullPathName);
        }
    }

    return juce::ValueTree();

}

void KrumTreeView::updateOpenness()
{
    auto xml = getOpennessState(true);
    auto openTree = fileBrowserValueTree.getChildWithName(FileBrowserValueTreeIds::opennessId);

    openTree.removeAllChildren(nullptr);
    openTree.appendChild(juce::ValueTree::fromXml(xml->toString()), nullptr);
}



void KrumTreeView::clearRecent()
{
    auto recentNode = rootNode->getSubItem(FileBrowserSectionIds::recentFolders_Ids);
    recentNode->clearSubItems();

    auto recentValueTree = fileBrowserValueTree.getChildWithName(FileBrowserValueTreeIds::recentFolderId);
    recentValueTree.removeAllChildren(nullptr);

}

void KrumTreeView::clearFavorites()
{
    auto favNode = rootNode->getSubItem(FileBrowserSectionIds::favoritesFolders_Ids);
    favNode->clearSubItems();

    auto favValueTree = fileBrowserValueTree.getChildWithName(FileBrowserValueTreeIds::favoritesFolderId);
    favValueTree.removeAllChildren(nullptr);

}

void KrumTreeView::removeItem(juce::String idString)
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
        if(treeItem)
        {
            juce::String sectionName;
            auto section = findSectionHeaderParent(item, sectionName);
            if (section)
            {
                FileBrowserSectionIds browserSectionId = FileBrowserSectionIds::favoritesFolders_Ids;
                
                if (sectionName.compare(FileBrowserValueTreeIds::recentFolderId) == 0)
                {
                    browserSectionId = FileBrowserSectionIds::recentFolders_Ids;
                }

                removeValueTreeItem(treeItem->getFile().getFullPathName(), browserSectionId);
                treeItem->removeThisItem();

            }
        }
    }

}

void KrumTreeView::mouseDrag(const juce::MouseEvent& event) 
{
   
    if (!areAnyItemsBeingEdited() && event.mouseWasDraggedSinceMouseDown())
    {
        juce::var description{ "FileBrowser-Drag" };
        juce::Image itemImage;
        std::unique_ptr<NumberBubble> numBub;

        auto treeItem = static_cast<KrumTreeItem*>(getSelectedItem(0)); 
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
    }
}

void KrumTreeView::setItemEditing(juce::String idString, bool isEditing)
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


bool KrumTreeView::areAnyItemsBeingEdited()
{

    auto faveNode = rootNode->getSubItem(FileBrowserSectionIds::favoritesFolders_Ids);

    for (int i = 0; i < faveNode->getNumSubItems(); i++)
    {
        auto faveFileNode = faveNode->getSubItem(i);
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


juce::ValueTree& KrumTreeView::getFileBrowserValueTree()
{
    return fileBrowserValueTree;
}

KrumTreeHeaderItem* KrumTreeView::getRootNode()
{
    return rootNode.get();
}





KrumTreeHeaderItem* KrumTreeView::makeHeaderItem(juce::TreeViewItem* item)
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


KrumTreeItem* KrumTreeView::makeTreeItem(juce::TreeViewItem* item)
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


KrumTreeItem* KrumTreeView::makeTreeItem(juce::Component* item)
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



KrumTreeHeaderItem* KrumTreeView::findSectionHeaderParent(juce::TreeViewItem* item, juce::String& sectionName)
{
    
    if (item->mightContainSubItems())
    {
        auto headerItem = static_cast<KrumTreeHeaderItem*>(item);
        if (headerItem)
        {
            auto parentItem = static_cast<KrumTreeHeaderItem*>(headerItem->getParentItem());
            if (parentItem)
            {
                bool fav = parentItem->getItemHeaderName().compare(FileBrowserValueTreeIds::favoritesFolderId) == 0;
                bool recent = parentItem->getItemHeaderName().compare(FileBrowserValueTreeIds::recentFolderId) == 0;
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
            bool fav = parentItem->getItemHeaderName().compare(FileBrowserValueTreeIds::favoritesFolderId) == 0;
            bool recent = parentItem->getItemHeaderName().compare(FileBrowserValueTreeIds::recentFolderId) == 0;
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



//=================================================================================================================================//
//=================================================================================================================================//
//=================================================================================================================================//

KrumFileBrowser::KrumFileBrowser(juce::ValueTree& previewerGainTree, juce::ValueTree& fileBrowserValueTree, juce::AudioFormatManager& formatManager)
    : audioPreviewer(formatManager, previewerGainTree), fileTree(fileBrowserValueTree, &audioPreviewer)
{

    addAndMakeVisible(audioPreviewer);
    audioPreviewer.toFront(false);

    addAndMakeVisible(fileTree);

    addAndMakeVisible(addFavoriteButton);

    //juce::String fileString = "C:\\Users\\krisc\\Documents\\Code Projects\\KrumSampler\\Resources\\add_white_24dp.svg";
    juce::String fileString = "Code Projects/KrumSampler/Resources/add_white_24dp.svg";
    
//#if JUCE_MACOS
//    fileString.clear();
//    fileString = "Users/kriscrawford/Documents/CodeProjects/KrumSampler/Resources/add_white_24dp.svg";
//#endif

    
    juce::File favButtonImFile =                     juce::File::getSpecialLocation(juce::File::SpecialLocationType::userDocumentsDirectory).getChildFile(fileString);
    
    auto favButtonImage = juce::Drawable::createFromSVGFile(favButtonImFile);

    if (favButtonImage == nullptr)
    {
        DBG("Fav Button File: " + juce::String (favButtonImFile.getFullPathName()));
        DBG("Fav Button Null");
    }
    
    addFavoriteButton.setImages(favButtonImage.get());
    addFavoriteButton.setButtonText("Add New Favorite Folder");
    addFavoriteButton.onClick = [this] { fileTree.pickNewFavorite(); };

    addFavoriteButton.setConnectedEdges(juce::Button::ConnectedOnBottom);
    addFavoriteButton.setColour(juce::TextButton::buttonColourId, juce::Colours::black);
    addFavoriteButton.setColour(juce::TextButton::buttonOnColourId, fontColor.contrasting(0.2f));

    addFavoriteButton.setTooltip("Add Files or Folders that will stay with this preset");
    
    //setOpaque(true);

}


KrumFileBrowser::~KrumFileBrowser()
{

}

void KrumFileBrowser::paint(juce::Graphics& g)
{
    auto area = getLocalBounds();
    float cornerSize = 5.0f;
    float outline = 1.0f;

    g.setColour(fontColor);
    g.setFont(22.0f);
    g.drawFittedText("File Browser", area.withBottom(titleH).withTrimmedTop(5).reduced(10), juce::Justification::centredLeft, 1);

    auto fileTreeFillBounds = fileTree.getBounds().expanded(5).withBottom(audioPreviewer.getBottom()).toFloat();
    g.drawRoundedRectangle(fileTreeFillBounds, cornerSize, outline);

    g.setColour(juce::Colours::darkgrey.darker());
    g.fillRoundedRectangle(fileTreeFillBounds, cornerSize);

}

void KrumFileBrowser::resized()
{
    auto area = getLocalBounds().reduced(10);
    int favButtonH = 35;
    int favButtonW = 50;
    int previewerH = 35;


    fileTree.setBounds(area.withTrimmedBottom(favButtonH).withTrimmedTop(titleH));

    addFavoriteButton.setBounds(area.withBottom(fileTree.getY() - 5).withLeft(area.getRight() - favButtonW));
   
    audioPreviewer.setBounds(area.withTop(fileTree.getBottom() + 5).withRight(area.getRight()).withHeight(previewerH));

}


int KrumFileBrowser::getNumSelectedItems()
{
    return fileTree.getNumSelectedItems();
}

//if no item is found, this will return a nullptr
KrumTreeItem* KrumFileBrowser::getSelectedItem(int index)
{
    auto item = fileTree.getSelectedItem(index);
    KrumTreeItem* krumItem = static_cast<KrumTreeItem*>(item);
    if (krumItem != nullptr)
    {
        return krumItem;
    }
    else
    {
        return nullptr;
    }
}

void KrumFileBrowser::addFileToRecent(const juce::File file, juce::String name)
{
    fileTree.addFileToRecent(file, name);
}

bool KrumFileBrowser::doesPreviewerSupport(juce::String fileExtension)
{
    return fileTree.hasAudioFormat(fileExtension);
}

void KrumFileBrowser::rebuildBrowser(juce::ValueTree& newTree)
{
    auto oldTree = fileTree.getFileBrowserValueTree();
    oldTree = newTree;
    fileTree.reCreateFileBrowserFromTree();
}

SimpleAudioPreviewer* KrumFileBrowser::getAudioPreviewer()
{
    return &audioPreviewer;
}
