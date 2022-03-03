# KrumSampler

### a simple to use drum sampler

![image](https://github.com/krismakesstuff/KrumSampler/blob/dev/Screen%20Shot%202022-03-03%20at%2010.48.08%20AM.png)

## Newest Update: 1.4.0-beta [Release](https://github.com/krismakesstuff/KrumSampler/releases/tag/1.3.0-beta)
- Check out my update [video](https://www.youtube.com/watch?v=LSDDeG-BdvY) 
- You can also check out the 1.3.0 [video](https://www.youtube.com/watch?v=9vDSEBo0MJI) , it has a little more in-depth look at the features

## goals/motivations
- I'm a working music [producer-engineer](https://krismakesmusic.com/) in NYC and I wanted to have complete control over my drum programming workflow.
- Too often, drum sampling plug-ins are just way too complicated from a UI standpoint, so I wanted it to be simple and quick to use.
- You also have limited flexibility with a lot of existing ones, so being able to change the code myself is amazing! 
- 2 years ago, I didn't know how to code at all and wanted to learn as I'm very interested in how these tools are made. ALL music professionals rely on these technologies to make a living, yet most of them have no clue how their tools really work.
- This project is part learning, part bespoke workflow building, but also part "Hey, look I can code! Hire me and let's build a plug-in!".
- Lastly, I feel that this has to be open-source. 
  - Because of the open source community of [JUCE](https://github.com/juce-framework/JUCE) as well as [learn](https://www.learncpp.com/) C++, I have been able to learn and build this stuff for free and by myself. The internet is truly an amazing place.
  - This is my way of "giving back" to those communities and hopefully any new comers can get some use out of my code and those communities as well!
----------------------------------------------------------

## How To Download Plug-in
- Check out [releases](https://github.com/krismakesstuff/KrumSampler/releases) for the latest download. 
  - I'm not currently using installers, so you'll have to put the appropriate plug-in into it's appropriate folder. 

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

### - video walkthrough
- easiest way to see all the features is to watch [this](https://www.youtube.com/watch?v=LSDDeG-BdvY)
- most of this is covered in the video, this is here for reference

### - info panel
- This responds to your mouse's position and gives a brief decscription of what you are hovering over. 
  - You can disable this by clicking the white "i" icon in the top right of the plug-in.  
### - adding your own samples
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
- Pitch (1.4.0-beta)
  - Click and drag the Pitch button up or down to change the pitch in semi-tones. 
  - Double-click to reset to original pitch.
- Reverse (1.4.0-beta)
  - Toggles which direction the sample is played. 
- Mute (1.4.0-beta)
  - Mutes the sample from playing. This also decreases the amount of voices being used to render the audio.
- Volume, Pan & Clip Gain
  - All sliders can be reset to default values when you double click on them.
  - The clip gain slider will appear when your mouse is over the thumbnail. You can click and drag the slider or just scroll with the mouse over the thumbnail to adjust the clip gain.
- Time Handles (1.3.0-beta)
  - these handles choose which part of the sample to play when triggered. You can click and drag the handles underneath the waveform. 
- Aux Outputs (1.3.0-beta)
  - each module has it's own output. By default, everything goes out 1-2. But you have the ability to bus out your modules in groups or however you see fit.
  -  You have to set up channels in your DAW to recieve these outputs. If you don't know how to do that, I show it in my (1.3.0-beta) walkthrough video. It's easy, and let's you put your own plug-ins on seperate outputs of the sampler. 
  - You have to use them in consecutive order. Meaning, you can't have modules outputting to Output 5-6, without first having a module use Output 3-4.
  - For AU: you'll have to use the Multi-Output version when instantiating your plugin. In Logic, you then have to go to the mix window and create Aux channels. 
- Menu Button 
  - Clicking this will open up the Module Settings Overlay, here you can reassign notes and recolor your module. You can also see the midi channel if you need that.
- Title Box
  - You can rename the module by double clicking on the name at the top  
### - hot swap!
- You can additionally "hot swap" samples by dragging a sample from the file browser and dropping it on the thumbnail of the desired module.
  - This makes it SUPER easy to change samples out. This is safe to do while playing, so loop your beat in your DAW and swap out the samples as you see fit. 
  - It must be a file from the File Browser, for now the swapping doesn't support external drag and drops.
### - hide browser
- Once you are up and running you can make the window smaller by hiding the File Browser. This gives you some more screen real estate to program your midi and interact with your DAW. 
  - Click the arrow button on the left side of the plug-in to accomplish this. It will also disable the Info Panel.
### - notes
- Some limitations
  - The maximum file length a sample can be is 3 seconds. This is a restriction that keeps the voice count down. 
    - if you had 10 7-second long files and triggered all of them and 2 seconds later you trigger them again, you would then be rendering 20 voices. The more Voices the more CPU you use. I'm trying to keep CPU to a minumum. That being said I will be increasing the max file length to 5(?) seconds. 
  -  The maximum number of modules is 20. This theoritcally could be more, but things get tricky when we start to consider automation.
  -  The maximum number of voices that the sampler has is 14. This just means it can render 14 files at the same time (technically it's 15, but one voice is always preserved for the file previewing function). I don't think you'll need more than that, but now that I have this in a stable place I would consider adding more.  
    - In the future it would be cool to auto generate samples for different velocities and in that case I could see more voices being necessary.   
- I envision only one instance of the plugin running per session and housing all your samples for easy mixing and file management. 
  - That being said, there's nothing stopping you from having multiple instances running (Although, I haven't tested this scenario much)
- Currently only the VST3 and AU builds are available
  - This plug-in comes in AU and VST3 formats, and will run on Windows and MacOS. (AAX coming soon!!)
  
- THIS IS IN BETA!! It runs fine in most cases, BUT testing is still on-going. Please let me know if you run into any issues!
  - You can contact me at <kris@krismakesmusic.com> or submit an [issue](https://github.com/krismakesstuff/KrumSampler/issues)
  - I'm new to using github so if you have any suggestions on how to handle this better please don't be afraid to let me know 
- Lastly, I would like to thank you for taking the time to check this out. I'm self taught and stoked to have gotten this far, but there is so much more I want to learn and include in this project. 
  - ANY help or feedback is genuinely appreciated! Happy mixing nerds!

## support
- If you want to support this project, you can buy me a coffee [here!](https://www.buymeacoffee.com/kriscrawford)

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
