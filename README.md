**19.08.18**
## Welcome!

The purpose of this project will be to determine what the vibrational damping effects of different longboard wheels are, and get some mechanical and firmware design in on the side while I'm in Hamburg!

I began this project after moving to Germany and having some bumpy rides over cobblestones on my Landyachtz Dinghy. I thought it would be useful to have a device that could measure the intensity of vibrations felt by the board rider with different wheel set ups for the electric longboarding industry or people generally interested in a smooth, cruisy ride.

My goal is to have an easy-to-operate measurement device that will be attached to a board, where the user just has to turn it on and push a button to start measuring, and then have results output at the end of a standard measurement period. One-touch operation.

This will allow the user to ride the board with different sets of wheels over different terrains and have readings output each time for comparison. I hope to test wheels such as [Shark Wheels](https://sharkwheel.com/70mm-78a-smoke-black-sidewinder-longboard-wheels/), [ABEC-11](http://www.abec11.com/products/abec11/abec-11-107mm-electric-flywheels-74a-77a), and some [Hawgs](http://www.hawgswheels.com/fatty/) that I already own.

I currently have an ~~STM32 L475 Discovery board I plan to use the accelerometer on,~~ just kidding, I realized that's too complicated and that using an Arduino (Nano) is perfectly acceptable for this application, and program it in C (of course). I'll use a couple of simple switches/buttons for power and start, an old rechargeable mobile battery pack or lithium ion battery for power, and design a 3D printed casing with a mounting system. Stay tuned!

**Update 19.08.18:** Using SPI with a MAX7219 7-seg display for readouts.

**Update 8.09.18:** Selected MPU-6050 6-axis acclerometer and gyro for vibration reading. I could have selected a simpler sensor (such as [this one](https://www.adafruit.com/product/2384)) but I like the idea of developing my I2C and SPI interfacing skills. Using [LEDControl](https://github.com/wayoda/LedControl) and [MPU6050](https://github.com/tockn/MPU6050_tockn) Arduino libraries.

**Update 11.11.18:** Firmware flowchart updated (above).

**Update 12.11.18:** I will use the IMU's acclerometer raw value readings in the z-axis to determine levels of vibration.

**Update 13.11.18:** Flowchart for firmware: 
![Flowchart](media/vsFlow.svg ) 

## Schematic diagram:  

![Schematic](media/vsSchematic.png) 

**Update 29.11.18:** Current CAD:
![isoview](media/vibeSensorIso.png)


**Update 2.05.19:** After a long break focusing on school, I finished putting the sensor together. Had to modify several components including the battery pack, main body, and foot button. Pictures in /media. During initial testing, the platform was surprisingly strong enough to prevent bolts from tearing through. Display and added power LED was too dim for daylight on a cloudy day. Kicked the power switch by accident and broke the connection [somewhere TBD].  
**TODO:**  

- "Push to begin" msg annoyingly hard coded to display 3x before button push is registered. 
- 2nd msg "to begin" should be changed to say something different. 
- Progress bar not displaying all the way through. 
- Resultant vibration level should be displayed longer.  



**You're welcome to use any of my code you like, but please give credit and I'd love to hear about it!**
