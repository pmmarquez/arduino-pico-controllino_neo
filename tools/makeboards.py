#!/usr/bin/env python3

def BuildFlashMenu(name, flashsize, fssizelist):
    for fssize in fssizelist:
        if fssize == 0:
            fssizename = "no FS"
        elif fssize < 1024 * 1024:
            fssizename = "FS: %dKB" % (fssize / 1024)
        else:
            fssizename = "FS: %dMB" % (fssize / (1024 * 1024))
        mn="%d_%d" % (flashsize, fssize)
        print("%s.menu.flash.%s=%dMB (%s)" % (name, mn, flashsize / (1024 * 1024), fssizename))
        print("%s.menu.flash.%s.upload.maximum_size=%d" % (name, mn, flashsize - 4096 - fssize))
        print("%s.menu.flash.%s.build.flash_length=%d" % (name, mn, flashsize - 4096 - fssize))
        print("%s.menu.flash.%s.build.eeprom_start=%d" % (name, mn, int("0x10000000",0) + flashsize - 4096))
        print("%s.menu.flash.%s.build.fs_start=%d" % (name, mn, int("0x10000000",0) + flashsize - 4096 - fssize))
        print("%s.menu.flash.%s.build.fs_end=%d" % (name, mn, int("0x10000000",0) + flashsize - 4096))

def BuildDebugPort(name):
    print("%s.menu.dbgport.Disabled=Disabled" % (name))
    print("%s.menu.dbgport.Disabled.build.debug_port=" % (name))
    for p in ["Serial", "Serial1", "Serial2"]:
        print("%s.menu.dbgport.%s=%s" % (name, p, p))
        print("%s.menu.dbgport.%s.build.debug_port=-DDEBUG_RP2040_PORT=%s" % (name, p, p))

def BuildDebugLevel(name):
    for l in [ ("None", ""), ("Core", "-DDEBUG_RP2040_CORE"), ("SPI", "-DDEBUG_RP2040_SPI"), ("Wire", "DDEBUG_RP2040_WIRE"),
               ("All", "-DDEBUG_RP2040_WIRE -DDEBUG_RP2040_SPI -DDEBUG_RP2040_CORE"), ("NDEBUG", "-DNDEBUG") ]:
        print("%s.menu.dbglvl.%s=%s" % (name, l[0], l[0]))
        print("%s.menu.dbglvl.%s.build.debug_level=%s" % (name, l[0], l[1]))

def BuildFreq(name):
    for f in [ 125, 50, 100, 133, 150, 175, 200, 225, 250, 275, 300]:
        warn = ""
        if f > 133: warn = " (Overclock)"
        print("%s.menu.freq.%s=%s MHz%s" % (name, f, f, warn))
        print("%s.menu.freq.%s.build.f_cpu=%dL" % (name, f, f * 1000000))

def BuildBoot(name):
    for l in [ ("Generic SPI /2", "boot2_generic_03h_2_padded_checksum"),  ("Generic SPI /4", "boot2_generic_03h_4_padded_checksum"),
            ("IS25LP080 QSPI /2", "boot2_is25lp080_2_padded_checksum"), ("IS25LP080 QSPI /4", "boot2_is25lp080_4_padded_checksum"),
            ("W25Q080 QSPI /2", "boot2_w25q080_2_padded_checksum"), ("W25Q080 QSPI /4", "boot2_w25q080_4_padded_checksum"),
            ("W25X10CL QSPI /2", "boot2_w25x10cl_2_padded_checksum"), ("W25X10CL QSPI /4", "boot2_w25x10cl_4_padded_checksum") ]:
        print("%s.menu.boot2.%s=%s" % (name, l[1], l[0]))
        print("%s.menu.boot2.%s.build.boot2=%s" % (name, l[1], l[1]))

def BuildHeader(name, prettyname, pid, boarddefine, variant, uploadtool, flashsize, boot2):
    print("%s.name=%s" % (name, prettyname))
    print("%s.vid.0=0x2e8a" % (name))
    print("%s.pid.0=%s" % (name, pid))
    print("%s.build.usbpid=-DSERIALUSB_PID=%s" % (name, pid))
    print("%s.build.board=%s" % (name, boarddefine))
    print("%s.build.mcu=cortex-m0plus" % (name))
    print("%s.build.variant=%s" % (name, variant))
    print("%s.upload.tool=%s" % (name, uploadtool))
    print("%s.upload.maximum_size=%d" % (name, flashsize))
    print("%s.upload.maximum_data_size=262144" % (name))
    print("%s.upload.wait_for_upload_port=true" % (name))
    print("%s.upload.erase_cmd=" % (name))
    print("%s.serial.disableDTR=false" % (name))
    print("%s.serial.disableRTS=false" % (name))
    print("%s.build.f_cpu=125000000" % (name))
    print("%s.build.led=" % (name))
    print("%s.build.core=rp2040" % (name))
    print("%s.build.mcu=rp2040" % (name))
    print("%s.build.ldscript=memmap_default.ld" % (name))
    print("%s.build.boot2=%s" % (name, boot2))

def BuildGlobalMenuList():
    print("menu.BoardModel=Model")
    print("menu.flash=Flash Size")
    print("menu.freq=CPU Speed")
    print("menu.dbgport=Debug Port")
    print("menu.dbglvl=Debug Level")
    print("menu.boot2=Boot Stage 2")


def MakeBoard(name, prettyname, pid, boarddefine, flashsizemb, boot2):
    for a, b, c in [ ["", "", "uf2conv"], ["picoprobe", " (Picoprobe)", "picoprobe"]]:
        n = name + a
        p = prettyname + b
        fssizelist = [ 0, 64 * 1024, 128 * 1024, 256 * 1024, 512 * 1024 ]
        for i in range(1, flashsizemb):
            fssizelist.append(i * 1024 * 1024)
        BuildHeader(n, p, pid, boarddefine, name, c, flashsizemb * 1024 * 1024, boot2)
        if name == "generic":
            BuildFlashMenu(n, 2*1024*1024, [0, 1*1024*1024])
            BuildFlashMenu(n, 4*1024*1024, [0, 2*1024*1024])
            BuildFlashMenu(n, 8*1024*1024, [0, 4*1024*1024])
            BuildFlashMenu(n, 16*1024*1024, [0, 8*1024*1024])
        else:
            BuildFlashMenu(n, flashsizemb * 1024 * 1024, fssizelist)
        BuildFreq(n)
        BuildDebugPort(n)
        BuildDebugLevel(n)
        if name == "generic":
            BuildBoot(n)


BuildGlobalMenuList()
MakeBoard("rpipico", "Raspberry Pi Pico", "0x000a", "RASPBERRY_PI_PICO", 2, "boot2_w25q080_2_padded_checksum")
MakeBoard("adafruitfeather", "Adafruit Feather RP2040", "0x000b", "ADAFRUIT_FEATHER_RP2040", 8, "boot2_generic_03h_4_padded_checksum")
MakeBoard("generic", "Generic RP2040", "0xf00a", "GENERIC_RP2040", 16, "boot2_generic_03h_4_padded_checksum")
