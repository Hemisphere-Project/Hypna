
#include <Arduino.h>
#include "USB.h"
#include "esp32-hal-tinyusb.h"

static const char *TAG = "usbdmidi";

// Message callbacks
void (*midi_callback)(uint8_t channel, uint8_t type, uint8_t key, uint8_t value) = NULL;
void (*midinote_callback)(uint8_t channel, uint8_t note, uint8_t velocity) = NULL;
void (*midicc_callback)(uint8_t channel, uint8_t cc, uint8_t value) = NULL;
void (*midipc_callback)(uint8_t channel, uint8_t pc) = NULL;

// Basic MIDI Messages
#define MIDI_NOTEOFF 0x80
#define MIDI_NOTEON 0x90
#define MIDI_CC 0xB0
#define MIDI_PC 0xC0

// From usb.org MIDI 1.0 specification. This 4 byte structure is the unit
// of transfer for MIDI data over USB.
typedef struct __attribute__((__packed__))
{
    uint8_t code_index_number : 4;
    uint8_t cable_number : 4;
    uint8_t MIDI_0;
    uint8_t MIDI_1;
    uint8_t MIDI_2;
} USB_MIDI_t;

// NOTEON:
static void usbmidi_noteon(uint8_t cable_num, uint8_t channel, uint8_t note, uint8_t velocity)
{
    if (tud_midi_mounted())
    {
        uint8_t note_on[3] = {MIDI_NOTEON | channel, note, velocity};
        tud_midi_stream_write(cable_num, note_on, 3);
    }
}

// NOTEON CABLE 0:
static void usbmidi_noteon(uint8_t channel, uint8_t note, uint8_t velocity)
{
    usbmidi_noteon(0, channel, note, velocity);
}

// NOTEOFF:
static void usbmidi_noteoff(uint8_t cable_num, uint8_t channel, uint8_t note, uint8_t velocity)
{
    if (tud_midi_mounted())
    {
        uint8_t note_off[3] = {MIDI_NOTEOFF | channel, note, velocity};
        tud_midi_stream_write(cable_num, note_off, 3);
    }
}

// NOTEOFF CABLE 0:
static void usbmidi_noteoff(uint8_t channel, uint8_t note, uint8_t velocity)
{
    usbmidi_noteoff(0, channel, note, velocity);
}

// CC:
static void usbmidi_cc(uint8_t cable_num, uint8_t channel, uint8_t cc, uint8_t value)
{
    if (tud_midi_mounted())
    {
        uint8_t cc_msg[3] = {MIDI_CC | channel, cc, value};
        tud_midi_stream_write(cable_num, cc_msg, 3);
    }
}

// CC CABLE 0:
static void usbmidi_cc(uint8_t channel, uint8_t cc, uint8_t value)
{
    usbmidi_cc(0, channel, cc, value);
}

// PC:
static void usbmidi_pc(uint8_t cable_num, uint8_t channel, uint8_t pc)
{
    if (tud_midi_mounted())
    {
        uint8_t pc_msg[2] = {MIDI_PC | channel, pc};
        tud_midi_stream_write(cable_num, pc_msg, 2);
    }
}

// PC CABLE 0:
static void usbmidi_pc(uint8_t channel, uint8_t pc)
{
    usbmidi_pc(0, channel, pc);
}


// READ MIDI PACKETS
//
static void usbmidi_task_read(void *arg)
{
    // The MIDI interface always creates input and output port/jack descriptors
    // regardless of these being used or not. Therefore incoming traffic should be
    // read (possibly just discarded) to avoid the sender blocking in IO
    uint8_t packet[4];
    bool read = false;
    for (;;)
    {
        delay(1);
        while (tud_midi_available())
        {
            read = tud_midi_packet_read(packet);
            if (read)
            {
                ESP_LOGI(TAG,
                         "Read - Time (ms since boot): %lld, Data: %02hhX %02hhX "
                         "%02hhX %02hhX",
                         esp_timer_get_time(), packet[0], packet[1], packet[2],
                         packet[3]);
                USB_MIDI_t *m = (USB_MIDI_t *)packet;
                Serial.printf(
                    "%lld: Cable: %d Code: %01hhX, Data: %02hhX %02hhX %02hhX\n",
                    esp_timer_get_time(), m->cable_number, m->code_index_number,
                    m->MIDI_0, m->MIDI_1, m->MIDI_2);

                uint8_t cin = m->code_index_number;
                uint8_t type = m->MIDI_0 & 0xF0;
                uint8_t channel = (m->MIDI_0 & 0x0F) + 1;
                uint8_t key = m->MIDI_1;
                uint8_t value = m->MIDI_2;
                
                // check CIN and type are valid
                if (cin << 4 != type) {
                    ESP_LOGE(TAG, "Invalid CIN %02hhX", cin);
                    continue;
                }

                // Call the callback function if set
                if (midi_callback) midi_callback(channel, type, key, value);

                // Call the note callback function if set
                if ( (type == MIDI_NOTEON || type == MIDI_NOTEOFF) && midinote_callback)
                {
                    if (type == MIDI_NOTEOFF) value = 0;
                    midinote_callback(channel, key, value);
                }

                // Call the CC callback function if set
                if (type == MIDI_CC && midicc_callback) midicc_callback(channel, key, value);

                // Call the PC callback function if set
                if (type == MIDI_PC && midipc_callback) midipc_callback(channel, key);

                // Echo the packet back to the sender
                // if (tud_midi_mounted())
                // {
                //     tud_midi_stream_write(m->cable_number, packet, 4);
                // }
            }
        }
    }
}


