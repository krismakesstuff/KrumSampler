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
//#include "C:\Users\krisc\Documents\JUCE\modules\juce_gui_basics\filebrowser\juce_FileTreeComponent.cpp"
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
class FileChooser;
class KrumFileBrowser;
//class juce::FileTreeComponent::FileListTreeItem;


// 
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
    const int rowHeight = 19;
    const int titleH = 21;
    const int fileIconSize = 20;
    const float fileIconAlpha = 0.5f;
    const int locationTabDepth = 22;
    const int currentPathHeight = 18;
}

namespace Colors
{
    const juce::Colour fontColor{ juce::Colours::lightgrey.darker(0.3f).withAlpha(0.6f) };
    const juce::Colour highlightFontColor{ juce::Colours::lightgrey };
    const juce::Colour highlightColor{ juce::Colours::black.withAlpha(0.15f) };
    const juce::Colour backgroundColor{ juce::Colours::black.withAlpha(0.001f) };
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
    
    //void paintHorizontalConnectingLine(juce::Graphics& g, const juce::Line<float>& line) override;
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

    std::unique_ptr<juce::Drawable> fileIcon = juce::Drawable::createFromImageData(BinaryData::audio_file_white_24dp_svg, BinaryData::audio_file_white_24dp_svgSize);
    

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

    juce::Colour bgColor{ juce::Colours::darkgrey.darker(0.3f) };

    bool editable = true;
    bool editing = false;

