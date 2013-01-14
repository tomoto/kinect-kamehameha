kinect-kamehameha V1.0b
Author: Tomoto S. Washio
Release Date: 1/13/2013
Project URL: http://code.google.com/p/kinect-kamehameha/

0. What's this?

This is a Kinect Hack that allows you to shoot an energy wave that
looks like Kamehameha for Dragon Ball.

1. Prerequisitive

(1) PrimeSense's 3D sensor such as Kinect (Microsoft), Xtion Live Pro
    (ASUS), PrimeSensor (PrimceSense) is required.
(2) Supported OS's depend on the sensor type.
(2.1) Kinect
    - Windows 7 or later, i.e. Windows OK that supports Kinect SDK
      (tested on Win7 Pro 64bit)
    - Unofficially, may work in Windows XP (tested on WinXP Pro SP3 32bit)
    - Unofficially, may work on Linux (not confirmed)
(2.2) Other PrimeSense sensors
    - Windows XP or later (tested on on WinXP Pro SP3 32bit and Win7 Pro 64bit)
    - Linux (tested on Ubuntu 12.10)
(3) Your computer should have sufficiently fast CPU and GPU.
    - The minimum requirement is unknown
    - The GPU must support programmable shaders and VAOs at least.
    - Core i7 2600 + GeForce GTX 285 is sufficient at least.
    See http://code.google.com/p/kinect-kamehameha/wiki/WorkingConfigurations
    for the list of the hardware/software configurations this program is
    reported to work.

2. Installation

2.1 Kinect SDK edition

- For Windows 7 or later + Kinect
  Easiest option for Kinect users who have Kinect SDK supported environment.

- Download and install Kinect SDK.
  http://www.kinectforwindows.org

  Then unpack this software anywhere you like.

2.2 OpenNI edition

- For Windows + general PrimeSense sensors. Kinect is also supported [*1].
  Installation is a bit cumbersome, and planned to be replaced with
  OpenNI2 edition explained later.

- Download OpenNI and other related software and install them.
  The process is complicated so check the information at the URL below.
  http://code.google.com/p/kinect-ultra/wiki/OpenNI_Downloads

  Then unpack this software anywhere you like.

2.3 OpenNI2(Beta) edition

- For Windows + general PrimeSense sensors. Kinect is also supported [*2].
  Installation is much easier than the previous OpenNI edition.
  The archive is large and it is still beta, though.

- Just unpack this software anywhere you like.
  To use Kinect, install Kinect SDK as 2.1.

2.4 Linux edition

- For Linux + general PrimeSense sensors. Kinect is not officially supported [*2].
  Implementation is based on OpenNI2(Beta).
  I don't know if it works except on Ubuntu 12.10.
  If you are interested, please feel free to explore by yourself.

- Unpack this software anywhere you like, and sudo install.sh.

[*1] Although it is unofficial, you can use Kinect on Windows XP w/o Kinect SDK.
[*2] It seems there is an unofficial way to use Kinect on OS's w/o Kinect SDK,
     but not confirmed. Hint: FreenectDriver.

3. How to Use

(1) Just run kinect-kamehameha.exe.
    - Set the working directory to the same location as the executable.
    - Hit [ESC] key to end.
    - Hit [Enter] key to make it full screen.

(2) See the demo at:
    - English
      http://www.youtube.com/watch?v=lajP7HO7ZyE
    - Japanese
      http://www.youtube.com/watch?v=r0l18SWKd88

(3) If you have any issues...
    - See FAQ at http://code.google.com/p/kinect-kamehameha/wiki/FAQ_en
    - Report issues at http://code.google.com/p/kinect-kamehameha/issues

4. License

See LICENSE.TXT.

5. Build

(1) Windows: Download the source code from the project page, and open
    build2010/kinect-ultra.sln with Visual C++ 2010. To use debugger,
    set the working directory to ".." in the project's preference.

(2) Linux: Download the source code from the project page, and follow
    the instruction in buildLinux/BUILD_linux.TXT.

Enjoy!
Tomoto
