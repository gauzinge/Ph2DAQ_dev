CMS Ph2 ACF (Acquisition & Control Framework) 
======================================

__Contains:__

- A middleware API layer, implemented in C++, which wraps the firmware calls and handshakes into abstracted functions

- A C++ object-based library describing the system components (CBCs,
Hybrids, Boards) and their properties(values, status)

- several utilities (like visitors to execute certain tasks for each item in the hierarchical Item description)

- a tools/ directory with several utilities (currently: calibration, hybrid testing, common-mode analysis)

- some applications: datatest, interfacetest, hybridtest, system, calibrate

Different versions
---------------------------------------

On this Repo, you can find different version of the software :
- a hopefully working and stable version on the master branch
- An in-progress version in the Dev branch
- A development version wiht a GUI
<br>

Changelog:
------------


- 09/07/14 : Added threading for stack writing registers
- 12/08/14 : Working agnostic version of the new structure on Master
- 15/08/14 : System Controller Class working
- 19/08/14 : Project wrapped, called ACF for Acquisition & Control Framework
- 09/10/14 : added Visitor class and the corresponding accept methods to the HWDescription objects
- 15/10/14 : re-wrote the GlibFWInterface::ReadData() method, completeley re-wrote the parsing of the raw buffer and the offsets, modified the Data and Event classes to be more lightweight and less complex
- 17/10/14 : renamed the project to Ph2_ACF & re-structured the folder architecture and added command line parsing to the executables (v1-00)
- 17/12/14 : major update (v1-01) including:
    - performance improvements (use of C++11 features)
    - general bugfixes
    - a new, faster calibration routine
    - a common-mode-noise tester tool
    - compatability with DIO5 FW for external triggering / clocking
    - updated address table for 2 & 8 CBC setups
    - new FW files for DIO5 FW for 2 & 8 CBC setups
    - a macro directory with a macro to visualize calibration results
    - commissioning mode is now off by default 

<br>

Setup
-----------------

Firmware for the GLIB can be found in /firmware. Since the "old" FMC flavour is deprecated, only new FMCs (both connectors on the same side) are supported.

