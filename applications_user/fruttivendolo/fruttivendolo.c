#include <furi.h>
#include <furi_hal.h>
//#include <furi_hal_gpio.h>
//#include <furi_hal_resources.h>
#include <gui/gui.h>
#include <locale/locale.h>
#include <input/input.h>
#include <storage/filesystem_api_defines.h>
#include <storage/storage.h>
//#include <stream/stream.h>
//#include <stream/buffered_file_stream.h>
//#include <toolbox/stream/file_stream.h>
#define TAG "fruttivendolo_app"

char* line_;
char* par1;
int BUFFER = 30;

typedef enum {
    fruttivendoloEventTypeKey,
    // You can add additional events here.
} fruttivendoloEventType;

typedef struct {
    fruttivendoloEventType type; // The reason for this event.
    InputEvent input;   // This data is specific to keypress data.
    // You can add additional data that is helpful for your events.
} fruttivendoloEvent;

typedef struct {
    FuriString* buffer;
    // You can add additional state here.
} fruttivendoloData;

typedef struct {
    FuriMessageQueue* queue; // Message queue (fruttivendoloEvent items to process).
    FuriMutex* mutex; // Used to provide thread safe access to data.
    fruttivendoloData* data; // Data accessed by multiple threads (acquire the mutex before accessing!)
} fruttivendoloContext;

// Invoked when input (button press) is detected.  We queue a message and then return to the caller.
static void fruttivendolo_input_callback(InputEvent* input_event, FuriMessageQueue* queue) {
    furi_assert(queue);
    fruttivendoloEvent event = {.type = fruttivendoloEventTypeKey, .input = *input_event};
    furi_message_queue_put(queue, &event, FuriWaitForever);
}

// Invoked by the draw callback to render the screen. We render our UI on the callback thread.
static void fruttivendolo_render_callback(Canvas* canvas, void* ctx) {
    // Attempt to aquire context, so we can read the data.
    fruttivendoloContext* fruttivendolo_context = ctx;
    if(furi_mutex_acquire(fruttivendolo_context->mutex, 200) != FuriStatusOk) {
        return;
    }

    fruttivendoloData* data = fruttivendolo_context->data;
    
	furi_string_printf(data->buffer, "Basic");
    //furi_string_cat_printf(data->buffer, "fruttivendolo");

    canvas_set_bitmap_mode(canvas, 1);
	canvas_set_font(canvas, FontPrimary);
	canvas_draw_str(canvas, 16, 7, "FRUTTAROLO BT");
	canvas_set_font(canvas, FontSecondary);
	canvas_draw_str(canvas, 3, 16, "1 - normale");
	canvas_draw_str(canvas, 3, 25, "2 - airtag");
	canvas_draw_str(canvas, 3, 34, "3 - keyboard");
	canvas_draw_str(canvas, 3, 43, "4 - tv notificaion");
	canvas_draw_str(canvas, 3, 52, "5 - iphone notification");
	canvas_draw_str(canvas, 1, 62, "Selezionato:");
	canvas_draw_str(canvas, 54, 62, line_);
	canvas_draw_circle(canvas, 104, 26, 16);
	canvas_set_font(canvas, FontSecondary);
	canvas_draw_str(canvas, 102, 30, "5");
	canvas_draw_str(canvas, 113, 30, "3");
	canvas_draw_str(canvas, 102, 19, "4");
	canvas_draw_str(canvas, 103, 41, "1");
	canvas_draw_str(canvas, 91, 30, "2");

	//canvas_draw_str_aligned(canvas, 40, 30, AlignLeft, AlignTop, furi_string_get_cstr(data->buffer));

    // Release the context, so other threads can update the data.
    furi_mutex_release(fruttivendolo_context->mutex);
}



