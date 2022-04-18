#include "Clock.h"
#include "Adc.h"
#include "DacInternal.h"
#include "Dio.h"

class Engine : private IClockObserver {
public:
    Engine(ClockTimer& clockTimer, Adc& adc, DacInternal& dac, Dio& dio);
    void init();
    void update();

    void togglePlay();
    void clockStart();
    void clockStop();
    bool clockRunning();

private:
    virtual void onClockOutput(const IClockObserver::OutputState& state) override;

    void initClock();
    void updateClockSetup();

    Clock _clock;
    Adc _adc;
    DacInternal _dac;
    Dio _dio;
};