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

/*
* 
* The File Browser holds folders and files chosen by the user for quick access. The file paths will save with the plugin as well as any custom names that are given to them by the user.
* There is a separate display name which can be changed by the user, but does not rename any actual files.
* There are three sections, the "Recent", "Favorites" and "File Browser sections. The Recent section gets automatically updated when a file is dropped on a new module. It will only hold files.
* The Favorites section is user selected and can be chosen from the File Browser section, or by drag and drop from external apps.
* The File Browser section gives the user acces to their whole computer and can also save locations, called "Places".
 
* This class is a bit confusing... The KrumFileBrowser holds a KrumTreeView. The KrumTreeView holds TreeViewItems. There are two types of TreeViewItems, KrumTreeHeaderItem and KrumTreeItem. 
* The KrumTreeHeaderItem is for folders and KrumTreeItem is for files. Both also have custom component subclasses that give them some custom functionality.
* The state of the tree will be saved with each use and must be restored as well. 
* This browser also connnects to the AudioPreviewer to preview files. 
* 
* TODO:
* - give the favorites section it's own thread to handle big drag and drops.
* - Fix the DemoKit building
* 
*/

class KrumTreeItem;
class KrumTreeHeaderItem;
class KrumTreeView;
class FavoritesTreeView;
class KrumModuleContainer;
class FileChooser;
class KrumFileBrowser;
class KrumSampler;




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
    reveal_Id,
    assignToModule,
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

    std::unique_ptr<juce::Drawable> audioFileIcon = juce::Drawable::createFromImageData(BinaryData::audio_file_white_24dp_svg, BinaryData::audio_file_white_24dp_svgSize);
    

    //-----------------------------------------

    class EditableComp : public juce::Label
    {
    public:
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
//--------------------------------------------------------------------------------------------------------------

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
//--------------------------------------------------------------------------------------------------------------

class RecentFilesList : public juce::ListBoxModel, 
                        public juce::Component,
                        public juce::ValueTree::Listener
{
public:
    RecentFilesList( KrumFileBrowser& fileBrowser, SimpleAudioPreviewer* previewer);
    
    ~RecentFilesList() override;

    void valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& removedChild, int indexOfRemoval) override;

    void resized() override;
    int getNumRows() override;
    
    void listBoxItemClicked(int row, const juce::MouseEvent& e) override;
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent& e) override;

    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height, bool rowIsSelected) override;

    //returns true if added, false if file already exists in recents section
    bool addFile(juce::File fileToAdd, juce::String name);
    void updateFileListFromTree(juce::ValueTree& recentsTree);

    int getNumSelectedRows();
    juce::Array<juce::ValueTree> getSelectedValueTrees();

    juce::var getDragSourceDescription(const juce::SparseSet<int>& rowsToDescribe) override;

    void clearList();

private:

    juce::String getFileName(int rowNumber);
    juce::String getFilePath(int rowNumber);

    bool expanded = false;
    //pass the ListBoxModel to the ListBox in the updateFileListFromTree() function that gets called once the tree has been updated
    juce::ListBox listBox{ "Recent Files", nullptr };
    juce::ValueTree recentValueTree;

    SimpleAudioPreviewer* previewer = nullptr;
    KrumFileBrowser& fileBrowser;

    std::unique_ptr<juce::Drawable> audioFileIcon = juce::Drawable::createFromImageData(BinaryData::audio_file_white_24dp_svg, BinaryData::audio_file_white_24dp_svgSize);

};
//--------------------------------------------------------------------------------------------------------------

class FileChooser;

//This TreeView holds all of the TreeViewItems declared above. All items are children of the rootNode member variable. 
class FavoritesTreeView :   public juce::TreeView,
                            public juce::DragAndDropContainer,
                            public juce::ValueTree::Listener
{
public:

    FavoritesTreeView(KrumFileBrowser& fb);
    ~FavoritesTreeView();

    void valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& removedChild, int indexOfRemoval) override;

    void paint(juce::Graphics& g) override;

    juce::Rectangle<int> getTreeViewBounds();

    void refreshChildren();
    void deselectAllItems();

    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& details) override;
    void itemDropped(const juce::DragAndDropTarget::SourceDetails& details) override;

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

    //juce::ValueTree& getFileBrowserValueTree();
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

    void makeModulesFromSelectedFiles();

private:

    friend class KrumTreeItem;
    

    bool showDropScreen = false;

    juce::ValueTree favoritesValueTree;
    std::unique_ptr<RootHeaderItem> rootItem;

    //juce::Colour fontColor{ juce::Colours::darkgrey };
    juce::Colour bgColor{ juce::Colours::black };
    juce::Colour conLineColor{ juce::Colours::darkgrey };

    KrumFileBrowser& fileBrowser;
    KrumModuleContainer* moduleContainer = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FavoritesTreeView)
};
//--------------------------------------------------------------------------------------------------------------


