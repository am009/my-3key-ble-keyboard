# 3key-bluetooth-mechanical-keyboard

Based on a tiny esp32 board TTGO T-Display. Draw a simple extension PCB with key switches for it. Connect with two row pins, and put battery in between.

基于TTGO T-Display这个小巧的开发板，通过排针排母连接一个带机械键盘轴的PCB，正好中间的位置放电池。本来打算放四个键，不太放得下就砍掉了一个键。

本项目的目的是学习使用Kicad，尝试画PCB，同时满足自己对小巧的蓝牙键盘的需求。没想到PCB双层板打样这么便宜，5块钱5张，那还用什么洞洞板，飞什么线，自己画拓展板去。这次拓展接线完全就是简单的IO口接轴接GND，甚至用esp32的内部上拉，走线甚至只用了单面。本来想尝试尝试esp_idf开发的，最后还是选择了Arduino IDE。。。比较简单。

## Reason
背anki用。三个键分别是Enter（good 绿色按钮）数字键1 (again 红色) 数字键2 (hard 灰色)
Make a small ble keyboard for Anki flashcards.
I mount my phone over my pillow, thinking about some way to control my phone. I tried using my Logitech bluethooth keyboard and feel great. When using Ankidroid to remember vocabularies, I found myself needing a tiny bluetooth keyboard.
the three key is set to Enter(for pressing the green(good) button), num 1(for red(again) button), and num 2(for gray(hard) button).

## Hardware

物料：

1. 电池最后选了802040（8mm厚，20mmx40mm） 800mah，勉强塞下。续航能力7-10天。是1.25mm 2P 正向接口。 15元两个包邮
2. 买了4个cherry 茶轴和4个灰色键帽。 带邮费26。
3. TTGO T-Display 47元（带排针），加两个12P排母。
4. 自备烙铁焊锡丝。

能够自定义三个键，暂不支持复杂的宏操作。

TTGO自带的按键一个设置为查看电量，一个设置为休眠和开机。

可以自定义蓝牙显示的名称，能够通过蓝牙显示电量信息。

打样打了5块，只用了一块，可以联系我。

唤醒方面原本是通过三个键同时按下唤醒的，最后还是改成了屏幕下方的按钮休眠。



## PCB

PCB设计使用Kicad. 用了下面的组件

[tmk - keyboard_parts](https://github.com/tmk/keyboard_parts.pretty) 键轴

[ttgo_esp32](https://github.com/BravoRoot/ttgo-tdisplay-kicad) for ttgo t-display component.

[MX_Alps_Hybrid](https://github.com/ai03-2725/MX_Alps_Hybrid) 键轴

[ttgo t-display 3d model](https://grabcad.com/library/lilygo-ttgo-t-display-esp32-1) 可以测量尺寸

画PCB的时候为了保险就留了上拉电阻的位置，最后确实不用接。



## Firmware

最后选择了Arduino IDE开发。安装：

1. [Arduino 蓝牙键盘库](https://github.com/T-vK/ESP32-BLE-Keyboard) 
2. [TTGO的屏幕支持库](https://github.com/Xinyuan-LilyGO/TTGO-T-Display) 



## TODO

1. 还是有外部上拉比较好，deep-sleep模式下负责内部上拉的RTC外设部分就可以休眠了。



## 图片展示

![](https://github.com/am009/my-3key-ble-keyboard/raw/main/photos/1.jpg)
![](https://github.com/am009/my-3key-ble-keyboard/raw/main/photos/4.jpg)
![](https://github.com/am009/my-3key-ble-keyboard/raw/main/photos/bat.jpg)
![](https://github.com/am009/my-3key-ble-keyboard/raw/main/photos/pcb1.jpg)
![](https://github.com/am009/my-3key-ble-keyboard/raw/main/photos/pcb2.jpg)
