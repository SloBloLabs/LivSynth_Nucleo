#include "UiController.h"
#include "System.h"
#include "NoteTrackEngine.h"

void UiController::init() {
    _lastControllerUpdateTicks = System::ticks();
}

void UiController::update() {
    //handle keys
    _leds.clear();

    uint8_t pattern = _engine.trackEngine()->pattern();
    uint8_t currentStep = reinterpret_cast<NoteTrackEngine*>(_engine.trackEngine())->currentStep();
    NoteSequence &sequence = _model.project().noteSequence(pattern);
    uint8_t firstStep = sequence.firstStep();
    uint8_t lastStep = sequence.lastStep();
    bool gate;
    uint32_t note;
    for(uint8_t step = firstStep; step <= lastStep; ++step) {
        gate = sequence.step(step).gate();
        note = sequence.step(step).note();

        if(gate) {
            // 68 per half step note
            // 5 octaves -> 5 * 12 * 68 = 4080
            // 4095 max
            // 360° ^= 12 * 68 = 816
            // Color: rot         gelb         grün          hellblau      dunkelblau    magenta       rot
            // Hue  : 0° -------- 60° -------- 120° -------- 180° -------- 240° -------- 300° -------- 0°
            // Note : C           D            E             F#            G#            A#            C
            // 12bit: 0           136          272           408           544           680           816
            // 12bit value = octave [0-4] * 816 + hue * 816 / 360
            _leds.setColourHSV(step, hueFromNote(note), 1.f, step == currentStep ? 1.f : .1f);
        }
    }
}

float UiController::hueFromNote(uint32_t note) {
    return ((note % 816) * 360.f) / 816;
}