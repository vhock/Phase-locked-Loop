Based on original project by Felix Tebbenjohanns, https://github.com/tefelixhu/redpitayapll  , Domink Windey and Markus Rademacher
I merely provided a C++-based interface instead of the original Python one.
# Getting started
### Windows:
From the "installer" folder, download the .zip file and extract it to any desired location. Execute the PLLInterface_Cpp.exe to launch the interface. 
### Linux:
No installer exists so far, it can however be deployed from the source using Qt functionality (https://doc.qt.io/qt-6/linux-deployment.html)

# Usage

### Connect
Specify IP-Address, username and password of the Red Pitaya and connect using the "Connect" button. The PLL parameters will be updated automatically if the corresponding bitfile is already running.
Connection is now active and will be monitored throughout the session.

### Launch bitfile
If the bitfile is not yet running, press the "Load Bitfile" button. This will copy the FPGA bitfile (Felix Tebbenjohanns) to the /tmp/ folder and execute it.
If the bitfile was already launched before, a second execution might crash the board and a reboot has to be done.
Parameters can be set as described in the README below.

### Log parameter changes
If the "Log parameter changes" checkbox is enbaled, parameter values will be read from the FPGA register after setting them and the value will be returned to the log. This slows down the setting of parameters, however.

### Send shell command
The "Send shell command" box allows for sending an arbitrary UNIX command to the Red Pitaya, for example sending "ls -all" will return the files in the active directory to the log.

***WARNING***
Parameters can be varied quickly using the spin boxes, however especially for request-heavy actions such as varying A or PHI this can result in "spamming" the Red Pitaya with an amount of SSH requests that leads to connection errors. If this occurs, a reconnect has to be done.

# Known bugs
## Phase-amplitude error
Phase values are converted to a radians value and converted to an integer value on the PLL. This leads to rounding errors, limiting the phase precision to ~2-3 deg.
##Low-amplitude phase insensitivity
At low amplitudes (a<5), phase changes might not be transferred to the PLL. This is due to the intertwinement of phase and amplitude on the PLL. Generally, phase changes should become more precise at high amplitudes.

# From the original README at https://github.com/tefelixhu/redpitayapll

# Overview
Two [PLLs](https://en.wikipedia.org/wiki/Phase-locked_loop) on a single Redpitaya. This code was developed in the Photonics Lab at ETH Zürich in order to [paramterically cool a levitated nanoparticle](https://photonics.ethz.ch/research/levitodynamics.html).
Initially, this code was hosted on the former [ETH git service](https://git.ee.ethz.ch). Unfortunately, the git history was lost after the move to github.
Tested frequencies go up to about 300 kHz, however the underlying clock frequency is 31.25 MHz making frequencies up to about 10 MHz possible in principle.
All code is written in VHDL and the top-level connection is done in Vivado 2017.2 using a block-diagram.

# Installation

Either use the precompiled bitfile in the corresponding folder `bitfile/pll_project.bit`, or generate it yourself. You should use Vivado 2017.2.
Upload it to the redpitaya's file system via scp or so; and launch with the command:
```
cat pll_project.bit > /dev/xdevcfg
```
A simple server program  written in python 3 can be run on a computer to manipulate the PLL easily (`server/pll.py`) and a more advanced graphical user interface is available if your run the server/gui.py application in python 3. Neccessary packages are `PyQt5`, `paramiko`, and `numpy`. It essentially makes use of the `monitor` command in order to read and write the internal memory. For register definitions see `doc/regs.pdf`. 

# Working principle
A schematic overview of the implementation can be found in `doc/figures/top_level_diagram.pdf` and `doc/figures/pll_schematic.pdf`.
Two independent PLLs are implemented and run on the two RF inputs IN1, IN2. Each PLL output or a combination can be set to the two RF outpus as explained below.
The PLL implementation consists of a phase detector which measures the phase between the input signal and the internally generated oscillator, a PI filter to lock the phase, a numerically controlled oscillator (NCO) which creates a complex oscillation signal, an optional clock divider (by 2; in order to output the second harmonic), and an output stage that creates a variable amplitude, variable gain, real oscillation. Each part is explained in more detail below:

## Phase detector
The input signal is multiplied by an internal complex oscillation and low-pass filtered. The resulting complex signal's phase is exactly the phase difference of the two oscillations. It is calculated using [CORDIC](https://en.wikipedia.org/wiki/CORDIC); for which a Xilinx IP block is used. Free parameters for the phase detector are `alpha` and `order` which both control the interanl low-pass filter, an [exponential smoothing filter](https://en.wikipedia.org/wiki/Exponential_smoothing), whose effective transfer function becomes: 
<pre>
                1  
LPF(f) = --------------  with fc = `alpha`/2pi * 122.07kHz and k = order between 1 and 8
         (1 + i f/fc)^k
</pre>
The output value of the phase detector is scaled by the constant _2^13/pi 1/rad = 2607.59/rad = 45.511/deg_.

## PI filter
A PI filter with limits on the output range given by `bw` in Hz, such that the integral does not diverge. The output y(t) for an input x(t) is:
<pre>
`kp` x(t) + `ki` * 122.07kHz * X(t) where X(t) is the time integral of x(t)
</pre>
Together with the NCO's gain (0.007276Hz) and the phase detector's gain (45.511/deg), the total proportioanl gain is:

_P = `kp` * 0.3311 Hz/deg_ 

and the integral gain 

_I = `ki` * 40422 Hz^2/deg_.


## NCO
The NCO is a 3rd party module by Simon Doherty, found on www.zipcores.com. It's output frequency is given by:

*fout = 31.25MHz/2^32 * `phase_inc` = 0.007276Hz * `phase_inc`*

Both quadrates (cos and sin) are calculated in order to create any phase and simplify phase detection.

## Clock divider (by 2)
A very simple clock divider can be added to the feedback signal in order to divide its frequency by 2. This results in an output frequency at the second harmonic of the input frequency.

## Output stage
The cos and sin created by the NCO can be added with different weights in order to generate any output amplitude and output phase.
      
# Settings

When opening the GUI, the first window allows to connect to the redpitaya. For this enter: *`hostname`,`username`,`password`*, where `hostname` is the redpitaya's URL, `username` and `password` are by default `root` on the redpitaya.

After successful connection to the device, a window opens which allows to modify all settings of the two  PLLs. There's a few global settings and PLL specific settings.


## Global settings 

- `output_1`: choose signal at RF output 1
- `output_2`: choose signal at RF output 2
  - possible output settings are:
    1. `PLL1`: output wave of the first PLL
    2. `PLL2`: output wave of the second PLL
    3. `PLL1+PLL2`: sum of the PLL waves
    4. `PLL1+IN2`: sum of the first PLL and the second RF input
    5. `IN1`: First RF input
    6. `IN2`: Second RF input
    7. `LI1_X`: X - quadrature of the Lock-in signal of the first PLL. If the PID is not enabeld, this is just a lock-in amplifier
    8. `LI1_Y`: Y - quadrature of the Lock-in signal of the first PLL.
- `ext_pins_p`: Integer between 0 and 255. External digital pins `PL_DIOx_P` on extension connector `E2` to control other hardware from redpitaya. See [schematic](https://dl.dropboxusercontent.com/s/jkdy0p05a2vfcba/Red_Pitaya_Schematics_v1.0.1.pdf).
- `ext_pins_n`: Integer between 0 and 255. `PL_DIOx_N` pins on extension connector `E2`


## PLL specific settings

- `2nd_harm`: When enabled, the output frequency is twice the input frequency.
- `pid_en`: when enabled, the PLL tries to lock to the input signal. Otherwise, the output frequency is constant given by `f0`
- `a`: Output amplitude between 0 and 127 (=1V).
- `phi`: Output phase in degree
- `kp`: P - value of the internal PI filter. *Important: put a negative value!* See PI subsection above
- `ki`: I - value of the internal PI filter. *Important: put a negative value!* See PI subsection above
- `f0`: Frequency setpoint in Hz
- `bw`: Allowed drift of the frequency in Hz
- `alpha`: Alpha parameter of the input filter in the phase detector (see Phase detector subsection above)
- `order`: betweeen 1 and 8. Order of the input filter (see Phase detector subsection above)
 

# Tested Values for parametric feedback cooling a levitated nanoparticle
- `2nd_harm` = 1
- `pid_en` = 1
- `a` = 30 (depends on the following amplifier)
- `phi` try out, depends on total system phase
- `kp` = -40 -> P = -13.2 Hz/deg
- `ki` = -0.128 -> I = -5174 Hz^2/deg
- `f0` = 45kHz, 130kHz, 150kHz
- `bw` = 5kHz
- `alpha` = 0.39 -> fc = 7.6kHz
- `order` = 4

# For developers
All settings made by the server program are refelected by `axi_gpio` blocks in Vivado whose addresses are distinct. Please find a memory overview in `doc/regs.pdf`. The current server program makes use of Linux' `monitor` command to read and write into memory.
To create the Vivado project, make sure you have the correct version (2017.2). Open Vivado and use its terminal to cd to the git base folder and call
```
source ./create_project.tcl
```
This should create a `tmp` folder with all project files.

If you make changes to the block design, export it as a .tcl script under File -> Export -> Export Block Design and replace the script/create_block_design.tcl file.



