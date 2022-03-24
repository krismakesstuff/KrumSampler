/*
  ==============================================================================

    KrumFileBrowser.h
    Created: 26 Mar 2021 12:46:23pm
    Author:  krisc

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "InfoPanel.h"

/*
* 
* The File Browser holds folders and files chosen by the user for quick access. These file paths will save with the plugin as well as any custom names that are given to them. 
* This holds the file path of the file and has a separate display name which can be changed by the user, but does not rename any actual files. 
* There are two sections, the "Recent" and "Favorites" sections. The Recent section gets automatically updated when a file is dropped on a new module. It will only hold files.
* The Favorites section is user selected and can be chosen from the "AddFavoriteButton", which will pull up a browser to choose files from. 
* 
* This class is a bit confusing... The KrumFileBrowser holds a KrumTreeView. The KrumTreeView holds TreeViewItems. There are two types of TreeViewItems, KrumTreeHeaderItem and KrumTreeItem. 
* The KrumTreeHeaderItem is for folders and KrumTreeItem is for files. Both also have custom component subclasses that give them some custom functionality.
* The state of the tree will be saved with each use and must be restored as well. 
* This browser also connnects to the AudioPreviewer to preview files. 
* 
* TODO:
* - Rebuild this whole thing. It could probably use it's own thread. This is one of the first things I built like 8 months ago and I hate it.
* - Add "Drives" section, that would make it a little bit easier to add favorite folders. It would just always have your drives shown.
* - Fix the DemoKit building
* 
*/

class KrumTreeItem;
class KrumTreeHeaderItem;
class KrumTreeView;
class SimpleAudioPreviewer;
class KrumModuleContainer;

//strings to access different parts of the saved ValueTree, for saving and loading TreeView(s)
namespace FileBrowserValueTreeIds
{
    //Probably should move these into the DECLARE_IDs section in Plugin processor.
    static const juce::String itemNameId{ "name" };
    static const juce::String pathId{ "path" };
    static const juce::String hiddenFilesId{ "hiddenFiles" };

}

namespace FileBrowserInfoStrings
{
    static const juce::String compTitle {"File Browser"};
    static const juce::String message {"This holds your favorite samples. You can drop samples and/or folders into the 'Favorites' section for easy access. You can also manually add files and folders by clicking the plus sign."};
}

//Ids for navigating the TreeView
enum FileBrowserSectionIds
{
    //userFolders_Ids,
    recentFolders_Ids,
    favoritesFolders_Ids,
    openness_Ids
};

//Ids for sorting
enum FileBrowserSortingIds
{
    folders_Id,
    files_Id,
    alphanumeric_Id
};

enum RightClickMenuIds
{
    rename_Id = 1,
    remove_Id,
    clear_Id,
};

//---------------------------------------
//---------------------------------------

//Represents a file in the File Browser. Contains a private subclass that responds to mouse clicks
class KrumTreeItem :    public juce::TreeViewItem,
                        public InfoPanelComponent
                        //public juce::Component
    