// SEND MIDI PACKETS (Exemple)
//
static void periodic_midi_write_example_cb(void *arg)
{
    // Example melody stored as an array of note values
    uint8_t const note_sequence[] = {
        74, 78, 81, 86, 90, 93, 98, 102, 57, 61, 66, 69, 73, 78, 81, 85,
        88, 92, 97, 100, 97, 92, 88, 85, 81, 78, 74, 69, 66, 62, 57, 62,
        66, 69, 74, 78, 81, 86, 90, 93, 97, 102, 97, 93, 90, 85, 81, 78,
        73, 68, 64, 61, 56, 61, 64, 68, 74, 78, 81, 86, 90, 93, 98, 102};

    static uint8_t const cable_num = 0; // MIDI jack associated with USB endpoint
    static uint8_t const channel = 0;   // 0 for channel 1
    static uint32_t note_pos = 0;

    // Previous positions in the note sequence.
    int previous = note_pos - 1;

    // If we currently are at position 0, set the
    // previous position to the last note in the sequence.
    if (previous < 0)
    {
        previous = sizeof(note_sequence) - 1;
    }

    // Send Note On for current position at full velocity (127) on channel 1.
    ESP_LOGI(TAG, "Writing MIDI data %d", note_sequence[note_pos]);

    if (tud_midi_mounted())
    {
        uint8_t note_on[3] = {MIDI_NOTEON | channel, note_sequence[note_pos], 127};
        tud_midi_stream_write(cable_num, note_on, 3);

        // Send Note Off for previous note.
        uint8_t note_off[3] = {MIDI_NOTEOFF | channel, note_sequence[previous], 0};
        tud_midi_stream_write(cable_num, note_off, 3);
    }

    // Increment position
    note_pos = (note_pos + 1) % sizeof(note_sequence);

}

// USB event callback
//
static void usbEventCallback(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == ARDUINO_USB_EVENTS)
    {
        arduino_usb_event_data_t *data = (arduino_usb_event_data_t *)event_data;
        switch (event_id)
        {
        case ARDUINO_USB_STARTED_EVENT:
            ESP_LOGI(TAG, "USB PLUGGED");
            break;
        case ARDUINO_USB_STOPPED_EVENT:
            ESP_LOGI(TAG, "USB UNPLUGGED");
            break;
        case ARDUINO_USB_SUSPEND_EVENT:
            ESP_LOGI(TAG, "USB SUSPENDED " + (data->suspend.remote_wakeup_en ? " (remote wakeup)" : ""));
            break;
        case ARDUINO_USB_RESUME_EVENT:
            ESP_LOGI(TAG, "USB RESUMED");
            break;

        default:
            break;
        }
    }
}

// TinyUSB descriptor callback
//
extern "C" uint16_t tusb_midi_load_descriptor(uint8_t *dst, uint8_t *itf)
{
    uint8_t str_index = tinyusb_add_string_descriptor("USBMIDI");
    uint8_t ep_num = tinyusb_get_free_duplex_endpoint();
    TU_VERIFY(ep_num != 0);
    uint8_t descriptor[TUD_MIDI_DESC_LEN] = {
        // Interface number, string index, EP Out & EP In address, EP size
        TUD_MIDI_DESCRIPTOR(*itf, str_index, ep_num, (uint8_t)(0x80 | ep_num), 64)};
    *itf += 1;
    memcpy(dst, descriptor, TUD_MIDI_DESC_LEN);
    return TUD_MIDI_DESC_LEN;
}

// USB MIDI initialization
//
void usbmidi_init()
{

    USB.onEvent(usbEventCallback);
    tinyusb_enable_interface(USB_INTERFACE_MIDI, TUD_MIDI_DESC_LEN, tusb_midi_load_descriptor);
    USB.begin();

    while (!Serial && millis() < 5000) delay(10);
    ESP_LOGI(TAG, "USB started");

    // Periodically send MIDI packets
    // int const tempo = 286;
    // const esp_timer_create_args_t periodic_midi_args = {
    //     .callback = &periodic_midi_write_example_cb,
    //     /* name is optional, but may help identify the timer when debugging */
    //     .name = "periodic_midi"};

    // ESP_LOGI(TAG, "MIDI write task init");
    // esp_timer_handle_t periodic_midi_timer;
    // ESP_ERROR_CHECK(esp_timer_create(&periodic_midi_args, &periodic_midi_timer));
    // ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_midi_timer, tempo * 1000));

    // Read recieved MIDI packets
    ESP_LOGI(TAG, "MIDI read task init");
    xTaskCreate(usbmidi_task_read, "usbmidi_task_read", 2 * 1024, NULL, 0, NULL);
}