    std::unique_ptr<juce::Drawable> folderIcon = juce::Drawable::createFromImageData(BinaryData::folder_white_24dp_svg, BinaryData::folder_white_24dp_svgSize);
    std::unique_ptr<juce::Drawable> folderOpenIcon = juce::Drawable::createFromImageData(BinaryData::folder_open_white_24dp_svg, BinaryData::folder_open_white_24dp_svgSize);


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

class RootHeaderItem :  public juce::TreeViewItem,
                        public InfoPanelComponent
{
public:
    RootHeaderItem(juce::ValueTree& sectionValueTree, FavoritesTreeView* rootItem);
    //SectionHeader();
    ~RootHeaderItem() override;

    void paintOpenCloseButton(juce::Graphics& g, const juce::Rectangle<float>& area, juce::Colour bgColor, bool isMouseOver) override;
    void paintItem(juce::Graphics& g, int width, int height) override;
    int getItemHeight() const override;

    bool mightContainSubItems() override;

    void itemClicked(const juce::MouseEvent& e) override;

    static void handleResult(int result, RootHeaderItem* comp);

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

class RecentFilesList : public juce::ListBoxModel, 
                        public juce::Component
{
public:
    RecentFilesList(SimpleAudioPreviewer* previewer);
    
    ~RecentFilesList() override;

    //void paint(juce::Graphics& g) override;
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

    std::unique_ptr<juce::Drawable> fileIcon = juce::Drawable::createFromImageData(BinaryData::audio_file_white_24dp_svg, BinaryData::audio_file_white_24dp_svgSize);

};

class FileChooser;

//This TreeView holds all of the TreeViewItems declared above. All items are children of the rootNode member variable. 
class FavoritesTreeView :   public juce::TreeView,
                            public juce::DragAndDropContainer
{
public:

    //FavoritesTreeView(FileChooser* fc, SimpleAudioPreviewer* prev);
    FavoritesTreeView(KrumFileBrowser& fb);
    ~FavoritesTreeView();

    /*void valueTreePropertyChanged(juce::ValueTree& treeChanged, const juce::Identifier& property) override;

    void valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& removedChild, int indexOfRemoval) override;*/

    void paint(juce::Graphics& g) override;

    juce::Rectangle<int> getTreeViewBounds();

    void refreshChildren();
    void deselectAllItems();

    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& details) override;
    void itemDropped(const juce::DragAndDropTarget::SourceDetails& details) override;

    //void pickNewFavorite();
    void createNewFavoriteFile(const juce::String& fullPathName); 
    void createNewFavoriteFolder(const juce::String& fullPathName);
    void addNewFavoriteSubFolder(juce::File& folder, int& numHiddenFiles, KrumTreeHeaderItem* parentNode, juce::ValueTree& parentTree);
    
    void reCreateFavoritesFromValueTree();
    void reCreateFavoriteFolder(juce::ValueTree& tree);
    void reCreateFavoriteFile(juce::ValueTree& fileValueTree);
    void reCreateFavoriteSubFolder(KrumTreeHeaderItem* parentNode, juce::ValueTree& parentTree);
    
    void sortFiles(FileBrowserSortingIds sortingId = FileBrowserSortingIds::folders_Id);

    void addDummyChild(juce::TreeViewItem* nodeToAddTo = nullptr);
    bool hasAudioFormat(juce::String fileExtension);

    //void updateValueTree(juce::String idString);
    void removeValueTreeItem(juce::String fullPathName, FileBrowserSectionIds browserSection);
    juce::ValueTree findTreeItem(juce::ValueTree parentTree, juce::String fullPathName);
    void updateOpenness();

    void clearFavorites();

    void removeItem(juce::String idString);
    void mouseDrag(const juce::MouseEvent& event) override;

    void dragOperationEnded(const juce::DragAndDropTarget::SourceDetails& details) override;

    void setItemEditing(juce::String idString, bool isEditing);
    bool areAnyItemsBeingEdited();


    juce::Array<juce::ValueTree> getSelectedValueTrees();

    juce::ValueTree& getFileBrowserValueTree();
    RootHeaderItem* getRootNode();

    KrumTreeHeaderItem* findSectionHeaderParent(juce::TreeViewItem* item, juce::String& sectionName);
    KrumTreeHeaderItem* makeHeaderItem(juce::TreeViewItem* item);
    KrumTreeItem* makeTreeItem(juce::TreeViewItem* item);
    KrumTreeItem* makeTreeItem(juce::Component* item);

    bool doesFolderExistInBrowser(juce::String fullPathName);

    //gives the browser an address to the KrumModuleContainer so we know which modules are being dragged over
    void assignModuleContainer(KrumModuleContainer* newContainer);

    juce::Colour getConnectedLineColor();
    juce::Colour getFontColor();

    void updateFavoritesFromTree(juce::ValueTree& newFavsTree);

private:

    /*class CustomFileChooser : public juce::FileChooser
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

    static void handleChosenFiles(const juce::FileChooser& fileChooser);*/
    
    //juce::ValueTree& fileBrowserValueTree;


    //std::unique_ptr<KrumTreeHeaderItem> rootItem;
    //int titleH = 20;

    bool showDropScreen = false;

    juce::ValueTree favoritesValueTree;
    std::unique_ptr<RootHeaderItem> rootItem;

    juce::Colour fontColor{ juce::Colours::darkgrey };
    juce::Colour bgColor{ juce::Colours::black };
    juce::Colour conLineColor{ juce::Colours::darkgrey };

    KrumFileBrowser& fileBrowser;

    //FileChooser* fileChooser;
    //SimpleAudioPreviewer* previewer;
    KrumModuleContainer* moduleContainer = nullptr;



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FavoritesTreeView)
};


//class LocationTabButton : public juce::TabBarButton 
//{
//public:
//    LocationTabButton(juce::TabbedButtonBar& barComp, const juce::String& tabName, int tabIndex);
//    ~LocationTabButton() override;
//
//    void paint(juce::Graphics& g) override;
//
//    juce::String name;
//    int index;
//    
//};
//
//class LocationTabBar : public juce::TabbedComponent
//{
//public:
//    LocationTabBar(FileChooser& fileChooser);
//    ~LocationTabBar() override; 
//
//
//    void currentTabChanged(int newCurrentTab, const juce::String& newCurrentTabName) override;
//    void popupMenuClickOnTab(int tabIndex, const juce::String& tabName) override;
//
//    void addLocation(juce::File location, juce::String name);
//    void addTabsFromLocationTree(juce::ValueTree& locationsTree);
//
//
//
//    static void handleTabRightClick(int result, LocationTabBar* tabBar, int tabIndex);
//    
//    juce::TabBarButton* createTabButton(const juce::String& tabName, int tabIndex) override;
//
//private:
//
//    int lastSelectedIndex = -1;
//
//    //store the user added locations
//    juce::ValueTree fileBrowserTree;
//    juce::Colour bgColor{ juce::Colours::transparentBlack };
//    FileChooser& fileChooser;
//
//    juce::ValueTree locationsValueTree;
//
//    static void handleRenameExit(int result, LocationTabBar* tabBar, juce::TextEditor* editor);
//
//};