//-------------------------------------------------------------------------------------------------------------
void create_file(char* par1) {
    // We need a storage struct (gain accesso to the filesystem API )
    Storage* storage = furi_record_open(RECORD_STORAGE);

    // storage_file_alloc gives to us a File pointer using the Storage API.
    File* file = storage_file_alloc(storage);
	bool result =
        storage_file_open(file, EXT_PATH("settings/blmode.config"), FSAM_WRITE, FSOM_OPEN_ALWAYS);
    char* content = (char*)malloc(sizeof(char) * BUFFER);
    content = par1;
	line_ = content;
    if(result) {
        // this function write data on a file
        storage_file_write(file, content, strlen(content));

        // Closing the "file descriptor"
        storage_file_close(file);

        // Freeing up memory
        storage_file_free(file);
    }
	return;
}

//-------------------------------------------------------------------------------------------------------------
void read_file() {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    File* file = storage_file_alloc(storage);

    // here I used FSOM_OPEN_EXISTING (Open file, fail if file doesn't exist)
    bool result =
        storage_file_open(file, EXT_PATH("settings/blmode.config"), FSAM_READ_WRITE, FSOM_OPEN_ALWAYS);

    if(result) {
        int buffer_size = 128;
        char* buffer = (char*)malloc(sizeof(char) * buffer_size);

        // read the content of the file and insert into a buffer.
        storage_file_read(file, buffer, buffer_size);
		line_ = buffer;

        // free resources
        storage_file_close(file);
        storage_file_free(file);
    }
	return;
}

//------------------------------------------------------------------------------------------------------------------------------------

int32_t fruttivendolo_app(void* p) {
    UNUSED(p);

    // Configure our initial data.
    fruttivendoloContext* fruttivendolo_context = malloc(sizeof(fruttivendoloContext));
    fruttivendolo_context->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    fruttivendolo_context->data = malloc(sizeof(fruttivendoloData));
    fruttivendolo_context->data->buffer = furi_string_alloc();

    // Queue for events (tick or input)
    fruttivendolo_context->queue = furi_message_queue_alloc(8, sizeof(fruttivendoloEvent));

    // Set ViewPort callbacks
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, fruttivendolo_render_callback, fruttivendolo_context);
    view_port_input_callback_set(view_port, fruttivendolo_input_callback, fruttivendolo_context->queue);

    // Open GUI and register view_port
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

    // Main loop
    fruttivendoloEvent event;
    bool processing = true;
    read_file();
	do {
		if (furi_message_queue_get(fruttivendolo_context->queue, &event, FuriWaitForever) == FuriStatusOk) {
            
			//create_file();
			FURI_LOG_T(TAG, "Got event type: %d", event.type);
            switch (event.type) {
                case fruttivendoloEventTypeKey:
                    // Short press of back button exits the program.
                    if(event.input.type == InputTypeShort && event.input.key == InputKeyBack) {
                        FURI_LOG_I(TAG, "Short-Back pressed. Exiting program.");
                        processing = false;
                    }
					if(event.input.type == InputTypeShort && event.input.key == InputKeyOk) {
                        create_file("5");
                    }
					if(event.input.type == InputTypeShort && event.input.key == InputKeyUp) {
                        create_file("4");
                    }
					if(event.input.type == InputTypeShort && event.input.key == InputKeyRight) {
                        create_file("3");
                    }
					if(event.input.type == InputTypeShort && event.input.key == InputKeyLeft) {
                        create_file("2");
                    }
					if(event.input.type == InputTypeShort && event.input.key == InputKeyDown) {
                        create_file("1");
                    }
                    break;
                default:
                    break;
            }

            // Send signal to update the screen (callback will get invoked at some point later.)
            view_port_update(view_port);
        } else {
            // We had an issue getting message from the queue, so exit application.
            processing = false;
        }
    } while (processing);

    // Free resources
    view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_record_close(RECORD_GUI);
    furi_message_queue_free(fruttivendolo_context->queue);
    furi_mutex_free(fruttivendolo_context->mutex);
    furi_string_free(fruttivendolo_context->data->buffer);
    free(fruttivendolo_context->data);
    free(fruttivendolo_context);

    return 0;
}
