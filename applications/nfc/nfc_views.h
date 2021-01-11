#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <gui/canvas.h>
#include <flipper_v2.h>

#include "nfc_types.h"

typedef enum {
    NfcViewRead,
    NfcViewEmulate,
    NfcViewField,
    NfcViewError,
} NfcView;

typedef struct {
    bool found;
    NfcDevice device;
} NfcViewReadModel;

void nfc_view_read_draw(Canvas* canvas, void* model);
void nfc_view_read_nfca_draw(Canvas* canvas, NfcViewReadModel* model);
void nfc_view_read_nfcb_draw(Canvas* canvas, NfcViewReadModel* model);
void nfc_view_read_nfcf_draw(Canvas* canvas, NfcViewReadModel* model);
void nfc_view_read_nfcv_draw(Canvas* canvas, NfcViewReadModel* model);

void nfc_view_emulate_draw(Canvas* canvas, void* model);

void nfc_view_field_draw(Canvas* canvas, void* model);

typedef struct {
    ReturnCode error;
} NfcViewErrorModel;

void nfc_view_error_draw(Canvas* canvas, void* model);
