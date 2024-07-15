# The Cannaster
## super convenient way to get nugs to the perfect moisture content for canning
The Cannaster simply uses a pressure sensor (bme280) to control a small vacuum pump, keeeping it at a certain pressure for a certain amount of time. I was able to use the OLED buffer by sacraficing a precise humidity sensor instead of relying on the readings from the bme280. However, the humidity sensors will be calibrated so this might not be a huge issue, time will tell. 
### the actual program for the Cannaster is to just turn on the pump if the humidity level rises above a certain level, and you can also do one where it'll turn on for a certain time then off again. 