//class FileChooserTreeView : public juce::FileTreeComponent
class FileChooser : public juce::Component,
                    public juce::FileBrowserListener,
                    public juce::ComboBox::Listener
{
public:

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

    void setDirectory(juce::File newDirectory);
    void goUp();

    void selectionChanged() override;
    void fileClicked(const juce::File& file, const juce::MouseEvent& e) override;
    void fileDoubleClicked(const juce::File& file) override;
    void browserRootChanged(const juce::File& newRoot) override;

    juce::File getSelectedFile();
    juce::Array<juce::File> getSelectedFiles();

    //void valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& addedChild) override;

    void updatePathBoxDropDown();
    void addPaths();
    void handleChosenPath();

    void animateAddPlace();
    void animateRemovePlace();

private:

    friend class FileChooserItem;

    juce::String findPathFromName(juce::String itemName);
    juce::File getFileFromChosenPath();

    void getPaths();
    
    juce::File defaultLocation{ juce::File::getSpecialLocation(juce::File::SpecialLocationType::userDesktopDirectory) };
    
    juce::Colour animationColor;

    juce::TimeSliceThread fileChooserThread{ "FileChooserThread" };
    juce::DirectoryContentsList directoryList{ nullptr, fileChooserThread };
    juce::FileTreeComponent fileTree{ directoryList };
    
    int userPlacesIndex = -1;

    juce::StringArray pathBoxNames, pathBoxPaths;
    
    class CurrentPathBox;
    class PathBoxItem : public juce::PopupMenu::CustomComponent
    {
    public:
        PathBoxItem(juce::String path, CurrentPathBox& owner);
        ~PathBoxItem() override;

        void paint(juce::Graphics& g) override;
        void getIdealSize(int& idealWidth, int& idealHeight) override;

        void mouseDown(const juce::MouseEvent& e) override;

        bool isSectionHeading = false;
        bool isSeparator = false;

        juce::File getFile() { return juce::File(path); }
        juce::String path;
    private:


        static void handleRightClick(int result, PathBoxItem* item);
        void removeThisPlace();

        CurrentPathBox& ownerComboBox;
    };

    class CurrentPathBox :  public InfoPanelComboBox,
                            public juce::ValueTree::Listener
                            
    {
    public:
        CurrentPathBox(FileChooser& fc);
        ~CurrentPathBox() override;

        void valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& childRemoved, int indexRemoved) override;

        void showPopup() override;
        void paint(juce::Graphics& g) override;
        void addPathBoxItem(int itemId, std::unique_ptr<PathBoxItem> pathBoxItem, juce::String title);

    private:
        friend class PathBoxItem;

        bool isUserPlace(PathBoxItem* itemToTest);
        FileChooser& fileChooser;

    };

    CurrentPathBox currentPathBox{ *this };

    void comboBoxChanged(juce::ComboBox* comboBoxThatHasChanged) override;

    InfoPanelDrawableButton goUpButton{ juce::DrawableButton::ButtonStyle::ImageFitted, "Up Button","Moves up one directory." };

    SimpleAudioPreviewer& previewer;
    KrumFileBrowser& fileBrowser;

    static void handleRightClick(int result, FileChooser* fileChooser);

    int lastSelectedId = -1;

};

//===============================================================================


class PanelHeader : public InfoPanelComponent,
                    public juce::DragAndDropTarget,
                    public juce::FileDragAndDropTarget,
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

    void mouseDown(const juce::MouseEvent& e) override;

    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& details) override;
    void itemDropped(const juce::DragAndDropTarget::SourceDetails& details) override;

    void animateAddFile();
    void animateRemoveFile();

    bool showingCanDropFile();
    void setShowCanDropFile(bool showShow);

private:

    juce::String getInfoPanelMessage();

    static void handleRightClick(int result, PanelHeader* header);

    bool mouseOver = false;
    bool showCanDropFile = false;
    PanelCompId panelCompId;

    juce::ConcertinaPanel& panel;
    juce::String title;

    void timerCallback() override;


    bool drawAnimation = false;
    int currentDrawFrame = 0;
    int animationLengthSecs = 2;

    juce::Colour animationColor;

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
    SimpleAudioPreviewer* getAudioPreviewer();

    void assignModuleContainer(KrumModuleContainer* container);

    void rebuildBrowser(juce::ValueTree& newTree);
    void buildDemoKit();

    juce::ValueTree getFileBrowserValueTree();
    juce::ValueTree& getStateValueTree();
    juce::AudioFormatManager& getFormatManager();
    PanelHeader* getPanelHeader(BrowserSections section);

    void addFileBrowserTreeListener(juce::ValueTree::Listener* listener);
    
private:

    juce::ValueTree& fileBrowserValueTree;
    juce::ValueTree& stateValueTree;
    juce::AudioFormatManager& formatManager;

    SimpleAudioPreviewer audioPreviewer;
    juce::ConcertinaPanel concertinaPanel;

    PanelHeader recentHeader{ "RECENT" , concertinaPanel, PanelHeader::PanelCompId::recent};
    RecentFilesList recentFilesList{ *this, &audioPreviewer };

    PanelHeader favoritesHeader{ "FAVORITES", concertinaPanel, PanelHeader::PanelCompId::favorites };
    FavoritesTreeView favoritesTreeView;

    PanelHeader filechooserHeader{ "FILE BROWSER", concertinaPanel, PanelHeader::PanelCompId::fileChooser };
    FileChooser fileChooser{*this, audioPreviewer};

    InfoPanelDrawableButton addFavoriteButton { juce::DrawableButton::ButtonStyle::ImageOnButtonBackground, "Add Favorites", "Opens a browser to select Folders and/or Files to add to the Favorites section"};
    
    bool init = true;

    juce::File demoKit;

    KrumModuleContainer* moduleContainer = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KrumFileBrowser)
};
