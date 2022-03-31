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
class FavoritesTreeView;
class SimpleAudioPreviewer;
class KrumModuleContainer;

//strings to access different parts of the saved ValueTree, for saving and loading TreeView(s)
//namespace FileBrowserValueTreeIds
//{
//    //Probably should move these into the DECLARE_IDs section in Plugin processor.
//    //static const juce::String itemNameId{ "name" };
//    //static const juce::String pathId{ "path" };
//    //static const juce::String hiddenFilesId{ "hiddenFiles" };
//
//}

namespace Dimensions
{
    const int rowHeight = 18;
    const int titleH = 21;
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
    //KrumTreeItem(KrumTreeView* parentTreeView, SimpleAudioPreviewer* preview, juce::File fullPathName, juce::String name = juce::String());
    KrumTreeItem(juce::ValueTree& fileValueTree, FavoritesTreeView* parentTreeView, SimpleAudioPreviewer* previewer);

    bool mightContainSubItems() override;
    
    std::unique_ptr<juce::Component> createItemComponent() override;
    
    void paintHorizontalConnectingLine(juce::Graphics& g, const juce::Line<float>& line) override;
    int getItemHeight() const override;

    void itemClicked(const juce::MouseEvent& e) override;
    void itemDoubleClicked(const juce::MouseEvent& e) override;

    void closeLabelEditor(juce::Label* label);

    juce::String getUniqueName() const override;
    
    juce::String getFilePath() const;
    juce::String getItemName() const;

    juce::File getFile();

    void setItemName(juce::String newName);
    bool isItemEditing();
    void setItemEditing(bool isEditing);
    void removeThisItem();

    void tellParentToRemoveMe();
    void setBGColor(juce::Colour newColor);

    void dragInParentTree(const juce::MouseEvent& e);

    juce::ValueTree getValueTree();

private:


    bool editing = false;
    
    FavoritesTreeView* parentTreeView;

    juce::ValueTree fileValueTree;

    juce::Colour bgColor{ juce::Colours::darkgrey.darker() };
    SimpleAudioPreviewer* previewer;

    //-----------------------------------------

    class EditableComp : public juce::Label
    {
    public:
        //EditableComp(KrumTreeItem& o, juce::String itemName, juce::Colour backColor = juce::Colour{});
        EditableComp(KrumTreeItem& o, juce::Colour backColor = juce::Colour{});

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
    //KrumTreeHeaderItem(KrumTreeView* pTree, juce::File fullPathName, juce::String name = juce::String(), int numFilesHidden = 0);
    KrumTreeHeaderItem(juce::ValueTree& folderValueTree, FavoritesTreeView* pTree);

    bool mightContainSubItems() override;

    std::unique_ptr<juce::Component> createItemComponent() override;

    void paintOpenCloseButton(juce::Graphics&, const juce::Rectangle< float >& area, juce::Colour backgroundColour, bool isMouseOver) override;
    void paintVerticalConnectingLine(juce::Graphics& g, const juce::Line<float>& line) override;
    void paintHorizontalConnectingLine(juce::Graphics& g, const juce::Line<float>& line) override;
    int getItemHeight() const override;

    void itemClicked(const juce::MouseEvent& e) override;
    void itemDoubleClicked(const juce::MouseEvent& e) override;

    juce::String getFolderPath() const;
    juce::String getItemHeaderName() const;
    void setItemHeaderName(juce::String newName);
    void setNumFilesExcluded(int numFilesHidden);
    int getNumFilesExcluded();

    juce::File getFile();

    juce::String getUniqueName() const override;

    void setBGColor(juce::Colour newColor);

    bool isItemEditing(bool checkChildren);
    void setItemEditing(bool isEditing);

    void setEditable(bool isEditable);
    bool isEditable();

    void removeThisHeaderItem();
    void tellParentToRemoveMe();
    void clearAllChildren();


private:

    FavoritesTreeView* parentTreeView;

    juce::ValueTree folderValueTree;

    juce::Colour bgColor{ juce::Colours::darkgrey.darker(0.6f) };

    bool editable = true;
    bool editing = false;

    //--------------------------------------------------------------------------------------
    
