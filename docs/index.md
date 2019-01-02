# Carbot

The WCRS carbot is a project to get people started with a mobile platform, using minimal materials.  The goal is to spend as little money as possible, while getting something that actually works.  Even if not very well.  Use if for a proof of concept, and to learn about the different parts that interact to make a mobile bot.

The base chassis uses 2 TT 130 motors with wheels, mounting brackets, ping pong ball, 6 cell battery holder, a piece of foam board, and some popsicle sticks, with a bit of extra M3 hardware and hot glue.  That is then controlled by an Arduino Uno motor shield.  A power switch is added to the prototyping area of the motor shield.

Expanding that to a remote controlled car needs another Arduino UNO, 2 nRF24L01 radio modules, a joystick, a 9V battery clip, and more foam board.  The joystick and radio modules are mounted on protoboard shields.

That hardware, combined with sketches in the [carbot](https://github.com/WCRSyyc/carbot) repository, provide the functionaly to demonstrate and explore many robotics features.  Once the concepts are learned, the same system can be reimplented ona a more robust platform, using a varity of differnt materials and tools.  Several different kits are available to build a much more solid 2 wheeled chassis.  Laser cut acrylic, wood, and 3D printing are other ways to go, depending on available resources, skills, and interest.

The basic chassis, or one of the improved versions, can be added to, to provide additional and different functionality.  The nRF24L01 radio shields can be replaced by Bluetooth, wi-fi, IR, or other communications channels.  The code for the chassis can be enhanced to return information to the hand held remote.  That can have simple LEDs added to show status information, or an LCD screen can be used to display it.  Sensors can be added to the chassis to allow it to be more autonomous.  If using Bluetooth or wi-fi, it is not diffcult to replace the custom joystick unit with a smart phone.

There are many directions to go from the initial carbot.  This is intended to be a starting point, an inexpensive prototype, where the cost of possible (and likely) mistakes is not going to be a roadblock to getting started.  Once the concepts and limitations are learned by working with this, the next, more permanent, project can be started.
