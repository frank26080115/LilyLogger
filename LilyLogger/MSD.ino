#include "FS.h"
#include "FFat.h"

#include "USB.h"
#include "USBMSC.h"

USBMSC MSC;

wl_handle_t flash_handle;
uint32_t sect_size;
uint32_t total_size;
uint32_t sect_cnt;

void usbEventCallback(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize);
int32_t onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize);
bool onStartStop(uint8_t power_condition, bool start, bool load_eject);

void usbmsc_init()
{
    flash_handle = FFat.getWlHandle();

    sect_size = wl_sector_size(flash_handle);
    total_size = wl_size(flash_handle);
    sect_cnt = total_size / sect_size;

    USB.onEvent(usbEventCallback);
    MSC.vendorID("ESP32");
    MSC.productID("USB_MSC");
    MSC.productRevision("1.0");
    MSC.onRead(onRead);
    MSC.onWrite(onWrite);
    MSC.mediaPresent(true);
    MSC.begin(sect_cnt, sect_size);
    USB.begin();
}

void usbEventCallback(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
  if(event_base == ARDUINO_USB_EVENTS){
    arduino_usb_event_data_t * data = (arduino_usb_event_data_t*)event_data;
    switch (event_id){
      case ARDUINO_USB_STARTED_EVENT:
        break;
      case ARDUINO_USB_STOPPED_EVENT:
        break;
      case ARDUINO_USB_SUSPEND_EVENT:
        break;
      case ARDUINO_USB_RESUME_EVENT:
        break;
      
      default:
        break;
    }
  }
}

int32_t onWrite(uint32_t lba, uint32_t offset, uint8_t* buffer, uint32_t bufsize)
{
  wl_write(flash_handle, (size_t)((lba * sect_size) + offset), (const void*) buffer, (size_t)bufsize);
  return bufsize;
}

int32_t onRead(uint32_t lba, uint32_t offset, void* buffer, uint32_t bufsize)
{
  wl_read(flash_handle, (size_t)((lba * sect_size) + offset), (void*)buffer, (size_t)bufsize);
  return bufsize;
}
