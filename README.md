# SunriseAlarm
## Building a Full-Room Sunrise Alarm Clock from Recycled TVs

I've always been interested in the idea of waking up with the sunrise. In addition to supporting a more natural circadian rhythm, I find that I am less groggy upon awakening.

A common solution is to leave the window curtains open. However, I live an a suburban environment where many streetlights shine into my windows at night. This light makes it difficult to fall asleep.

My solution to this problem was to create a device that would physically block the windows at night and then itself brighten at sunrise. Years ago, I watched a YouTube video about turning recycled TVs into light fixtures and I realized that this would be a viable solution. I could physically block the windows with the TVs at night and then in the morning, have them illuminate synchronously via an Arduino circuit.

These are the steps I followed to build it:

### Building the TVs
1. I followed DIY Perks' wonderful YouTube video to prep the TVs
2. I wired 4 LED strips in parallel on the backlight display for increased luminosity
3. I made sure to leave enough wire exposed outside of the TV to power it via my circuit

### Building the Circuit
1. I adapted Benjamen Lim's great guide and Arduino code to suit my needs
2. After building the circuit, I adapted the code for my desired time zone and "sunrise" timing
3. Since it runs off of an Adafruit real-time clock chip, I can upload the code once and have it repeatedly run daily with no maintenance
4. I hooked up the TV power wires to the original LED output of the circuit (the TV is essentially a big LED) and my circuit was complete

Here is the V1 circuit and TV screens to be mounted (my Arduino code can be found here):

![IMG_5701](https://github.com/user-attachments/assets/f65349df-9175-4885-8229-08de1b4a034b)
![IMG_5619](https://github.com/user-attachments/assets/71f72883-22ee-4877-9104-e92b820e8a51)

Here is a video of it working in action prior to mounting!

https://github.com/user-attachments/assets/5c9d304f-1915-4935-afd1-f604496fee8c

After a little drilling, elbow grease, and twine, the screens are mounted. Here is a photo of them turned off and then fully turned on:

![IMG_5620](https://github.com/user-attachments/assets/c9c6e287-9ab1-4292-8272-bed5e211350e)
![IMG_5621](https://github.com/user-attachments/assets/f74f87ee-b9df-4f2c-a6a1-f08585bb2892)

I was blown away at how bright these screens were and how much they illuminated the room with believable daylight. And finally, here is a video of the linear version of the brightening working in action:

https://github.com/user-attachments/assets/21c917a7-7ee5-4174-97aa-447280f482ce

This was easily the largest project I have ever worked on as it encompassed both software and hardware. Look mom, my electrical engineering degree actually paid off!
