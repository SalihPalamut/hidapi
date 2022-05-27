/*******************************************************
 Windows HID simplification

 Alan Ott
 Signal 11 Software

 8/22/2009

 Copyright 2009

 This contents of this file may be used by anyone
 for any reason without any conditions and may be
 used as a starting point for your own applications
 which use HIDAPI.
********************************************************/

#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>

#include <hidapi.h>

// Headers needed for sleeping.
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <signal.h>
#endif

// Fallback/example
#ifndef HID_API_MAKE_VERSION
#define HID_API_MAKE_VERSION(mj, mn, p) (((mj) << 24) | ((mn) << 8) | (p))
#endif
#ifndef HID_API_VERSION
#define HID_API_VERSION HID_API_MAKE_VERSION(HID_API_VERSION_MAJOR, HID_API_VERSION_MINOR, HID_API_VERSION_PATCH)
#endif

//
// Sample using platform-specific headers
#if defined(__APPLE__) && HID_API_VERSION >= HID_API_MAKE_VERSION(0, 12, 0)
#include <hidapi_darwin.h>
#endif

#if defined(_WIN32) && HID_API_VERSION >= HID_API_MAKE_VERSION(0, 12, 0)
#include <hidapi_winapi.h>
#endif

#if defined(USING_HIDAPI_LIBUSB) && HID_API_VERSION >= HID_API_MAKE_VERSION(0, 12, 0)
#include <hidapi_libusb.h>
#endif
//

void ReadCallBack(unsigned char *data, int size)
{
	int i = 0;
	char *Buff = (char *)calloc(size * 5, 1);

	for(i = 0; i < size; i++)
	{
		sprintf(Buff + (i * 5), "0x%02X ", data[i]);
	}

	printf("Readed Data Size %d\n%s\n", size, Buff);

	free(Buff);
}
#ifndef _WIN32
int interrupted = 1;
void sigint_handler(int sig)
{
	printf("Kill from %d\n", sig);

	interrupted = 0;
}
#endif
int main(int argc, char* argv[])
{
	(void)argc;
	(void)argv;


	int res;
#define MAX_STR 255
	wchar_t wstr[MAX_STR];
	hid_device *handle;


	struct hid_device_info *devs, *cur_dev;

#ifndef _WIN32
	// Register Signal Handler
	signal(SIGINT, sigint_handler);
#endif

	printf("hidapi test/example tool. Compiled with hidapi version %s, runtime version %s.\n", HID_API_VERSION_STR, hid_version_str());
	if(HID_API_VERSION == HID_API_MAKE_VERSION(hid_version()->major, hid_version()->minor, hid_version()->patch))
	{
		printf("Compile-time version matches runtime version of hidapi.\n\n");
	}
	else
	{
		printf("Compile-time version is different than runtime version of hidapi.\n]n");
	}

	if(hid_init())
		return -1;

#if defined(__APPLE__) && HID_API_VERSION >= HID_API_MAKE_VERSION(0, 12, 0)
	// To work properly needs to be called before hid_open/hid_open_path after hid_init.
	// Best/recommended option - call it right after hid_init.
	hid_darwin_set_open_exclusive(0);
#endif

	devs = hid_enumerate(0x0, 0x0);
	cur_dev = devs;
	while(cur_dev)
	{
		printf("Device Found\n  type: %04hx %04hx\n  path: %s\n  serial_number: %ls", cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
		printf("\n");
		printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
		printf("  Product:      %ls\n", cur_dev->product_string);
		printf("  Release:      %hx\n", cur_dev->release_number);
		printf("  Interface:    %d\n",  cur_dev->interface_number);
		printf("  Usage (page): 0x%hx (0x%hx)\n", cur_dev->usage, cur_dev->usage_page);
		printf("\n");
		cur_dev = cur_dev->next;
	}
	hid_free_enumeration(devs);

#ifdef Debug
	printf("!!Debug Mode ON!!\n\n");
#endif


	// Open the device using the VID, PID,
	// and optionally the Serial number.
	////handle = hid_open(0x4d8, 0x3f, L"12345");
	handle = hid_open_Callback(NULL, 0x0416, 0x5020, NULL, &ReadCallBack);
	if(!handle)
	{
		printf("unable to open device\n");
		return 1;
	}

	// Read the Manufacturer String
	wstr[0] = 0x0000;
	res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
	if(res < 0)
		printf("Unable to read manufacturer string\n");
	printf("Manufacturer String: %ls\n", wstr);

	// Read the Product String
	wstr[0] = 0x0000;
	res = hid_get_product_string(handle, wstr, MAX_STR);
	if(res < 0)
		printf("Unable to read product string\n");
	printf("Product String: %ls\n", wstr);


	while(interrupted)
	{
		sleep(5);
	}

	hid_close(handle);

	/* Free static HIDAPI objects. */
	hid_exit();

#ifdef _WIN32
	system("pause");
#endif

	return 0;
}