NOTE: If you are doing the install for the first time on the latest [VM v1.1.0] (http://sbgcmstrackerupgrade.in2p3.fr/) then follow the preliminary setup, otherwise you can skip this.

1. Remove the current gcc and old boost libraries:

        sudo yum remove devtoolset-1.1-gcc-debuginfo
        sudo yum remove boost

2. Install the latest gcc compiler:

        sudo yum install devtoolset-2
        sudo ln -s /opt/rh/devtoolset-2/root/usr/bin/* /usr/local/bin/
        hash -r

   This should give you gcc 4.8.1:

        gcc --version

3. Finally, update uHAL to version 2.3:

        sudo yum groupremove uhal
        wget http://svnweb.cern.ch/trac/cactus/export/28265/tags/ipbus_sw/uhal_2_3_0/scripts/release/cactus.slc5.x86_64.repo 

    (You may need the --no-check-certificate)
	
        sudo cp cactus.slc5.x86_64.repo /etc/yum.repos.d/cactus.repo
        sudo yum clean all
        sudo yum groupinstall uhal

4. Re-compile ROOT using the new gcc 4.8: [Instructions](http://root.cern.ch/drupal/content/installing-root-source) - make sure to use "fixed location installation"


Note: You may also need to set the environment variables:

    export LD_LIBRARY_PATH=/opt/cactus/lib:$LD_LIBRARY_PATH
    export PATH=/opt/cactus/bin:$PATH


The Ph2_ACF Software : 
----------------------------------

Follow these instructions to install and compile the libraries:
(provided you installed the latest version of gcc, µHal, boost as mentioned above)

1. Clone the GitHub repo.

2. Do a make in the root of the repo (make sure you have all µHal, root, boost... libraries on your computer).

3. Launch 
        
        systemtest --help
     
     command if you want to test the parsing of the HWDescription.xml file

4. Launch

        datatest --help
        
    command if you want to test if you can correctly read data.

6. Launch

        calibrate --help

    to calibrate a hybrid,

        hybridtest --help

    to test a hybird's I2C registers and input channel connectivity

        cmtest --help

    to run the CM noise study


7. an example of how to use visitors can be found in src/interfacetest.cc or in the HybridTester class


__What can you do with the software ?__

At the moment the package provides the following features:

- Configure the Glib & Cbcs
- Manipulate the registers in the Glib
- Manipulate the registers in the Cbcs
- Read Data
- Calibrate Hybrids
- Validate Hybrids
- Perform CM noise tests
- user external trigger and clock signals for your tests
- any other routine you want to implement yourself ... 


__Nota Bene:__
When you write a register in the Glib or the Cbc, the corresponding map of the HWDescription object in memory is also updated, 
so that you always have an exact replica of the HW Status in the memory.

Register values are:
- 8-bit unsigend integers for the CBCs that should be edited in hex notation, i.e. '0xFF'
- 32-bit unsigned integers for the GLIB: decimal values

For debugging purpose, you can activate DEV_FLAG in the sources or in the Makefile and also activate the uHal log in RegManager.cc.


__External Clock and Trigger:__

In order to use external Clock and Trigger functionality, a DIO5 mezzanine is required. It is available from the [CERN OHR](http://www.ohwr.org/projects/fmc-dio-5chttla) and sold by several commercial vendors.
For instructions on how to use it, see this [file](https://github.com/gauzinge/Ph2_ACF/blob/Dev/doc/TK_DAQ_MONO_GLIB3_FMCDIO5_v3.0_09-12-2014.pdf). The [firmware](https://github.com/gauzinge/Ph2_ACF/tree/Dev/firmware) is included in this repository.



Example HWDescription.xml File with DIO5 support:
-----------------------------------------------------
```xml

<?xml version='1.0' encoding = 'UTF-8' ?>
<HwDescription>

    <!-- The file containing the connection data & IP addresses -->
    <Connections name="file://settings/connections_2CBC.xml"/>

    <Shelve Id="0" >
        <BeBoard Id="0" connectionId="board0" boardType="GLIB">

            <Module FeId="0" FMCId="0" ModuleId="0" Status="1">

                <!-- Global CBC registers are applied to all CBCs and override register values from the config files -->
                <Global_CBC_Register name="TriggerLatency"> 0x0C </Global_CBC_Register> -->

                <!-- a base path to the CBC files -->
                <CBC_Files path="./settings/"/>

                <CBC Id="0" configfile="Cbc_default_hole.txt"/>
                <CBC Id="1" configfile="Cbc_default_hole.txt"/>
            </Module>

            <!-- Commissioning Mode -->
            <!-- set to 1 to enable commissioning mode -->
            <Register name="COMMISSIONNING_MODE_RQ"> 0 </Register>
            <!-- set to 1 to enable test pulse in commissioning mode -->
            <Register name="COMMISSIONNING_MODE_CBC_TEST_PULSE_VALID"> 0 </Register>
            <!-- Delays after testpulse / L1A -->
            <Register name="COMMISSIONNING_MODE_DELAY_AFTER_FAST_RESET"> 50 </Register>
            <Register name="COMMISSIONNING_MODE_DELAY_AFTER_L1A"> 400 </Register>
            <Register name="COMMISSIONNING_MODE_DELAY_AFTER_TEST_PULSE"> 201 </Register>

            <!-- Trigger -->
            <!-- set to 1 to use external triggers -->
            <Register name="user_wb_ttc_fmc_regs.pc_commands.TRIGGER_SEL"> 0 </Register>
            <!-- internal trigger frequency: see address_table*.xml for details -->
            <Register name="user_wb_ttc_fmc_regs.pc_commands.INT_TRIGGER_FREQ"> 10 </Register>

            <!-- ************************************ -->
            <!-- This part is only required with the DIO5 mezzanine -->

            <!-- DIO5 threshold: [v]/3.3*256 -->
            <Register name="user_wb_ttc_fmc_regs.dio5.fmcdio5_threshold_trig_in"> 40 </Register>
            <!-- set to 0 for rising edge, 1 for falling -->
            <Register name="user_wb_ttc_fmc_regs.dio5.fmcdio5_trig_in_edge"> 0 </Register>
            <!-- Leave as default -->
            <Register name="user_wb_ttc_fmc_regs.dio5.fmcdio5_trig_in_50ohms" > 1 </Register>
            <Register name="user_wb_ttc_fmc_regs.dio5.fmcdio5_trig_out_50ohms"> 0 </Register>
            <!-- set to 1 to output L1A signal, 0 for input pulse -->
            <Register name="user_wb_ttc_fmc_regs.dio5.fmcdio5_lemo2_sig_sel"> 1 </Register>

            <!-- Clock -->
            <!-- set to 1 for external clocking -->
            <Register name="user_wb_ttc_fmc_regs.dio5.clk_mux_sel"> 0 </Register>
            <!-- DIO5 threshold: [v]/3.3*256 -->
            <Register name="user_wb_ttc_fmc_regs.dio5.fmcdio5_threshold_clk_in"> 40 </Register>
            <!-- Leave as default -->
            <Register name="user_wb_ttc_fmc_regs.dio5.fmcdio5_clk_in_50ohms"> 1 </Register>
            <Register name="user_wb_ttc_fmc_regs.dio5.fmcdio5_clk_out_50ohms"> 0 </Register>

           <!-- End of DIO5 registers -->
            <!-- ************************************ -->

            <!-- Acquisition -->
            <Register name="user_wb_ttc_fmc_regs.pc_commands.ACQ_MODE"> 1 </Register>
            <Register name="cbc_stubdata_latency_adjust_fe1"> 1 </Register>
            <Register name="cbc_stubdata_latency_adjust_fe2"> 1 </Register>
            <Register name="user_wb_ttc_fmc_regs.pc_commands.CBC_DATA_GENE"> 1 </Register>
            <Register name="user_wb_ttc_fmc_regs.pc_commands.CBC_DATA_PACKET_NUMBER"> 10 </Register>
            <Register name="user_wb_ttc_fmc_regs.pc_commands2.clock_shift"> 0 </Register>

            <!-- Polarity -->
            <Register name="user_wb_ttc_fmc_regs.pc_commands2.negative_logic_CBC"> 0 </Register>
            <Register name="user_wb_ttc_fmc_regs.pc_commands2.negative_logic_sTTS"> 0 </Register>
            <Register name="user_wb_ttc_fmc_regs.pc_commands2.polarity_tlu"> 0 </Register>
        </BeBoard>
    </Shelve>
</HwDescription>

<!-- Settings node to pass any std::string - int combination to applications -->
<Settings>
    <Setting name="RunNumber"> 1 </Setting>
    <!-- Hole mode from settings overrides  "negative_logic_CBC" register-->
    <Setting name="HoleMode"> 1 </Setting>
</Settings>


```


Known Issues:
---------------------
Several bugs / problematic behavior has been reported by various users that is not direclty linked to the Ph2_ACF libraries, however, some workarounds are provided:

- When configuring a CBC object (writing all registers at once), the MSB of the Register "FrontEncControl" is read back incorrectly. This only manifests in electron mode (0xC3 instead of 0x43). The cause of this problem is identified as a FW artefact and the error itself can be safely ignored until the problem is solved. The chips will still properly configure and data quality should not be affected.

- uHAL exceptions and UDP timeouts when reading larger packet sizes from the GLIB board: 
    this can happen for some users (cause not yet identified) but can be circumvented by changing the line

        "ipbusudp-2.0://192.168.000.175:50001"

    in the connections.xml file to

        "chtcp-2.0://localhost:10203?target=192.168.000.175:50001"

    & then launching the CACTUS control hub by the command:

        /opt/cactus/bin/controlhub_start

    This uses TCP protocol instead of UDP which accounts for packet loss but decreases the performance.

- SegmentationViolations on lines that contain
        
        gStyle->Set ... ;

    statements. This has been observed by several users on the VM and can be fixed by re-compiling ROOT using GCC 4.8



__Known Issues:__
-------------------------
Several bugs / problematic behavior has been reported by various users that is not direclty linked to the Ph2_ACF libraries, however, some workarounds are provided in the following:

- When configuring a CBC object (writing all registers at once), the MSB of the Register "FrontEncControl" is read back incorrectly. This only manifests in hole mode (0xC3 instead of 0x43). The cause of this problem is identified as related to the FW and the error itself can be safely ignored until the problem is solved. The chips will still properly configure and data quality should not be affected.

- uHAL exceptions and UDP timeouts when reading larger packet sizes from the GLIB board: this can happen for some users (cause not yet identified) but can be circumvented by changing the line
        "ipbusudp-2.0://192.168.000.175:50001"
in the connections.xml file to
        "chtcp-2.0://localhost:10203?target=192.168.000.175:50001"
& then launching the CACTUS control hub by the command:
        /opt/cactus/bin/controlhub_start

This uses TCP protocol instead of UDP which accounts for packet loss but decreases the performance.




Support, Suggestions ?
----------------------

For any support request/suggestions, mail georg.auzingerSPAMNOT@cern.ch
