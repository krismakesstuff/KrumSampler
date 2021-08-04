/*
  ==============================================================================

    KrumFileBrowser.h
    Created: 26 Mar 2021 12:46:23pm
    Author:  krisc

  ==============================================================================
*/

#pragma once
#include "SimpleAudioPreviewer.h"
#include <JuceHeader.h>

/*
* 
* The File Browser holds folders and files chosen by the user for quick access. These file paths will save with the plugin as well as any custom names that are given to them. 
* This holds the file path of the file and has a separate display name which can be changed by the user, but doesnt' not rename any actual files. 
* There are two sections, the "Recent" and "Favorites" sections. The Recent section gets automatically updated when a file is dropped on the FileDrop component. It will only hold files.
* The Favorites section is user selected and can be chosen from the "AddFavoriteButton", which will pull up a browser to choose files from. 
* 
* This class is a bit confusing... The KrumFileBrowser holds a KrumTreeView. The KrumTreeView holds TreeViewItems. There are two types of TreeViewItems, KrumTreeHeaderItem and KrumTreeItem. 
* The KrumTreeHeaderItem is for folders and KrumTreeItem is for files. Both also have custom component subclasses that give them some custom functionality.
* The state of the tree will be saved with each use and must be restored as well. 
* This browser alos connnects to the AudioPreviewer to preview files. 
* 
* NOTE: The AudioPreviewer is not working on MacOS.
* 
* 
*/



class KrumTreeItem;
class KrumTreeHeaderItem;
class KrumTreeView;

namespace FileBrowserValueTreeIds
{
    static const juce::String folderId{ "Folder" };
    static const juce::String fileId{ "File" };
    static const juce::String itemNameId{ "name" };
    static const juce::String pathId{ "path" };
    static const juce::String hiddenFilesId{ "hiddenFiles" };

    static const juce::String recentFolderId{ "Recent" };
    static const juce::String favoritesFolderId{ "Favorites" };
    static const juce::String opennessId{ "OpennessState" };
}


enum FileBrowserSectionIds
{
    //userFolders_Ids,
    recentFolders_Ids,
    favoritesFolders_Ids,
    openness_Ids
};

enum FileBrowserSortingIds
{
    folders_Id,
    files_Id,
    alphanumeric_Id
};


//---------------------------------
//---------------------------------

