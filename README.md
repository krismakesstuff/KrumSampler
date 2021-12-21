# KrumSampler

### a simple to use drum sampler that let's you easily add your own samples

![image](https://github.com/krismakesstuff/KrumSampler/blob/master/Screenshot2021-12-21.png)

## How to use KrumSampler

### - info panel
- Before you get started, if you aren't sure on my explanations or just want to skip this. You can always use the Info Panel in the top left of the plugin. 
  - This responds to your mouse's position and tells you whats under your mouse. 
  - Make sure the "i" icon is white like pictured above. You can also disable the Info Panel if it's getting annoying by clicking on the same "i" icon. 
### - add your own samples
- The first thing you'll want to do is get your own samples in the plugin. If you don't have any you can use the DemoKit provided.
  - Drag and drop files or folders from a file explorer directly to the File Browser, or you can use the "+" button to select some files manually. 
  - These will be added to the "Favorites" section of the browser.
  - Once they are here you can preview them. Double-clicking the file will play it at the volume of the slider underneath the File Browser.
    - If Auto-Play is active, then the file will play everytime it's clicked.  
  - rename or delete files with a right-click.
    - this doesn't rename or delete the actual files, just how they are represented in this plug-in. 
### - create modules
- Once you have your favorite samples picked out, drag and drop one of the samples onto the empty module that says "Drop Samples Here".
  - The module will then show its "Module Settings Overlay". 
  - When only dropping 1 sample the "Midi Listen" button will automatically be enabled (Red). This means you simply play the midi note you want to assign to that sample and then hit confirm. 
  - You can also drop multiple samples and it will create modules for every sample, you then have to hit "Midi Listen" on each one to assign its midi. (I want this process to have more flexibility so I'm going to add some global preferences and some multi-module selecting and assigning)
### - module features
- Each module has it's own volume, pan and clip gain. The play button will play the sample, the settings cog will open the Module Settings Overlay. 
  - The clip gain slider will appear when your mouse is over the thumbnail. You can click and drag the slider or just scroll with the mouse over the thumbnail to adjust the clip gain.
  - All sliders can be reset to default values when you double click on them.
  - There is more to come for the modules. There will be aux outputs, solo, mute, pitch shift. Maybe more but for sure those, so stay tuned!
### - hot swap!
- You can additionally "hot swap" samples by dragging a sample from the file browser and dropping it on the thumbnail of the desired module.
  - This makes it SUPER easy to change samples out. This is safe to do while playing, so loop your beat in your DAW and swap out the samples as you see fit. 
  - You must drag a file from the File Browser, for now the swapping doesn't support external drag and drops.
### - hide browser
- Once you are up and running you can make the window smaller by hiding the File Browser. Gives you some more screen real estate to program your midi. 
  - Click the arrow button on the left side of the plug-in to accomplish this. It will also hide the Info Panel.
### - notes
- I envision only one instance of the plugin running per session and housing all your samples for easy mixing and file management. 
  - That being said, there's nothing stopping you from having multiple instances running (Although, I haven't tested this scenario much)
- Currently only the VST3 on Windows is running BUT  
  - This is a plug-in that comes in AU and VST3 formats, and will run on Windows and MacOS. (AAX coming soon!!)
  - I will update this ReadMe and the release as I update this which will be very soon.
  
- THIS IS IN BETA!! It runs fine in most cases, BUT testing is still on-going. Please let me know if you run into any issues!
  - You can contact me at <kris@krismakesmusic.com> or submit an [issue](https://github.com/krismakesstuff/KrumSampler/issues)
  - I'm new to using github so if you have any suggestions on how to handle this better please don't be afraid to let me know 
- Lastly, I would like to thank you for taking the time to check this out. I'm self taught and stoked to have gotten this far, but there is so much more I want to learn and include in this project. 
  - ANY help or feedback is genuinely appreciated! Happy mixing nerds!

## MAKE SURE TO CHECK THE ISSUES SECTION
- Visit [issues](https://github.com/krismakesstuff/KrumSampler/issues). Shows the latest on current bugs and upcoming features! 

## How To Download
- Check out [releases](https://github.com/krismakesstuff/KrumSampler/releases) for the latest download. 
  - I'm not currently using installers, so you'll have to put the appropraite plug-in into it's appropriate folder. (Once I update all the builds to the latest version I will make the installers)

## How To Build
- If you would like to build the plug-in to work on it yourself, you will need the [JUCE](https://github.com/juce-framework/JUCE) framework installed and up to date. 
- Once you have that installed you can then open the krumsampler.jucer file in the projucer.  
  - Make sure the Resources folder can be found by the projucer. You might have to remove the ones in the projucer and re-add the folder. 
  - Click the "save and open IDE" button icon towards the top of the projucer window. 
  - Please let me know if you run into any [issues!](https://github.com/krismakesstuff/KrumSampler/issues)
- Compile and build in your IDE.