    class EditableHeaderComp : public juce::Label
    {
    public:
        EditableHeaderComp(KrumTreeHeaderItem& o, juce::Colour backColor = juce::Colour{});
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

class SectionHeader :   public juce::TreeViewItem,
                        public InfoPanelComponent
{
public:
    SectionHeader(juce::ValueTree& sectionValueTree, FavoritesTreeView* rootItem);
    //SectionHeader();
    ~SectionHeader() override;

    void paintOpenCloseButton(juce::Graphics& g, const juce::Rectangle<float>& area, juce::Colour bgColor, bool isMouseOver) override;
    void paintItem(juce::Graphics& g, int width, int height) override;
    int getItemHeight() const override;

    bool mightContainSubItems() override;

    void itemClicked(const juce::MouseEvent& e) override;

    static void handleResult(int result, SectionHeader* comp);

private:

    juce::ValueTree sectionValueTree;
    FavoritesTreeView* parentTreeView;

};


//--------------------------------------------------------------------------------------------------------------
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
class FavoritesTreeView :   public juce::TreeView,
                            public juce::DragAndDropContainer,
                            public juce::ValueTree::Listener
{
public:

    FavoritesTreeView(juce::ValueTree& fileBrowserTree, SimpleAudioPreviewer* prev);
    ~FavoritesTreeView();

    void valueTreePropertyChanged(juce::ValueTree& treeChanged, const juce::Identifier& property) override;

    void valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& removedChild, int indexOfRemoval) override;

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

    
    void reCreateFileBrowserFromValueTree();
    void reCreateFavoriteFolder(juce::ValueTree& tree);
    void reCreateFavoriteFile(juce::ValueTree& fileValueTree);
    void reCreateFavoriteSubFolder(KrumTreeHeaderItem* parentNode, juce::ValueTree& parentTree);
    void reCreateRecentFile(juce::ValueTree& fileValueTree);
    
    void sortFiles(FileBrowserSortingIds sortingId = FileBrowserSortingIds::folders_Id);

    void addDummyChild(juce::TreeViewItem* nodeToAddTo = nullptr);
    bool hasAudioFormat(juce::String fileExtension);

    //void updateValueTree(juce::String idString);
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


    juce::Array<juce::ValueTree> getSelectedValueTrees();

    juce::ValueTree& getFileBrowserValueTree();
    SectionHeader* getRootNode();

    KrumTreeHeaderItem* findSectionHeaderParent(juce::TreeViewItem* item, juce::String& sectionName);
    KrumTreeHeaderItem* makeHeaderItem(juce::TreeViewItem* item);
    KrumTreeItem* makeTreeItem(juce::TreeViewItem* item);
    KrumTreeItem* makeTreeItem(juce::Component* item);

    bool doesFolderExistInBrowser(juce::String fullPathName);

    //gives the browser an address to the KrumModuleContainer so we know which modules are being dragged over
    void assignModuleContainer(KrumModuleContainer* newContainer);

    juce::Colour getConnectedLineColor();
    juce::Colour getFontColor();


private:

    class CustomFileChooser : public juce::FileChooser
    {
    public:
        CustomFileChooser(juce::String title, juce::File locationToShow, juce::String formats, FavoritesTreeView* ownerTree)
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

        FavoritesTreeView* owner = nullptr;
    };


    std::unique_ptr<CustomFileChooser> currentFileChooser = nullptr;

    static void handleChosenFiles(const juce::FileChooser& fileChooser);
    
    juce::ValueTree& fileBrowserValueTree;

    //std::unique_ptr<KrumTreeHeaderItem> rootItem;
    std::unique_ptr<SectionHeader> rootItem;

    int titleH = 20;

    juce::Colour fontColor{ juce::Colours::darkgrey };
    juce::Colour bgColor{ juce::Colours::black };
    juce::Colour conLineColor{ juce::Colours::darkgrey };


    SimpleAudioPreviewer* previewer;

    KrumModuleContainer* moduleContainer = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FavoritesTreeView)
};


//-make recents sections a file list, only show 1 or a few files, with an arrow to expand to see all

class RecentFilesList : public juce::ListBoxModel, 
                        public juce::Component
{
public:
    RecentFilesList(SimpleAudioPreviewer* previewer);
    
    ~RecentFilesList() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    int getNumRows() override;
    
    void listBoxItemClicked(int row, const juce::MouseEvent& e) override;
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent& e) override;

    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;

    void addFile(juce::File fileToAdd, juce::String name);
    void updateFileListFromTree(juce::ValueTree& recentsTree);

    int getNumSelectedRows();
    juce::Array<juce::ValueTree> getSelectedValueTrees();


    juce::var getDragSourceDescription(const juce::SparseSet<int>& rowsToDescribe) override;

private:

    juce::String getFileName(int rowNumber);
    juce::String getFilePath(int rowNumber);

    bool expanded = false;
    juce::ListBox listBox{ "Recent Files", this };
    juce::ValueTree recentValueTree;

    SimpleAudioPreviewer* previewer = nullptr;
};


//class FavoritesTreeView : public juce::TreeView
//{
//public:
//    FavoritesTreeView(juce::ValueTree& favValTree) 
//        :favoritesValueTree(favValTree) 
//    {}
//    ~FavoritesTreeView() override {}
//
//    void paint(juce::Graphics& g) override
//    {
//        g.fillAll(juce::Colours::darkgreen);
//        g.setColour(juce::Colours::white);
//        g.drawFittedText("Favorites Tree", getLocalBounds(), juce::Justification::centred, 1);
//    }
//private:
//
//    juce::ValueTree& favoritesValueTree;
//
//};