class KrumTreeItem :    public juce::TreeViewItem,
                        public juce::Component
{
public:
    KrumTreeItem(KrumTreeView* parentTreeView, SimpleAudioPreviewer* preview, juce::File fullPathName, juce::String name = juce::String());
   
    bool mightContainSubItems() override;
    
    juce::Component* createItemComponent() override;
    
    void paintHorizontalConnectingLine(juce::Graphics& g, const juce::Line<float>& line) override;

    void itemClicked(const juce::MouseEvent& e) override;
    void itemDoubleClicked(const juce::MouseEvent& e) override;

    void itemSelectionChanged(bool isNowSelected) override;
    void closeLabelEditor(juce::Label* label);

    juce::String getUniqueName() const override;
    
    juce::File& getFile();
    juce::String getItemName();

    void setItemName(juce::String newName);
    bool isItemEditing();
    void setItemEditing(bool isEditing);
    void removeThisItem();

    void tellParentToRemoveMe();
    void setBGColor(juce::Colour newColor);
    

private:

    juce::File file;
    juce::String itemName;

    bool editing = false;

    
    juce::Colour bgColor{ juce::Colours::darkgrey.darker() };
    KrumTreeView* parentTree;
    SimpleAudioPreviewer* previewer;

    //============================================================================================================//

    class EditableComp : public juce::Label
    {
    public:
        EditableComp(KrumTreeItem& o, juce::String itemName, juce::Colour backColor = juce::Colour{});

        void paint(juce::Graphics& g) override;

        void textWasEdited() override;
        void editorAboutToBeHidden(juce::TextEditor* editor) override;

        void mouseDown(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;
        void mouseDoubleClick(const juce::MouseEvent& e) override;
        

    private:
        KrumTreeItem& owner;
        juce::Colour bgColor;

        JUCE_LEAK_DETECTOR(EditableComp)

    };

    JUCE_LEAK_DETECTOR(KrumTreeItem)

};

//=================================================================================================================================//

class KrumTreeHeaderItem :  public juce::TreeViewItem,
                            public juce::Component
{
public:
    KrumTreeHeaderItem(KrumTreeView* pTree, juce::File fullPathName, juce::String name = juce::String(), int numFilesHidden = 0);

    bool mightContainSubItems() override;

    juce::Component* createItemComponent() override;

    void paintOpenCloseButton(juce::Graphics&, const juce::Rectangle< float >& area, juce::Colour backgroundColour, bool isMouseOver) override;
    void paintVerticalConnectingLine(juce::Graphics& g, const juce::Line<float>& line) override;
    void paintHorizontalConnectingLine(juce::Graphics& g, const juce::Line<float>& line) override;

    void itemClicked(const juce::MouseEvent& e) override;
    void itemDoubleClicked(const juce::MouseEvent& e) override;

    juce::File& getFile();
    juce::String getItemHeaderName();
    void setItemHeaderName(juce::String newName);

    juce::String getUniqueName() const override;

    void setBGColor(juce::Colour newColor);

    bool isItemEditing(bool checkChildren);
    void setItemEditing(bool isEditing);

    void setEditable(bool isEditable);
    bool isEditable();

    void removeThisHeaderItem();
    void tellParentToRemoveMe();
    void clearAllChildren();

    void setNumFilesExcluded(int numFilesHidden);
    int getNumFilesExcluded();

private:

    KrumTreeView* parentTree;

    juce::File file;
    juce::String headerName;

    juce::Colour bgColor{ juce::Colours::darkgrey.darker(0.6f) };

    bool editable = true;
    bool editing = false;
    int numFilesExcluded;

//=================================================================================================================================//
    
    class EditableHeaderComp : public juce::Label
    {
    public:
        EditableHeaderComp(KrumTreeHeaderItem& o, juce::String itemName, juce::Colour backColor = juce::Colour{});
        void paint(juce::Graphics& g) override;

        void textWasEdited() override;

        void editorAboutToBeHidden(juce::TextEditor* editor) override;
        void mouseDown(const juce::MouseEvent& e) override;

        void mouseDoubleClick(const juce::MouseEvent& e) override;


    private:
        KrumTreeHeaderItem& owner;
        juce::Colour bgColor;

        JUCE_LEAK_DETECTOR(EditableHeaderComp)
    };
    
    JUCE_LEAK_DETECTOR(KrumTreeHeaderItem)
};

//just so the connecting lines will draw to the bottom, purely for visual purposes
class DummyTreeItem : public juce::TreeViewItem
{
public:
    DummyTreeItem()
    {}

    bool mightContainSubItems() override
    {
        return false;
    }

    JUCE_LEAK_DETECTOR(DummyTreeItem)
};

//---------------------------------
//---------------------------------
class FileBrowserSorter
{
public:
    //Default puts folders first
    FileBrowserSorter(FileBrowserSortingIds sortBy = FileBrowserSortingIds::folders_Id)
        : sortingId(sortBy)
    {
    }

    int compareElements(juce::TreeViewItem* first, juce::TreeViewItem* second)
    {
        if (sortingId == folders_Id)
        {
            bool firstHasSubItems = first->mightContainSubItems();
            bool secondHasSubItems = second->mightContainSubItems();

            if (firstHasSubItems && !secondHasSubItems)
            {
                return -1;
            }
            
            if (!firstHasSubItems && secondHasSubItems)
            {
                return 1;
            }

            return 0;
        }
        
        return 0;
    }

    FileBrowserSortingIds sortingId;
};

//when draggin multiple files from the browser, this icon will show how many files you have selected. 
class NumberBubble : public juce::Component
{
public:
    NumberBubble(int numberToDisplay, juce::Colour backgroundColor, juce::Rectangle<int> parentBounds)
        : number(numberToDisplay), bgColor(backgroundColor)
    {
        setBounds(parentBounds.getRight() - 20, 3, 15, 15);
    }

    ~NumberBubble() override
    {}

    void paint(juce::Graphics& g) override
    {
        g.setColour(bgColor);
        g.fillEllipse(getBounds().toFloat());
        
        g.setColour(juce::Colours::white);
        g.drawFittedText(juce::String(number), getBounds(), juce::Justification::centred, 1);

    }

    int number;
    juce::Colour bgColor;

};


class KrumTreeView :    public juce::TreeView,
                        public juce::DragAndDropContainer
{
public:

    KrumTreeView(juce::ValueTree& fileBrowserTree, SimpleAudioPreviewer* prev);
    ~KrumTreeView();

    void paint(juce::Graphics& g) override;

    juce::Rectangle<int> getTreeViewBounds();

    void refreshChildren();
    void deselectAllItems();

    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    void pickNewFavorite();
    void addFileToRecent(juce::File file, juce::String name);
    void createNewFavoriteFile(const juce::String& fullPathName);
    void createNewFavoriteFolder(const juce::String& fullPathName);
    void addNewFavoriteSubFolder(juce::File& folder, int& numHiddenFiles, KrumTreeHeaderItem* parentNode, juce::ValueTree& parentTree);

    
    void reCreateFileBrowserFromTree();
    void reCreateFavoriteFolder(juce::ValueTree& tree, juce::String name, juce::String fullPath, int hiddenFiles);
    void reCreateFavoriteFile(juce::String name, juce::String fullPath);
    void reCreateFavoriteSubFolder(KrumTreeHeaderItem* parentNode, juce::ValueTree& parentTree);
    void reCreateRecentFile(juce::String name, juce::String fullPath);
    
    void sortFiles(FileBrowserSortingIds sortingId = FileBrowserSortingIds::folders_Id);

    void addDummyChild(juce::TreeViewItem* nodeToAddTo = nullptr);
    bool hasAudioFormat(juce::String fileExtension);

    void updateValueTree(juce::String idString);
    void removeValueTreeItem(juce::String fullPathName, FileBrowserSectionIds browserSection);
    juce::ValueTree findTreeItem(juce::ValueTree parentTree, juce::String fullPathName);
    void updateOpenness();

    void clearRecent();
    void clearFavorites();

    void removeItem(juce::String idString);
    void mouseDrag(const juce::MouseEvent& event) override;

    void setItemEditing(juce::String idString, bool isEditing);
    bool areAnyItemsBeingEdited();

    juce::ValueTree& getFileBrowserValueTree();
    KrumTreeHeaderItem* getRootNode();

    KrumTreeHeaderItem* findSectionHeaderParent(juce::TreeViewItem* item, juce::String& sectionName);
    KrumTreeHeaderItem* makeHeaderItem(juce::TreeViewItem* item);
    KrumTreeItem* makeTreeItem(juce::TreeViewItem* item);
    KrumTreeItem* makeTreeItem(juce::Component* item);

private:

    juce::ValueTree& fileBrowserValueTree;

    std::unique_ptr<KrumTreeHeaderItem> rootNode;

    juce::Colour fontColor{ juce::Colours::darkgrey };
    juce::Colour bgColor{ juce::Colours::black };

    SimpleAudioPreviewer* previewer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KrumTreeView)
};


class KrumFileBrowser : public juce::Component
{
public:

    KrumFileBrowser(juce::ValueTree& valueTree, juce::ValueTree& fileBroswerValueTree, juce::AudioFormatManager& formatManager);
    ~KrumFileBrowser();

    void paint(juce::Graphics& g) override;
    void resized() override;

    int getNumSelectedItems();
    KrumTreeItem* getSelectedItem(int index);

    void addFileToRecent(const juce::File file, juce::String name); 

    bool doesPreviewerSupport(juce::String fileExtension);
    SimpleAudioPreviewer* getAudioPreviewer();
    
    void rebuildBrowser(juce::ValueTree& newTree);


private:

    KrumTreeView fileTree;

    SimpleAudioPreviewer audioPreviewer;
    juce::DrawableButton addFavoriteButton{ "Add Favorite", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground };

    juce::Colour fontColor{ juce::Colours::lightgrey };
    
    int titleH = 30;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KrumFileBrowser)
};