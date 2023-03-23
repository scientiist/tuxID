////////////////////////////////////////////////
// Hardware ID - Know Who You're Dealing With //
// @auth William J. Tomasine II, Josh O'Leary //

// ABSTRACT //
// This module facilitates uniquely identifying devices via reading hardware information.
// Such as peripherial serial numbers, drive configurations, component specs.
// In this specific implementation case, for x86 desktops running Linux.
//
// A Hardware Profile is a structure, containing fields for each Hardware Token collected.
// A Hardware Hash is generated via concatenating the Hardware Profile fields
// into one string, and feeding that to a standard cryptographic hashing algorithm.
//
// This hash uniquely identifies the device, without presenting a privacy hazard to the device owner.
// Use cases:
// Prevent MultiAccounting in online games.
// Detect end-users running VPNs, Virtual Machines etc.
//
// Further Considerations: hardware changes slowly over time, but hashes

// LICENSE //
// MIT probably

// SAMPLES //

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <libudev.h>
#include <sys/stat.h>
#include <sys/stat.h>
#include <unistd.h>


namespace tuxID
{

    struct HardwareProfile
    {
        std::string diskSerialCode;
    };


    HardwareProfile getCurrentHardwareProfile();
    std::string getHardwareHash(HardwareProfile);
    std::string getHardwareHash();
    std::string getDiskSerialCode();
    bool getIsLikelyVirtualMachine();
    bool getIsDefinitelyVirtualMachine();
    bool isVirtualMachine();
    bool isSuperUser();
    bool shellCommandReturns(const char* command);

}
int load_udev() {}
int udev_error() { }
bool hasDisk(const char* disk) {
	struct udev *ud = NULL;
	struct stat statbuf;
	struct udev_device *device = NULL;
	struct udev_list_entry *entry = NULL;

	ud = udev_new();
	if (NULL == ud) { udev_error(); exit(1); }

	if (0 != stat(disk, &statbuf)) {
		std::cout << "Failed to stat " << disk << std::endl;
	}
	device = udev_device_new_from_devnum(ud, 'b', statbuf.st_rdev);
}

bool tuxID::isSuperUser() {
    if (getuid() == 0)
        return 1;
    return 0;
}

bool tuxID::shellCommandReturns(const char* command) {
    FILE *shellCommand = popen(command, "r");
    char buf[16];
    if (fread (buf, 1, sizeof (buf), shellCommand) > 0) {
        return 1;
    }
}
// disk = "/dev/sda"
std::string tuxID::getDiskSerialCode()  {
    struct udev *ud = NULL;
    struct stat statbuf;
    struct udev_device *device = NULL;
    struct udev_list_entry *entry = NULL;

    ud = udev_new();
    if (NULL == ud) {
        fprintf(stderr, "Failed to init udev.\n");
        exit(1);
    }
    // TODO: Detect drive type
    // IDE drives are /dev/hda /dev/mmcblk /dev/nvme
    std::string diskTypes[5] = {"/dev/sda", "/dev/hda", "/dev/mmcblk0", "/dev/nvme0"};
    int arrayPosition = 0;
    while(0 != stat(diskTypes[arrayPosition].c_str(), &statbuf)){
        arrayPosition = arrayPosition + 1;
    }
    if (0 != stat(diskTypes[arrayPosition].c_str(), &statbuf)) {
	std::cout << "Failed to open disk " << std::endl;
	exit(1);
    }
    device = udev_device_new_from_devnum(ud, 'b', statbuf.st_rdev);
    if (NULL == device) {
       	std::cout << "Failed to open disk"<< std::endl;
        exit(1);
    }

    entry = udev_device_get_properties_list_entry(device);
    while (NULL != entry) {
        if (0 == strcmp(udev_list_entry_get_name(entry),
                        "ID_SERIAL")) {
            break;
        }

        entry = udev_list_entry_get_next(entry);
    }
    //printf(udev_list_entry_get_value(entry));
    return std::string(udev_list_entry_get_value(entry));
}

bool tuxID::isVirtualMachine() {
    if (tuxID::shellCommandReturns("lsmod | grep virtio"))
        return 1;
    if (tuxID::shellCommandReturns("lsmod | grep vboxguest"))
        return 1;
    if (tuxID::shellCommandReturns("lsmod | grep vmwgfx"))
        return 1;
    if(tuxID::shellCommandReturns("lsmod | grep cirrus"))
        return 1;
    if(tuxID::shellCommandReturns("lsmod | grep qemu"))
        return 1;
    if (tuxID::shellCommandReturns("cat /etc/fstab | grep /dev/vda"))
        return 1;

    return 0;
}
