#include <events/mbed_events.h>
#include <mbed.h>
#include "ble/BLE.h"
#include "ble/gap/Gap.h"
#include "ble/services/BatteryService.h"
#include "ble/services/DeviceInformationService.h"
#include "pretty_printer.h"
#include "ISL29125.h"

class RGBService {
public:
    typedef uint16_t RGBType_t;

    RGBService(BLE& _ble) :
        ble(_ble),
        redCharacteristic("00001111-0000-1000-8000-00805F9B34FB", &red, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY),
        greenCharacteristic("00002222-0000-1000-8000-00805F9B34FB", &green, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY),
        blueCharacteristic("00003333-0000-1000-8000-00805F9B34FB", &blue, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY)
    {
        GattCharacteristic *charTable[] = { &redCharacteristic, &greenCharacteristic, &blueCharacteristic };
        GattService rgbService(GattService::UUID_ENVIRONMENTAL_SERVICE, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
        ble.gattServer().addService(rgbService);
    }

    void updateRed(RGBType_t newRedVal) {
        red = newRedVal;
        ble.gattServer().write(redCharacteristic.getValueHandle(), (uint8_t *) &red, sizeof(RGBType_t));
    }

    void updateGreen(RGBType_t newGreenVal) {
        green = newGreenVal;
        ble.gattServer().write(greenCharacteristic.getValueHandle(), (uint8_t *) &green, sizeof(RGBType_t));
    }

    void updateBlue(RGBType_t newBlueVal) {
        blue = newBlueVal;
        ble.gattServer().write(blueCharacteristic.getValueHandle(), (uint8_t *) &blue, sizeof(RGBType_t));
    }

private:
    BLE& ble;
    RGBType_t red;
    RGBType_t green;
    RGBType_t blue;

    ReadOnlyGattCharacteristic<RGBType_t> redCharacteristic;
    ReadOnlyGattCharacteristic<RGBType_t> greenCharacteristic;
    ReadOnlyGattCharacteristic<RGBType_t> blueCharacteristic;
};

/* device name */
const static char DEVICE_NAME[] = "RGBSensor";

/* list of services */
#define UUID_RGB_SERVICE 0x181A
#define UUID_DEVICE_INFORMATION_SERVICE 0x180A
UUID uuid_list[] = { UUID_RGB_SERVICE, UUID_DEVICE_INFORMATION_SERVICE };

/* Advertising data buffer */
uint8_t adv_buffer[ble::LEGACY_ADVERTISING_MAX_SIZE];

/* BLE event queue */
static events::EventQueue event_queue(/* event count */ 16 * EVENTS_EVENT_SIZE);

bool initFlag = false;

Ticker updateSensors;

// Declare and define a measurement update flag
bool sensorFlag = false;

void updateMeasurments(){
    sensorFlag = true;
}

void on_init_complete(BLE::InitializationCompleteCallbackContext *params) {
    if (params->error != BLE_ERROR_NONE) {
        printf("Ble initialization failed.");
        initFlag = false;
        return;
    }
    initFlag = true;
}

class RGBApp : ble::Gap::EventHandler {
public:
    RGBApp(BLE &ble, events::EventQueue &event_queue) :
        _ble(ble),
        _event_queue(event_queue),
        _connected(false),
        _rgbService(ble)
        {}
    
    void update_sensor_value() {
        if (_connected) {
            uint16_t red, green, blue;

            /* Read the RGB sensor values */
            read_ISL29125(red, green, blue);

            _rgbService.updateRed(red);
            _rgbService.updateGreen(green);
            _rgbService.updateBlue(blue);
        }
    }

private:
    void onDisconnectionComplete(const ble::DisconnectionCompleteEvent&) {
        _ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
        _connected = false;
    }

    void onConnectionComplete(const ble::ConnectionCompleteEvent &event) {
        if (event.getStatus() == BLE_ERROR_NONE) {
            _connected = true;
        }
    }

private:
    BLE &_ble;
    events::EventQueue &_event_queue;
    bool _connected;
    RGBService _rgbService;
};

void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context) {
    event_queue.call(Callback<void()>(&context->ble, &BLE::processEvents));
}

void start_advertising(BLE &ble) {
    ble::AdvertisingDataBuilder builder(adv_buffer);

    builder.setFlags();
    builder.setName(DEVICE_NAME);
    builder.setLocalServiceList(mbed::make_Span(uuid_list, sizeof(uuid_list) / sizeof(UUID)));

    ble::AdvertisingParameters adv_parameters(
        ble::advertising_type_t::CONNECTABLE_UNDIRECTED,
        ble::adv_interval_t(ble::millisecond_t(1000))
    );

    ble_error_t error = ble.gap().setAdvertisingParameters(ble::LEGACY_ADVERTISING_HANDLE, adv_parameters);
    if (error) {
        printf("Error during Gap::setAdvertisingParameters: %d\n", error);
        return;
    }

    error = ble.gap().setAdvertisingPayload(ble::LEGACY_ADVERTISING_HANDLE, builder.getAdvertisingData());
    if (error) {
        printf("Error during Gap::setAdvertisingPayload: %d\n", error);
        return;
    }

    error = ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
    if (error) {
        printf("Error during Gap::startAdvertising: %d\n", error);
        return;
    }
}

int main() {
    BLE& mydevice = BLE::Instance();
    mydevice.onEventsToProcess(schedule_ble_events);

    RGBApp *eventHandler = new RGBApp(mydevice, event_queue);
    Gap& myGap = mydevice.gap();
    myGap.setEventHandler((ble::Gap::EventHandler *) eventHandler);

    mydevice.init(&on_init_complete);

    while (1) {
        if (sensorFlag) {
            eventHandler->update_sensor_value();
            sensorFlag = false;
        }

        if (initFlag) {
            print_mac_address();
            start_advertising(mydevice);

            updateSensors.attach(&updateMeasurments, 1);
            initFlag = false;
        }

        /* check for BLE events */
        event_queue.dispatch(100);
    }
    return 0;
}