# portapack-havoc-modified

**New H3 Mini announced. Features 3.2" screen and original thin encoder**

<img width="500" alt="H3SE" src="https://user-images.githubusercontent.com/17997195/227824450-1465d64b-4ca7-4d9d-bf06-1714b9f1bec1.png">

**Distributors:**

HamRadioShop:

https://www.aliexpress.com/item/3256804550974838.html


https://www.ebay.com/itm/XXX/115523402262

Swiftflying:

https://www.aliexpress.com/item/3256804545943517.html

<img width="500" alt="Screenshot 2022-05-19 103253" src="https://user-images.githubusercontent.com/17997195/169192013-493f29fb-e11a-48dd-9c3b-5a8f83d17eff.png">

This repository mainly focuses on hardcore DSP stuff. UI remains original HAVOC style.

GPS simulator / Analog TV demodulator(PAL) / Bluetooth receiver / NRF24L01 demod / improved Pocsag tx and rx (P/N phase).

**Features above are developed and published from this repository.**
**Later taken by Mayhem, and modified to some other UI/font.**


# Does H3/H3 SE/H3 Mini compatible with Mayhem?

**Yes**


<img width="434" alt="Screenshot 2022-05-18 173305" src="https://user-images.githubusercontent.com/17997195/169008646-e3950dbc-f9f5-4abf-bf33-16004d7b0dcc.png">

# Does latest features compatible with H1 or H2?

**No** for H2 (close source, **Do not buy or support**)

**Plausible** for H1 with my add-on board



# Latest/Best firmware for H3/H3 SE/H3 Mini?

**Most useful features of portapack are chosen and included in my version of firmware.** 

**However, features below are only available in my firmware on H3/H3 SE.**



* Train Detector (NEW)
* SSTV(robot8, martin m1&m2, scottie s1&s2, pd-120) receiving
* NOAA receiving 
* Improved Analog TV receiver
* Improved Signal Generator and Soundboard (added AM support)
* Built in microphone
* Built in GPS receiver for location and time sync
* Built in battery and level indicator
* Built in barometer
* Built in compass

* 
![image](https://user-images.githubusercontent.com/17997195/165471964-f718298b-6c43-4e9f-9c1b-8aed3bd489e5.PNG)
![image](https://user-images.githubusercontent.com/17997195/163305857-1dcc1f41-17e2-4243-978b-0eece7e4295a.PNG)
![image](https://user-images.githubusercontent.com/17997195/163305865-d89d2a28-f1ce-4a96-b9f9-e79d228027ef.PNG)
![image](https://user-images.githubusercontent.com/17997195/167530145-e39a9bff-d586-4b85-8f72-e17fb8fab285.PNG)
![image](https://user-images.githubusercontent.com/17997195/163305881-9e3298d3-0408-45fc-9793-5a285ac2276f.PNG)
![image](https://user-images.githubusercontent.com/17997195/167978159-77b60f81-dd0a-4f12-9623-d687d8b48c24.PNG)


# Train Detector (with multi-language support):
![image](https://user-images.githubusercontent.com/17997195/230917324-a0f4ce2d-7458-4bc8-852b-fb4989b673c0.PNG)
![image](https://user-images.githubusercontent.com/17997195/230917329-c4b7a896-5200-46a7-8769-9e793b703615.PNG)

# Drone FPV camera demod:
![image](https://user-images.githubusercontent.com/17997195/229970665-6354057d-4c62-4d64-bff8-f586be1e24a7.PNG)
![image](https://user-images.githubusercontent.com/17997195/229970683-c478a067-52ca-4f2d-91f6-64b44f68ce90.PNG)


# Analog TV Testing Method:

Signal source can be commercial TV broadcasting signal 

or **HackTV** with commands as below:

ffmpeg -i input.mp4 -vf scale=832:576 output.mp4

hacktv -f 434000000 -m i -g 47 output.mp4

![image](https://user-images.githubusercontent.com/17997195/229970771-e1455f05-da34-4434-93dc-fa5ddc78f3fe.png)
![image](https://user-images.githubusercontent.com/17997195/229970789-78b44001-698d-452d-91e5-999efc583417.png)