class LocationTabBar : public juce::TabbedComponent
{
public:
    LocationTabBar()
        :juce::TabbedComponent(juce::TabbedButtonBar::Orientation::TabsAtLeft)
    {
        setTabBarDepth(25);
        addTab("Desktop", juce::Colours::white, createTabButton("Desktop", -1), true);
        addTab("Drive 1", juce::Colours::blue, createTabButton("Drive 1", -1), true);
        addTab("Location", juce::Colours::green, createTabButton("Location", -1), true);
        //addTab("Location", juce::Colours::green, createTabButton("Location", -1), true);
        //addTab("Location", juce::Colours::green, createTabButton("Location", -1), true);
        //addTab("Location", juce::Colours::green, createTabButton("Location", -1), true);
        //addTab("Location", juce::Colours::green, createTabButton("Location", -1), true);
    }
    ~LocationTabBar() override {}


    //  juce::TabBarButton* createTabButton(const juce::String& tabName, int tabIndex) override

private:

    //store the user added locations
    juce::ValueTree fileBrowserTree;

};

//class FileChooserTreeView : public juce::FileTreeComponent
class FileChooser : public juce::Component
{
public:
    //TODO
    /*
    * - preview audio files, need effiecent way to decide which files can be previewed
    *   - will need to handle clicks, sublclassing TreeViewItems? 
    * - connect location tabs, probably have this component hold the tabs
    *   - make custom tab paint methods
    *   - make add location tab, makes new tab from selection or a right-click on the folder (no files)
    * - drag and drop to favorites section
    * - make file chooser collapsible??
    * 
    * 
    * 
    */
    
    FileChooser()
    {
        directoryList.setDirectory(defaultLocation, true, true);
        fileTree.setItemHeight(19);
        fileTree.refresh();

        fileChooserThread.startThread(4);

        addAndMakeVisible(locationTabs);

        addAndMakeVisible(fileTree);

    }

    ~FileChooser() override {}

    void paint(juce::Graphics& g) override
    {
        auto area = getLocalBounds();


        g.setColour(juce::Colours::white);
        g.drawFittedText("File Browser", area.withBottom(titleH), juce::Justification::centredLeft, 1);
    }


    void resized() override
    {
        auto area = getLocalBounds();

        int tabDepth = 25;

        fileTree.setBounds(area.withTrimmedLeft(tabDepth).withTop(titleH));
        locationTabs.setBounds(area.withTop(fileTree.getY()).withRight(fileTree.getX()));

    }

private:
    //juce::File defaultLocation{ "C:\\Users\\krisc\\Desktop" };
    juce::File defaultLocation{ juce::File::getSpecialLocation(juce::File::SpecialLocationType::userDesktopDirectory) };
    
    LocationTabBar locationTabs;

    juce::TimeSliceThread fileChooserThread{ "FileChooserThread" };
    juce::DirectoryContentsList directoryList{ nullptr, fileChooserThread };
    juce::FileTreeComponent fileTree{ directoryList };
    
    int titleH = 20;

};


namespace DragStrings
{
    const juce::String recentsDragString{ "RecentsFileDrag-" };
    const juce::String favoritesDragString{ "FavoritesFileDrag-" };
}

class KrumFileBrowser : public InfoPanelComponent
{
public:

    enum class BrowserSections
    {
        recent,
        favorites,
        fileChooser,
    };



    KrumFileBrowser(juce::ValueTree& fileBroswerValueTree, juce::AudioFormatManager& formatManager, 
                    juce::ValueTree& stateTree, juce::AudioProcessorValueTreeState& apvts, KrumSampler& s);
    ~KrumFileBrowser();

    void paint(juce::Graphics& g) override;
    void resized() override;

    int getNumSelectedItems(BrowserSections section);
    KrumTreeItem* getSelectedItem(int index);

    juce::Array<juce::ValueTree> getSelectedFileTrees(BrowserSections section);


    void addFileToRecent(const juce::File file, juce::String name); 

    bool doesPreviewerSupport(juce::String fileExtension);
    //SimpleAudioPreviewer* getAudioPreviewer();
    
    //void assignAudioPreviewer(SimpleAudioPreviewer* previewer);
    void assignModuleContainer(KrumModuleContainer* container);

    void rebuildBrowser(juce::ValueTree& newTree);
    void buildDemoKit();

private:

    juce::ValueTree& fileBrowserValueTree;

    SimpleAudioPreviewer audioPreviewer;
  
    RecentFilesList recentFilesList{ &audioPreviewer };
    FavoritesTreeView favoritesTreeView;
    FileChooser fileChooser{};


    InfoPanelDrawableButton addFavoriteButton {"Add Favorites", "Opens a browser to select Folders and/or Files to add to the Favorites section", "", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground};
    
    juce::Colour fontColor{ juce::Colours::lightgrey };
    
    int titleH = 20;
    int previewerH = 35;

    juce::File demoKit;



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KrumFileBrowser)
};
