/*
  ==============================================================================

    KrumFileBrowser.h
    Created: 26 Mar 2021 12:46:23pm
    Author:  krisc

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "SimpleAudioPreviewer.h"

class KrumTreeItem;
class KrumTreeHeaderItem;
//class KrumTreeView;

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
    }

    FileBrowserSortingIds sortingId;
};

class NumberBubble : public juce::Component
{
public:
    NumberBubble(int numberToDisplay, juce::Colour backgroundColor, juce::Rectangle<int> parentBounds)
        : number(numberToDisplay), bgColor(backgroundColor)
    {
        //setSize(15, 15);
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
    //void setItemSelected(KrumTreeItem* itemToSet,bool isSelected, bool deselectOthers);

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
    juce::ValueTree& findTreeItem(juce::ValueTree& parentTree, juce::String fullPathName);
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

    KrumFileBrowser(juce::ValueTree& previewerGainTree, juce::ValueTree& fileBroswerValueTree, juce::AudioFormatManager& formatManager);
    ~KrumFileBrowser();

    void paint(juce::Graphics& g) override;
    void resized() override;

    //void mouseDown(const juce::MouseEvent& event) override;

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
   
    //juce::Colour fontColor{ juce::Colours::darkgrey/*.darker()*/ };
    juce::Colour fontColor{ juce::Colours::lightgrey/*.darker()*/ };
    
    int titleH = 30;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KrumFileBrowser)

};


