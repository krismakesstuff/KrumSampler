# KrumSampler

### A simple to use Drum Sampler Plug-in

![image](https://github.com/krismakesstuff/KrumSampler/blob/master/KrumSamplerScreenshot%2009292021.PNG)

## How to use KrumSampler
- Drag and drop files into the drop area at the top left to create a new module.
  - You then assign a midi key to your sample. This allows you to record and playback midi clips in your DAW to trigger your assigned samples.
- Each module has it's own volume, pan and clip gain. The Module's midi and color can be reassigned via the settings menu.  
- You can additionally "hot swap" samples by dragging a sample and dropping it on the thumbnail of the desired module. No need to reassign midi or anything.
- The File Browser lets you add whole folders of samples for easy access to your favorite packs, and let's you preview them. It also keeps track of your recently used samples.
- I envision there only being one instance of the plugin per session and housing all your samples for easy mixing and file management. 
  - That being said, there's nothing stopping you from having multiple instances running (Although, I haven't tested this scenario much)
- This is a plug-in that comes in AAX, AU, and VST3 formats, and will run on Windows and MacOS. 
- NOTE: THIS IS IN BETA!! (Probably should be alpha...) It runs fine in most cases, BUT I still haven't tested nearly as much as I would like. Please let me know if you run into any issues!
  - You can contact me at kris@krismakesmusic.com or submit an issue on this repo
  - I'm new to using github so if you have any suggestions on how to handle this better please don't be afraid to let me know 
- Lastly, I would like to thank you for taking the time to check this out. I'm self taught and stoked to have gotten this far, but there is so much I want to learn and include in this project. Any help or feedback is genuinely appreciated!! Happy mixing, nerds!

## Upcoming features:
- An awesome library of samples that come with the sampler. I have custom samples from my own use as a working producer, as well as friends that have shown interest in providing samples free of charge. It's just a matter of collecting them. 
- Aux output channels
  -  Adding Aux outputs will allow you to route the output of each individual module to a track in your DAW, where you can add any plugins you would like.
- Mute and Solo per module
- Module Display rearranging
  - This would let you drag and drop modules to move them around in the viewport
  - Could also see a button that would rearrange them according to your midi assignments
- File Browser to show Drives by default
- ModuleSettingsOverlay revamp
  - This is where you assign midi and color, just want it to look better

## Need To Fix/Rework:
- Automation is really not safe..
  - What I mean by that is if you have written automation in your DAW, say the volume slider on module 4. It will work fine, BUT if you delete a module in front of it, say module    2. It will reassign all the automation lanes after that deleted module. So your automation might be attached to the wrong module, or just might be deleted. This is annoying,      and something I would like to address quickly. If you have any experience in dealing with these types of issues then please help!
  - This one of the issues that Aux output channels will help solve by shifting automation to the DAW lane, but I would still like to be accomodating to either workflow.


## How To Build
- Detailed instructions to come within the next couple days.

## How To Download
- Detailed instructions to come within the next couple days.