//class FileChooserTreeView : public juce::FileTreeComponent
class FileChooser : public juce::Component,
                    public juce::FileBrowserListener,
                    public juce::ValueTree::Listener,
                    public juce::ComboBox::Listener
{
public:
    //TODO
    /*
    * - drag and drop to favorites section
    * 
    */

    enum RightClickMenuIds
    {
        addToFavorite = 1,
        addPlace,
        revealInOS,
    };
    
    FileChooser(KrumFileBrowser& fileBrower, SimpleAudioPreviewer& previewer);
    ~FileChooser() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void addLocationTabsFromTree(juce::ValueTree& locationsTree);

    void setDirectory(juce::File newDirectory);
    void goUp();
    
    void selectionChanged() override;
    void fileClicked(const juce::File& file, const juce::MouseEvent& e) override;
    void fileDoubleClicked(const juce::File& file) override;
    void browserRootChanged(const juce::File& newRoot) override;

    juce::File getSelectedFile();
    juce::Array<juce::File> getSelectedFiles();

    void valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& addedChild) override;

    void updatePathList();
    void addDefaultPaths();
    //void addLocationPaths();
    void handleChosenPath();

    void animateAddPlace();

    //const juce::String& getDragAndDropDescription() const override;

private:

    friend class FileChooserItem;

    juce::String findPathFromName(juce::String itemName);
    juce::File getFileFromChosenPath();

    //void getDefaultRoots(juce::StringArray& rootNames, juce::StringArray& rootPaths);
    void getDefaultPaths();
    //void getLocations(juce::StringArray& locationNames, juce::StringArray& locationPaths);
    
    juce::File defaultLocation{ juce::File::getSpecialLocation(juce::File::SpecialLocationType::userDesktopDirectory) };
    

    //LocationTabBar locationTabs;

    juce::TimeSliceThread fileChooserThread{ "FileChooserThread" };
    juce::DirectoryContentsList directoryList{ nullptr, fileChooserThread };
    juce::FileTreeComponent fileTree{ directoryList };
    
   /* class FileTree : public juce::FileTreeComponent 
    {
        FileTree(juce::DirectoryContentsList& listToShow);
        ~FileTree() override;


    };*/


    //juce::ComboBox currentPathBox;
    juce::StringArray pathBoxNames, pathBoxPaths;
    
    class CurrentPathBox;
    class PathBoxItem : public juce::PopupMenu::CustomComponent
    {
    public:
        PathBoxItem(CurrentPathBox& owner);
        ~PathBoxItem() override;

        void paint(juce::Graphics& g) override;
        void getIdealSize(int& idealWidth, int& idealHeight) override;

        //void mouseEnter(const juce::MouseEvent& e) override;
        //void mouseExit(const juce::MouseEvent& e) override;
        void mouseDown(const juce::MouseEvent& e) override;


        //void paint(juce::Graphics& g) override;
    private:

        static void handleRightClick(int result, PathBoxItem* item);


        CurrentPathBox& ownerComboBox;
    };

    class CurrentPathBox : public juce::ComboBox
    {
    public:
        CurrentPathBox(FileChooser& fc);
        ~CurrentPathBox() override;
        void showPopup() override;

        void paint(juce::Graphics& g) override;
        void addPathBoxItem(int itemId, std::unique_ptr<PathBoxItem> pathBoxItem, juce::String title);

    private:
        FileChooser& fileChooser;

    };

    CurrentPathBox currentPathBox{ *this };

    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;
    //juce::StringArray rootPaths;

    


    //juce::TextButton goUpButton;
    juce::DrawableButton goUpButton{"UpButton", juce::DrawableButton::ButtonStyle::ImageFitted};
    //std::unique_ptr<juce::Button> goUpButton;

    SimpleAudioPreviewer& previewer;
    KrumFileBrowser& fileBrowser;

    static void handleRightClick(int result, FileChooser* fileChooser);

    int lastSelectedId = -1;
    //juce::ChildProcess
    //juce::FileChooser

};