{
public:
    KrumTreeItem(KrumTreeView* parentTreeView, SimpleAudioPreviewer* preview, juce::File fullPathName, juce::String name = juce::String());
   
    bool mightContainSubItems() override;
    
    std::unique_ptr<juce::Component> createItemComponent() override;
    
    void paintHorizontalConnectingLine(juce::Graphics& g, const juce::Line<float>& line) override;

    void itemClicked(const juce::MouseEvent& e) override;
    void itemDoubleClicked(const juce::MouseEvent& e) override;

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

    void dragInParentTree(const juce::MouseEvent& e);

private:

    juce::File file;
    juce::String itemName;

    bool editing = false;
    
    KrumTreeView* parentTree;
    juce::Colour bgColor{ juce::Colours::darkgrey.darker() };
    SimpleAudioPreviewer* previewer;

    //-----------------------------------------

    class EditableComp : public juce::Label
    {
    public:
        EditableComp(KrumTreeItem& o, juce::String itemName, juce::Colour backColor = juce::Colour{});

        void paint(juce::Graphics& g) override;

        void textWasEdited() override;
        void editorAboutToBeHidden(juce::TextEditor* editor) override;
        
        void mouseEnter(const juce::MouseEvent& e) override;
        void mouseExit(const juce::MouseEvent& e) override;
        
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;
        void mouseDoubleClick(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;

        static void handleResult(int result, EditableComp* comp);

    private:

        KrumTreeItem& owner;
        juce::Colour bgColor;

        JUCE_LEAK_DETECTOR(EditableComp)

    };

    
    JUCE_LEAK_DETECTOR(KrumTreeItem)
};

//=================================================================================================================================//
//Similar to the KrumTreeItem, except this is meant to represent folders. So it has some different logic, but has similar structure. Also has a subclass. 
class KrumTreeHeaderItem :  public juce::TreeViewItem,
                            public InfoPanelComponent
                            //public juce::Component
{
public:
    KrumTreeHeaderItem(KrumTreeView* pTree, juce::File fullPathName, juce::String name = juce::String(), int numFilesHidden = 0);

    bool mightContainSubItems() override;

    std::unique_ptr<juce::Component> createItemComponent() override;

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

    //-------------------------------------
    
    class EditableHeaderComp : public juce::Label
    {
    public:
        EditableHeaderComp(KrumTreeHeaderItem& o, juce::String itemName, juce::Colour backColor = juce::Colour{});
        void paint(juce::Graphics& g) override;

        void textWasEdited() override;

        void editorAboutToBeHidden(juce::TextEditor* editor) override;
        
        void mouseEnter(const juce::MouseEvent& e) override;
        void mouseExit(const juce::MouseEvent& e) override;
        
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDoubleClick(const juce::MouseEvent& e) override;

        static void handleResult(int result, EditableHeaderComp* comp);

    private:
        KrumTreeHeaderItem& owner;
        juce::Colour bgColor;

        JUCE_LEAK_DETECTOR(EditableHeaderComp)
    };
    
    JUCE_LEAK_DETECTOR(KrumTreeHeaderItem)
};

//just so the connecting lines will draw to the bottom, purely for visual purposes (not using)
class DummyTreeItem : public juce::TreeViewItem
{
public:
    DummyTreeItem() { }

    bool mightContainSubItems() override { return false;}

    JUCE_LEAK_DETECTOR(DummyTreeItem)
};

//---------------------------------
//Sorting class, doesn't seem to be working correctly at the moment. Only sorting (trying) folders first, but could do alpha
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

//when dragging multiple files from the browser, this icon will show how many files you have selected. 
class NumberBubble : public juce::Component
{
public:
    NumberBubble(int numberToDisplay, juce::Colour backgroundColor, juce::Rectangle<int> parentBounds);
    ~NumberBubble() override;

    void paint(juce::Graphics& g) override;

    int number;
    juce::Colour bgColor;
};

//This TreeView holds all of the TreeViewItems declared above. All items are children of the rootNode member variable. 
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

    void dragOperationEnded(const juce::DragAndDropTarget::SourceDetails& details) override;

    void setItemEditing(juce::String idString, bool isEditing);
    bool areAnyItemsBeingEdited();

    juce::ValueTree& getFileBrowserValueTree();
    KrumTreeHeaderItem* getRootNode();

    KrumTreeHeaderItem* findSectionHeaderParent(juce::TreeViewItem* item, juce::String& sectionName);
    KrumTreeHeaderItem* makeHeaderItem(juce::TreeViewItem* item);
    KrumTreeItem* makeTreeItem(juce::TreeViewItem* item);
    KrumTreeItem* makeTreeItem(juce::Component* item);

    bool doesFolderExistInBrowser(juce::String fullPathName);

    //give the browser an address for the modulecontainer so we can tell it where the mouse is, specifically when dragging,  and it can tell the modules what to do 
    void assignModuleContainer(KrumModuleContainer* newContainer);

    juce::Colour getConnectedLineColor();

private:

    class CustomFileChooser : public juce::FileChooser
    {
    public:
        CustomFileChooser(juce::String title, juce::File locationToShow, juce::String formats, KrumTreeView* ownerTree)
            : juce::FileChooser(title, locationToShow, formats, false), owner(ownerTree)
        {}

        ~CustomFileChooser() { owner = nullptr;}

        void showFileChooser(int flags)
        {
            if (fileChooserCallback)
            {
                launchAsync(flags, fileChooserCallback);
            }
            else
            {
                DBG("Callback OR FileChooser is Null");
            }
        }

        std::function<void(const juce::FileChooser&)> fileChooserCallback;

        KrumTreeView* owner = nullptr;
    };

    std::unique_ptr<CustomFileChooser> currentFileChooser = nullptr;

    static void handleChosenFiles(const juce::FileChooser& fileChooser);
    
    juce::ValueTree& fileBrowserValueTree;

    std::unique_ptr<KrumTreeHeaderItem> rootNode;


    juce::Colour fontColor{ juce::Colours::darkgrey };
    juce::Colour bgColor{ juce::Colours::black };
    juce::Colour conLineColor{ juce::Colours::darkgrey };


    SimpleAudioPreviewer* previewer;

    KrumModuleContainer* moduleContainer = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KrumTreeView)
};


class KrumFileBrowser : public InfoPanelComponent
{
public:

    KrumFileBrowser(SimpleAudioPreviewer& previewer, juce::ValueTree& fileBroswerValueTree/*, juce::AudioFormatManager& formatManager*/);
    ~KrumFileBrowser();

    void paint(juce::Graphics& g) override;
    void resized() override;

    int getNumSelectedItems();
    KrumTreeItem* getSelectedItem(int index);

    void addFileToRecent(const juce::File file, juce::String name); 

    bool doesPreviewerSupport(juce::String fileExtension);
    SimpleAudioPreviewer* getAudioPreviewer();
    void assignModuleContainer(KrumModuleContainer* container);

    void rebuildBrowser(juce::ValueTree& newTree);
    void buildDemoKit();

private:

    juce::ValueTree& fileBrowserValueTree;

    KrumTreeView fileTree;

    SimpleAudioPreviewer& audioPreviewer;
  
    InfoPanelDrawableButton addFavoriteButton {"Add Favorites", "Opens a browser to select Folders and/or Files to add to the Favorites section", "", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground};
    
    juce::Colour fontColor{ juce::Colours::lightgrey };
    
    int titleH = 30;

#if JucePlugin_Build_Standalone
    juce::File demoKit;
#endif


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KrumFileBrowser)
};
