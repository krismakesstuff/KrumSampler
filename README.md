# KrumSampler

A free open-source drum sampler plug-in. Use your own samples with a DAW-like channel strip layout. VST3 & AU formats.

![image](/Screen%20Shot%202022-07-03%20at%202.37.28%20PM.png)

## Newest Update: 1.5.0-beta (video coming soon)
- 1.4.0-Beta [video](https://www.youtube.com/watch?v=LSDDeG-BdvY) 
- 1.3.0-Beta [video](https://www.youtube.com/watch?v=9vDSEBo0MJI) , it has a little more in-depth look at the features

---
## How To Download Plug-in
- Check out [releases](https://github.com/krismakesstuff/KrumSampler/releases) for the latest download. 
  - I'm not currently using installers, so you'll have to put the appropriate plug-in into it's appropriate folder.
  - On MAC, you'll most likely have to allow the pulg-in access in the security settings.

## How To Build Project
- If you would like to build the plug-in and work on it yourself, you will need the [JUCE](https://github.com/juce-framework/JUCE) framework installed and up to date. 
- Once you have that installed you can then open the krumsampler.jucer file in the projucer.  
  - Make sure the Resources folder can be found by the projucer. You might have to remove the ones in the projucer and re-add the folder. 
  - Click the "save and open IDE" button icon towards the top of the projucer window. 
  - Please let me know if you run into any [issues!](https://github.com/krismakesstuff/KrumSampler/issues)
- Compile and build in your IDE.
- Visit [issues](https://github.com/krismakesstuff/KrumSampler/issues). Shows the latest on current bugs and upcoming features.

## License
- GNU GENERAL PUBLIC LICENSE Version 3

---
## Design Goals

- The main design goal is simplicity, with the intention of saving you time. Too often, drum sampling plug-ins have overly complicated UIs and you need to learn a whole new interface to make even the smallest adjustments.
- I want KrumSampler to be simple and quick to use, with the goal of staying out of the way so you can mix and produce music.
- I wanted the UI to have an "at-a-glance" design, where you can tell each samples' state by just opening the plugin. No menu diving and no need to be limited to manipulating only one sample at a time.
- I'm a working music [producer-engineer](https://krismakesmusic.com/) in NYC and I wanted to have complete control over my drum programming workflow. Most features are built specifically to my workflow, but I would love outside perspective on other workflows.

## Workflow
KrumSampler lets you see your samples as "DAW like" channel strips, letting you quickly view and adjust volume, pan, clip gain, midi note, bus ouput, length, color, pitch and reverse settings, without having to open any menus. Here is a quick worflow example.

1. Access and preview files on your system in the "File Browser" section or use an external file browser. 
2. Drop your samples on the "Drop Sample Area" area to make a new channel strip module. 
3. Assign a midi note to the newly made channel strip module(s).
4. Quickly audition multiple samples together using Multi-Control. 
5. Use your DAW's midi roll to program your beat. Too many drum samplers want you to learn their midi programming methods, which just isn't necessary. Keep all the midi for all your instruments in the DAW. 
6. Loop your beat and sound design. "Hot swap" samples as needed.
7. Use Auxillary Outputs to bus samples to DAW tracks. This allows you to use your own plugins on only certain samples. This is a common feature in many samplers, but is often under-utilizied in my opinion.
8. Colors! Yes colors look cool, but the colors are meant to be used as a tool to easily identify a group of channel strips. Color coding will allow you to easily navigate your channel strips when using a lot of samples.
9. Mix your song. Use the channel strips to easily mix samples in with the rest of your track.

###### TIP: The Info Panel at the bottom of the UI gives contextual information about what your mouse is hovering over.

----------------------------------------------------------


## What's new in 1.5.0-Beta update?
- **File Browser**. This panel has been completely rebuilt and now lets you browse files on your system and preview them before adding them to you session. You can see external hard drives and also save "Places" on your drive, to easily get back to a location.
- **Sample Drop Area**. The Sample Drop Area is now static and is always visible. Giving you the ability to hide the File Browser section all together and still be able to drop samples from an external application. If you drop multiple samples, the newly made channel strips will  automatically start listening for a midi message. This is to allow you to make layered samples instantly.
- **Drag Handles**. These were added to the bottom of the channel strips. Clicking this selects the module. You can also drag the module from left to right to rearrange it's current position. Additionally, if you hold the shift or control key modifier, you can select multiple modules and use Multi-Control. 

    *NOTE: You cannot rearrange multiple channel strips at the same time using multi-control. This feature will be added in the next update. For now, you have to move them one-by-one.*
- **Multi-Control**. You can now select multiple modules and treat them as an "instant group". When multiple modules are selected, holding the shift key will enable mutli-control, signified by a red outline around the modules. When a parameter is changed in one module, it will change in all the other selected modules. This also works for auditioning samples via the Play button, which lets you hear samples layered together without having to reassign their midi notes. This is my favorite feature and may be the biggest time saver. I'm not sure I've seen another sampler allow this kind of auditioning. (I could be wrong). 
- **Resizable Window**. You can now resize the width of the window. This current width gets saved with you session.
- **Midi Label**. Prior to 1.5.0-Beta, you had to enter into a separate window to change the midi assignment and it took multiple clicks. This has been refined to simply right-clicking the midi label to set the channel strip to "listen" for midi. Sending a midi message will stop the channel strip from listening further, or you can left click the midi label to stop listening as well.
- **Default color**. Now when a channel strip is created, it's default color is grey. In addition, the color palette has been simplified to make the channel strips easier to visually separate.

--------------------------------------------------------

## Notes
- There are some intentional limitations that limit the cpu usage to leave plenty of compute for other plugins. I feel as though the limitations should not to interfere with most use-cases, but that may not be the case and I'm open to changing these. 
  - The maximum file length a sample can be is 20 seconds. 
  -  The maximum number of channel strips modules is 20. 
  -  The maximum number of voices that the sampler has is 40. This just means it can render 40 files at the same time (technically it's 41, but one voice is always preserved for the file previewing function). 
- I envision only one instance of the plugin running per session and housing all your samples for easy mixing and file management. 
  - That being said, there's nothing stopping you from having multiple instances running (Although, I haven't tested this scenario much)
- Currently only the VST3 and AU builds are available
  - This plug-in comes in AU and VST3 formats, and will run on Windows and MacOS. (AAX coming soon!!)
  
- THIS IS IN BETA!! It runs fine in most cases, BUT testing is still on-going. Please let me know if you run into any issues!
  - You can contact me at <kris@krismakesmusic.com> or submit an [issue](https://github.com/krismakesstuff/KrumSampler/issues)
- ANY help or feedback is genuinely appreciated! Happy mixing nerds!

## support
- If you want to support this project, you can buy me a coffee [here!](https://www.buymeacoffee.com/kriscrawford)
