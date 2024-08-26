#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <filesystem>
#include <algorithm>
#include <vector>
#include <fstream>
#include <thread>

#include <libudev.h>
#include <string.h>

using namespace std;

struct input_event {
    struct timeval time;
    unsigned short type;
    unsigned short code;
    unsigned int value;
};

vector<string> get_files(string path) {
    vector<string> files;
    for (const auto &entry : filesystem::directory_iterator(path)) {
        files.push_back(entry.path());
    }
    return files;
}

void unpackData(const char* data, input_event &event) {
    memcpy(&event.time.tv_sec, data, sizeof(event.time.tv_sec));
    memcpy(&event.time.tv_usec, data + 4, sizeof(event.time.tv_usec));
    memcpy(&event.type, data + 8, sizeof(event.type));
    memcpy(&event.code, data + 10, sizeof(event.code));
    memcpy(&event.value, data + 12, sizeof(event.value));
}

void unbind_device(const string devpath) {
    ofstream outFile("/sys/bus/usb/drivers/usb/unbind");

    if (outFile.is_open()) {
        outFile << devpath;
        outFile.close();
        cout << "Device unbinded: " << devpath << endl;
    } else {
        cout << "Unable to open file.\n";
    }
}

void watch_device(const string device, const string devpath) {
    cout << "Watching " << device << " (" << devpath << ")" << endl;
    int fd = open(device.c_str(), O_RDONLY);
    if (fd < 0) {
        cerr << "Failed to open " << device << endl;
        return;
    }

    input_event event;
    input_event last_event;
    bool defined = false;
    int seconds, useconds;

    int flags = 0;
    int last_flag = 0;

    while (true) {
        int bytesRead = (int)read(fd, &event, sizeof(event));
        if (bytesRead < (int)sizeof(event)) {
            cerr << "Error reading from device." << endl;
            break;
        }


        if (defined && event.value == 1 && last_event.value == 1) {
            seconds = event.time.tv_sec - last_event.time.tv_sec;
            useconds = event.time.tv_usec - last_event.time.tv_usec;

            if (seconds == 0) {
                //cout << useconds << endl;
                if (last_flag == 0) last_flag = event.time.tv_sec;
                if ((event.time.tv_sec - last_flag) < 5) {
                    if (useconds < 10000) {
                        flags += 1;
                        last_flag = event.time.tv_sec;
                    }
                } else {
                    flags = 0;
                }

                if (flags >= 5) {
                    cout << "RUBBER DUCKY DETECTED!" << endl;
                    unbind_device(devpath);
                    flags = 0;
                    return;
                }
            }
        }
        
        if (event.value == 1) {
            last_event = event;
            defined = true;
        }
    }

    close(fd);
    return;
}

int main() {
    const string events_filepath = "/dev/input/";
    //const string unbind_filepath = "/sys/bus/usb/drivers/";
    //string filepath = "./test";

    struct udev* udev = udev_new();
    if (!udev) {
        cerr << "Failed to create udev object\n";
        return 1;
    }

    struct udev_monitor* mon = udev_monitor_new_from_netlink(udev, "udev");
    if (!mon) {
        cerr << "Failed to create udev monitor\n";
        udev_unref(udev);
        return 1;
    }

    udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", NULL);
    udev_monitor_enable_receiving(mon);

    int fd = udev_monitor_get_fd(mon);

    vector<string> old_events_files, new_events_files;
    vector<string> old_usb_files, new_usb_files;

    old_events_files = get_files(events_filepath);
    string keyboard_events_path;
    string devpath;

    while (true) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        int ret = select(fd + 1, &fds, NULL, NULL, NULL);
        if (ret > 0 && FD_ISSET(fd, &fds)) {
            struct udev_device* dev = udev_monitor_receive_device(mon);
            if (dev) {
                const string action = (string)udev_device_get_action(dev);
                const char* devtype = udev_device_get_devtype(dev);

                if (action != "" && devtype && strcmp(devtype, "usb_device") == 0) {
                    if (action == "add") {
                        cout << "New USB device" << endl;
                        new_events_files.clear();
                        new_events_files = get_files(events_filepath);

                        for (const auto path : new_events_files) {;
                            if (find(old_events_files.begin(), old_events_files.end(), path) == old_events_files.end()) {
                                devpath = (string) udev_device_get_devpath(dev);
                                devpath = devpath.substr(devpath.find_last_of("/") + 1);

                                thread subthread(watch_device, (string)path, (string)devpath);
                                subthread.detach();
                                keyboard_events_path = (string) path;
                            }
                        }


                        old_events_files = new_events_files;
                    } else if (action == "remove") {
                        cout << action << endl;
                        old_events_files = get_files(events_filepath);
                    }
                }

                udev_device_unref(dev);
            }
        }

    }


    udev_monitor_unref(mon);
    udev_unref(udev);

}
