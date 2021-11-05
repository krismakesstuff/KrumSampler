# KrumSampler

### A simple to use Drum Sampler Plug-in

![image](https://github.com/krismakesstuff/KrumSampler/blob/master/KrumSamplerScreenshot%2009292021.PNG)

## How to use KrumSampler
- Drag and drop files into the drop area at the top left to create a new module.
  - You then assign a midi key to your sample. This allows you to record and playback midi clips in your DAW to trigger your assigned samples.
- Each module has it's own volume, pan and clip gain. The Module's midi and color can be reassigned via the settings menu. 
  - The clip gain slider will appear when your mouse is over the thumbnail.
  - All sliders can be reset to default values when you double click on them.  
- You can additionally "hot swap" samples by dragging a sample and dropping it on the thumbnail of the desired module. No need to reassign midi or anything.
- The File Browser lets you add whole folders of samples for easy access to your favorite packs and let's you preview them. It also keeps track of your recently used samples.
  - The default behavior to audition a sample is to double-click the file. If the auto-play option is checked, then it will play everytime you click a file. The audio previewer has a seperate volume slider.
  - Additionaly, you can drag and drop files and folders from external apps (DAW, FileExplorer, Finder) that will save to the Favorites section.
- I envision only one instance of the plugin running per session and housing all your samples for easy mixing and file management. 
  - That being said, there's nothing stopping you from having multiple instances running (Although, I haven't tested this scenario much)
- This is a plug-in that comes in AU and VST3 formats, and will run on Windows and MacOS. (AAX coming soon!!)
- NOTE: THIS IS IN BETA!! (Probably should be alpha...) It runs fine in most cases, BUT I still haven't tested nearly as much as I would like. Please let me know if you run into any issues!
  - You can contact me at kris@krismakesmusic.com or submit an [issue](https://github.com/krismakesstuff/KrumSampler/issues)
  - I'm new to using github so if you have any suggestions on how to handle this better please don't be afraid to let me know 
- Lastly, I would like to thank you for taking the time to check this out. I'm self taught and stoked to have gotten this far, but there is so much I want to learn and include in this project. Any help or feedback is genuinely appreciated! Happy mixing, nerds!

## MAKE SURE TO CHECK THE ISSUES SECTION
- Visit [issues](https://github.com/krismakesstuff/KrumSampler/issues)
  - Shows the latest on current bugs and upcoming features! 

## How To Download
- Check out [releases](https://github.com/krismakesstuff/KrumSampler/releases) for the latest download.

## How To Build
- If you would like build the plug-in to work on it yourself, you will need the [JUCE](https://github.com/juce-framework/JUCE) framework installed and up to date. 
- Once you have that installed you can then open the krumsampler.jucer file in their projucer. Click the "save and open IDE" button icon towards the top of the projucer window. 
  - If you can't save and open, the Resources folder may be looking at the wrong path. I'm getting rid of binary data to avoid this situation, but I have provided the folder of samples, so just point the projucer to the KrumSampler/Resources folder in your project folder.
- Compile and build in your IDE. The plugins should copy themselves into the right locations for your DAW. You can also run it standalone to quickly see if it's working.