//===============================================================================

//===============================================================================

class PanelHeader : public InfoPanelComponent,
                    public juce::DragAndDropTarget,
                    public juce::Timer
{
public:

    enum PanelCompId
    {
        recent, 
        favorites, 
        fileChooser,
    };

    PanelHeader(juce::String headerTitle, juce::ConcertinaPanel& concertinaPanel, PanelCompId panelId);
    ~PanelHeader() override; 

    void resized() override;
    void paint(juce::Graphics& g) override;
    
    juce::Component* getPanelComponent();

    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& details) override;
    void itemDropped(const juce::DragAndDropTarget::SourceDetails& details) override;

    void animateAddFile();

private:

    juce::String getInfoPanelMessage();

    bool mouseOver = false;
    bool showCanDropFile = false;
    PanelCompId panelCompId;

    //juce::TextButton expandButton;

    //juce::ShapeButton arrowButton{ "arrowButton", juce::Colours::lightgrey, juce::Colours::lightgrey.darker(), juce::Colours::lightgrey.withAlpha(0.2f) };

    juce::ConcertinaPanel& panel;
    juce::String title;
    //bool closed = false;

    void timerCallback() override;


    bool drawAnimation = false;
    int currentDrawFrame = 0;
    int animationLengthSecs = 2;

    juce::Colour animationColor{ juce::Colours::green.withAlpha(0.8f) };

};


//===============================================================================

namespace DragStrings
{
    const juce::String recentsDragString{ "RecentsFileDrag-" };
    const juce::String favoritesDragString{ "FavoritesFileDrag-" };
    const juce::String fileChooserDragString{ "FileChooserFileDrag-" };
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

    juce::Array<juce::ValueTree> getSelectedFileTrees(BrowserSections section);

    void addFileToRecent(const juce::File file, juce::String name);
    void addFileToFavorites(juce::File file);
    bool doesPreviewerSupport(juce::String fileExtension);
    //SimpleAudioPreviewer* getAudioPreviewer();

    //void assignAudioPreviewer(SimpleAudioPreviewer* previewer);
    void assignModuleContainer(KrumModuleContainer* container);

    void rebuildBrowser(juce::ValueTree& newTree);
    void buildDemoKit();

    juce::ValueTree& getFileBrowserValueTree();

private:

    friend class FavoritesTreeView;


    juce::ValueTree& fileBrowserValueTree;

    SimpleAudioPreviewer audioPreviewer;

    juce::ConcertinaPanel concertinaPanel;

    PanelHeader recentHeader{ "Recent" , concertinaPanel, PanelHeader::PanelCompId::recent};
    RecentFilesList recentFilesList{ &audioPreviewer };
    //std::unique_ptr<RecentSection> recentSection;

    PanelHeader favoritesHeader{ "Favorites", concertinaPanel, PanelHeader::PanelCompId::favorites };
    FavoritesTreeView favoritesTreeView;
    //std::unique_ptr<FavoritesSection> favoritesSection;

    PanelHeader filechooserHeader{ "File Browser", concertinaPanel, PanelHeader::PanelCompId::fileChooser };
    FileChooser fileChooser{*this, audioPreviewer};
    //std::unique_ptr<FileChooserSection> fileChooserSection;

    InfoPanelDrawableButton addFavoriteButton {"Add Favorites", "Opens a browser to select Folders and/or Files to add to the Favorites section", "", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground};
    
    juce::Colour fontColor{ juce::Colours::lightgrey };
    
    int titleH = 20;
    int previewerH = 35;

    juce::File demoKit;

    


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KrumFileBrowser)
};
