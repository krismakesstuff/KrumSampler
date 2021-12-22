# KrumSampler

### a simple to use drum sampler

![image](https://github.com/krismakesstuff/KrumSampler/blob/master/Screenshot2021-12-21.png)

## goals/motivations
- I'm a working music [producer-engineer](https://krismakesmusic.com/) in NYC and I wanted to have complete control over my drum programming workflow.
- Too often, drum sampling plug-ins are just way too complicated from a UI standpoint, so I wanted it to be simple and quick to use.
- You also have limited flexibility with a lot of existing ones, so being able to change the code myself will be amazing! 
- 2 years ago, I didn't know how to code at all and wanted to learn as I'm very interested in how these tools are made. ALL music professionals rely on these technologies to make a living, yet most of them have no clue how their tools really work. (I've been working on this for the last 9 months or so)
- This project is part learning, part bespoke workflow building, but also part "hey, look I can code! So hire me.".
- Lastly, I feel that this has to be open-source. 
  - Because of the open source community of [JUCE](https://github.com/juce-framework/JUCE) as well as [learn](https://www.learncpp.com/) C++, I have been able to learn and build this stuff for free and by myself. The internet is truly an amazing place. Now that I have harvested knowledge from it, I must give some back.
  - This is my way of "giving back" to those communities and hopefully any new comers can get some use out of my code and those communities as well!
----------------------------------------------------------

## How To Download Plug-in
- Check out [releases](https://github.com/krismakesstuff/KrumSampler/releases) for the latest download. 
  - I'm not currently using installers, so you'll have to put the appropriate plug-in into it's appropriate folder. (Once I update all the builds to the latest version I will make the installers)

## How To Build Project
- If you would like to build the plug-in and work on it yourself, you will need the [JUCE](https://github.com/juce-framework/JUCE) framework installed and up to date. 
- Once you have that installed you can then open the krumsampler.jucer file in the projucer.  
  - Make sure the Resources folder can be found by the projucer. You might have to remove the ones in the projucer and re-add the folder. 
  - Click the "save and open IDE" button icon towards the top of the projucer window. 
  - Please let me know if you run into any [issues!](https://github.com/krismakesstuff/KrumSampler/issues)
- Compile and build in your IDE.

## MAKE SURE TO CHECK THE ISSUES SECTION
- Visit [issues](https://github.com/krismakesstuff/KrumSampler/issues). Shows the latest on current bugs and upcoming features! 


--------------------------------------------------------

## How to use KrumSampler

### - info panel
- Before you get started, if you just want to skip this and download, go ahead. You can always use the Info Panel in the top left of the plugin. This will be here for reference.
  - This responds to your mouse's position and tells you whats under your mouse. 
  - Make sure the "i" icon is active (white), like pictured above. You can also disable the Info Panel by clicking on the same "i" icon. 
### - add your own samples
- The first thing you'll want to do is get your own samples in the plugin.
  - Drag and drop files or folders from Finder/File Explorer directly to the File Browser, or you can use the "+" button to select some files manually. 
  - These will be added to the "Favorites" section of the browser.
  - Once files are in the File Browser, you can play them. Double-clicking the file will play it at the volume set by the slider underneath the File Browser.
    - If Auto-Play is active, then the file will play everytime it's clicked.  
  - rename or delete files with a right-click.
    - this doesn't rename or delete the actual files, just how they are represented in this plug-in. 
### - create modules
- Drag and drop one of the samples onto the empty module that says "Drop Samples Here".
  - The module will then show its "Module Settings Overlay". 
  - When only dropping 1 sample the "Midi Listen" button will automatically be enabled (red). This means you simply play the midi note you want to assign to that sample and then hit confirm. 
  - You can also drop multiple samples and it will create modules for every sample, you then have to hit "Midi Listen" on each one to assign its midi. (In the future, I want this process to have more flexibility so I'm going to add some global preferences and some multi-module selecting and assigning)
### - module features
- Each module has it's own volume, pan and clip gain. The play button will play the sample, the settings cog will open the Module Settings Overlay. 
  - The clip gain slider will appear when your mouse is over the thumbnail. You can click and drag the slider or just scroll with the mouse over the thumbnail to adjust the clip gain.
  - All sliders can be reset to default values when you double click on them.
  - There is more to come for the modules. There will be aux outputs, solo, mute, pitch shift. Maybe more, but for sure those, so stay tuned!
### - hot swap!
- You can additionally "hot swap" samples by dragging a sample from the file browser and dropping it on the thumbnail of the desired module.
  - This makes it SUPER easy to change samples out. This is safe to do while playing, so loop your beat in your DAW and swap out the samples as you see fit. 
  - It must be a file from the File Browser, for now the swapping doesn't support external drag and drops.
### - hide browser
- Once you are up and running you can make the window smaller by hiding the File Browser. This gives you some more screen real estate to program your midi and interact with your DAW. 
  - Click the arrow button on the left side of the plug-in to accomplish this. It will also hide the Info Panel.
### - notes
- Some limitations
  - The maximum file length a sample can be is 3 seconds. This is a restriction that keeps the voice count down. 
    - if you had 10 5-second long files and triggered all of them and 2 seconds later you trigger them again, you would then be rendering 20 voices. The more Voices the more CPU you use. I'm trying to keep CPU to a minumum. 3 seconds is plenty for most drum samples.
  -  The maximum number of modules is 20. This theoritcally could be more, but things get tricky when we start to consider automation.
  -  The maximum number of voices that the sampler has is 14. This just means it can render 14 files at the same time (technically it's 15, but one voice is always preserved for the file previewing function). I don't think you'll need more than that, but now that I have this in a stable place I would consider adding more.  
    - In the future it would be cool to auto generate samples for different velocities and in that case I could see more voices being necessary.   
- I envision only one instance of the plugin running per session and housing all your samples for easy mixing and file management. 
  - That being said, there's nothing stopping you from having multiple instances running (Although, I haven't tested this scenario much)
- Currently only the VST3 and AU builds are available
  - This plug-in comes in AU and VST3 formats, and will run on Windows and MacOS. (AAX coming soon!!)
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
  - I'm not currently using installers, so you'll have to put the appropriate plug-in into it's appropriate folder. (Once I update all the builds to the latest version I will make the installers)

## How To Build
- If you would like to build the plug-in and work on it yourself, you will need the [JUCE](https://github.com/juce-framework/JUCE) framework installed and up to date. 
- Once you have that installed you can then open the krumsampler.jucer file in the projucer.  
  - Make sure the Resources folder can be found by the projucer. You might have to remove the ones in the projucer and re-add the folder. 
  - Click the "save and open IDE" button icon towards the top of the projucer window. 
  - Please let me know if you run into any [issues!](https://github.com/krismakesstuff/KrumSampler/issues)
- Compile and build in your IDE.
