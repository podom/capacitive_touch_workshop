#ifndef __SWITCHEDCAPSENSOR_H__
#define __SWITCHEDCAPSENSOR_H__

#define DIRECT_READ(base, mask)         (((*(base)) & (mask)) ? true : false)
#define DIRECT_MODE_INPUT(base, mask)   ((*((base)+1)) &= ~(mask))
#define DIRECT_MODE_OUTPUT(base, mask)  ((*((base)+1)) |= (mask))
#define DIRECT_WRITE_LOW(base, mask)    ((*((base)+2)) &= ~(mask))
#define DIRECT_WRITE_HIGH(base, mask)   ((*((base)+2)) |= (mask))

class SwitchedCapSensor {
  public:
    static const uint32_t DEFAULT_TIMEOUT=100;
    static const int32_t DEFAULT_HYSTERESIS=20;
    static const uint16_t BASELINE_RATE_DEFAULT=50;
    static const int32_t BASELINE_RESET_THRESHOLD=20;

    SwitchedCapSensor(const uint8_t sensePin, const uint8_t groundPin, const uint8_t samples, const int32_t threshold)
    : mInitialized(false)
    , mSwitchState(false)
    , mChanged(false)
    , mLastResult(0)
    , mBaseline(0) 
    , mThreshold(threshold)
    , mHysteresis(DEFAULT_HYSTERESIS)
    , mBaselineCount(0) 
    , mBaselineRate(BASELINE_RATE_DEFAULT)
    , mSamples(samples)
    , mSense(sensePin)
    , mGround(groundPin) {
    }

    bool Update() {
      //get raw sensor value
      mLastResult=sampleSensor();

      //check baseline initialization flag
      //reset baseline value if flag is unset
      if (!mInitialized) {
        mLastResult=sampleSensor();
        delay(10);
        mLastResult=sampleSensor();
        mBaseline=mLastResult;
        mInitialized=true;
        mBaselineCount=0;
      }

      //compute sensor level as difference between raw and baseline
      const int32_t difference=mLastResult-mBaseline;

      //adjust baseline
      //Baseline drifts toward result value when switch off
      //baseline not affected while switch on
      //Baseline adjustment rate determined by mBaselineRate
      mBaselineCount = (mBaselineCount+1) % mBaselineRate;
      if (mBaselineCount==0){// && !mSwitchState) {
        if (difference > BASELINE_RESET_THRESHOLD) {
          mBaseline=mLastResult;
        } else if (difference > 0) {
          int32_t adjustment=(difference+31)/32;
          mBaseline+=adjustment;
        } else if (difference < 0) {
          int32_t adjustment=(difference-31)/32;
          mBaseline+=adjustment;
        } else {
        }
      }

      //Check switch state
      if (!mSwitchState && difference < -(mThreshold+mHysteresis)) {
        mSwitchState=true;
        mChanged=true;
      } else if (mSwitchState && difference > -(mThreshold-mHysteresis)) {
        mSwitchState=false;
        mChanged=true;
      } else {
      }

      return mSwitchState;
      }

      bool State() const {
        return mSwitchState;
      }

      bool Changed() {
        bool rVal=mChanged;
        mChanged=false;
        return rVal;
      }

      int32_t LastResult() const {
        return mLastResult;
      }

      int32_t Baseline() const {
        return mBaseline;
      }

      void ResetBaseline() {
        mInitialized=0;
      }

      private:
      bool mInitialized; //has the baseline been initialized? Also used for reset.
      bool mSwitchState; //Is the switch off or on?
      bool mChanged; //Did the switch state just change?

      int32_t mLastResult; //Last raw result value from sensor.
      int32_t mBaseline; //Sensor baseline value.
      int32_t mThreshold; //on/off threshold
      int32_t mHysteresis; //on/off hysteresis

      uint16_t mBaselineCount; //counter to control baseline adjustment rate
      uint16_t mBaselineRate; //baseline adjustment counter period

      uint8_t mSamples; //samples value for capacitiveSensorRaw

      uint8_t mSense, mGround;

      long sampleSensor() {
        //disable pullups by setting PUD bit
        const uint8_t old_mcucr=MCUCR;
        MCUCR|=0x10;

        //access ports directly
        IO_REG_TYPE snsPin=digitalPinToBitMask(mSense);
        volatile IO_REG_TYPE* snsPort=PIN_TO_BASEREG(mSense);
        IO_REG_TYPE gndPin=digitalPinToBitMask(mGround);
        volatile IO_REG_TYPE* gndPort=PIN_TO_BASEREG(mGround);

        //initial state
        DIRECT_MODE_OUTPUT(snsPort,snsPin);
        DIRECT_WRITE_LOW(snsPort,snsPin);
        DIRECT_MODE_OUTPUT(gndPort,gndPin);
        DIRECT_WRITE_LOW(gndPort,gndPin);

        bool state=false;
        long rVal=0;

        //gnd to input
        DIRECT_MODE_INPUT(gndPort,gndPin);
        //sns to high
        DIRECT_WRITE_HIGH(snsPort,snsPin);

        while (!state) {
          //sns to input
          DIRECT_MODE_INPUT(snsPort,snsPin);
          //reconnect cap low
          DIRECT_MODE_OUTPUT(gndPort,gndPin);
          //ready to check state
          state=DIRECT_READ(snsPort,snsPin);
          //gnd to input
          DIRECT_MODE_INPUT(gndPort,gndPin);
          //sns to output
          DIRECT_MODE_OUTPUT(snsPort,snsPin);
          rVal++;
        }
        DIRECT_MODE_OUTPUT(snsPort,snsPin);
        DIRECT_WRITE_LOW(snsPort,snsPin);
        DIRECT_MODE_OUTPUT(gndPort,gndPin);
        DIRECT_WRITE_LOW(gndPort,gndPin);

        return rVal;

      }

      void settle() {
        volatile int count=1;
        while (count--) {
        }
      }

    };

#endif
